#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <android/log.h>

#include <libavutil/frame.h>
#include <libavutil/mem.h>
	
#include <libavcodec/avcodec.h>

#define LOGI(TAG,...)  __android_log_print(ANDROID_LOG_INFO,TAG,__VA_ARGS__)
#define Log(fmt...)   LOGI("Gview",fmt)

AVCodecContext *decodeAudioCxt= NULL;

static void decode_audio(AVCodecContext *dec_ctx, AVPacket *pkt, AVFrame *frame)
{
    int i, ch;
    int ret, data_size;

    /* send the packet with the compressed data to the decoder */
    ret = avcodec_send_packet(dec_ctx, pkt);
    if (ret < 0) {
        Log("Error submitting the packet to the decoder\n");
        return;
    }

    /* read all the output frames (in general there may be any number of them */
    while (ret >= 0) {
        ret = avcodec_receive_frame(dec_ctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0) {
            Log("Error during decoding\n");
            return;
        }
        data_size = av_get_bytes_per_sample(dec_ctx->sample_fmt);
        if (data_size < 0) {
            /* This should not occur, checking just for paranoia */
            Log("Failed to calculate data size\n");
            return;
        }
    }
    av_frame_free(&frame);
    av_packet_free(&pkt);
}

int decode_audio_init()
{
    /* find the MPEG audio decoder */
    const AVCodec *codec = avcodec_find_decoder(AV_CODEC_ID_MP2);
    if (!codec) {
        Log("Codec not found\n");
        return 1;
    }

    decodeAudioCxt = avcodec_alloc_context3(codec);
    if (!decodeAudioCxt) {
        Log("Could not allocate audio codec context\n");
        return 1;
    }

    /* open it */
    if (avcodec_open2(decodeAudioCxt, codec, NULL) < 0) {
        Log("Could not open codec\n");
        return 1;
    }

    return 0;
}

void decode_audio_destroy()
{
    avcodec_free_context(&decodeAudioCxt);
}
