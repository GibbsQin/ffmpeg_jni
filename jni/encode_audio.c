#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <android/log.h>

#include <libavcodec/avcodec.h>

#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/frame.h>
#include <libavutil/samplefmt.h>

#define LOGI(TAG,...)  __android_log_print(ANDROID_LOG_INFO,TAG,__VA_ARGS__)
#define Log(fmt...)   LOGI("Gview",fmt)

AVCodecContext *encodeAudioCxt= NULL;
AVFrame *frame;
AVPacket *pkt;

/* check that a given sample format is supported by the encoder */
static int check_sample_fmt(const AVCodec *codec, enum AVSampleFormat sample_fmt)
{
    const enum AVSampleFormat *p = codec->sample_fmts;

    while (*p != AV_SAMPLE_FMT_NONE) {
        if (*p == sample_fmt)
            return 1;
        p++;
    }
    return 0;
}

/* just pick the highest supported samplerate */
static int select_sample_rate(const AVCodec *codec)
{
    const int *p;
    int best_samplerate = 0;

    if (!codec->supported_samplerates)
        return 44100;

    p = codec->supported_samplerates;
    while (*p) {
        if (!best_samplerate || abs(44100 - *p) < abs(44100 - best_samplerate))
            best_samplerate = *p;
        p++;
    }
    return best_samplerate;
}

/* select layout with the highest channel count */
static int select_channel_layout(const AVCodec *codec)
{
    const uint64_t *p;
    uint64_t best_ch_layout = 0;
    int best_nb_channels   = 0;

    if (!codec->channel_layouts)
        return AV_CH_LAYOUT_STEREO;

    p = codec->channel_layouts;
    while (*p) {
        int nb_channels = av_get_channel_layout_nb_channels(*p);

        if (nb_channels > best_nb_channels) {
            best_ch_layout    = *p;
            best_nb_channels = nb_channels;
        }
        p++;
    }
    return best_ch_layout;
}

static void encode_audio(AVFrame *frame, AVPacket *pkt)
{
    frame->nb_samples     = encodeAudioCxt->frame_size;
    frame->format         = encodeAudioCxt->sample_fmt;
    frame->channel_layout = encodeAudioCxt->channel_layout;

    int ret;

    /* send the frame for encoding */
    ret = avcodec_send_frame(encodeAudioCxt, frame);
    if (ret < 0) {
        Log("Error sending the frame to the encoder\n");
        return;
    }

    /* read all the available output packets (in general there may be any
     * number of them */
    while (ret >= 0) {
        ret = avcodec_receive_packet(encodeAudioCxt, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0) {
            Log("Error encoding audio frame\n");
            return;
        }

        av_packet_unref(pkt);
    }
}

int encode_audio_init()
{
    const AVCodec *codec;
    int ret;

    /* find the MP2 encoder */
    codec = avcodec_find_encoder(AV_CODEC_ID_MP2);
    if (!codec) {
        Log("Codec not found\n");
        return 1;
    }

    encodeAudioCxt = avcodec_alloc_context3(codec);
    if (!encodeAudioCxt) {
        Log("Could not allocate audio codec context\n");
        return 1;
    }

    /* put sample parameters */
    encodeAudioCxt->bit_rate = 64000;

    /* check that the encoder supports s16 pcm input */
    encodeAudioCxt->sample_fmt = AV_SAMPLE_FMT_S16;
    if (!check_sample_fmt(codec, encodeAudioCxt->sample_fmt)) {
        Log("Encoder does not support sample format %s",
                av_get_sample_fmt_name(encodeAudioCxt->sample_fmt));
        return 1;
    }

    /* select other audio parameters supported by the encoder */
    encodeAudioCxt->sample_rate    = select_sample_rate(codec);
    encodeAudioCxt->channel_layout = select_channel_layout(codec);
    encodeAudioCxt->channels       = av_get_channel_layout_nb_channels(encodeAudioCxt->channel_layout);

    /* open it */
    if (avcodec_open2(encodeAudioCxt, codec, NULL) < 0) {
        Log("Could not open codec\n");
        return 1;
    }

    /* packet for holding encoded output */
    pkt = av_packet_alloc();
    if (!pkt) {
        Log("could not allocate the packet\n");
        return 1;
    }

    /* frame containing input raw audio */
    frame = av_frame_alloc();
    if (!frame) {
        Log("Could not allocate audio frame\n");
        return 1;
    }

    frame->nb_samples     = encodeAudioCxt->frame_size;
    frame->format         = encodeAudioCxt->sample_fmt;
    frame->channel_layout = encodeAudioCxt->channel_layout;

    /* allocate the data buffers */
    ret = av_frame_get_buffer(frame, 0);
    if (ret < 0) {
        Log("Could not allocate audio data buffers\n");
        return 1;
    }

    return 0;
}

void encode_audio_destroy()
{
	av_frame_free(&frame);
    av_packet_free(&pkt);
    avcodec_free_context(&encodeAudioCxt);
}