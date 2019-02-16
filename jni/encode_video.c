#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <android/log.h>

#include <libavcodec/avcodec.h>

#include <libavutil/opt.h>
#include <libavutil/imgutils.h>

#define LOGI(TAG,...)  __android_log_print(ANDROID_LOG_INFO,TAG,__VA_ARGS__)
#define Log(fmt...)   LOGI("Gview",fmt)

AVCodecContext *encodeVideoCxt= NULL;
AVFrame *frame;
AVPacket *pkt;

static void encode_video(AVFrame *frame, AVPacket *pkt)
{
	frame->format = encodeVideoCxt->pix_fmt;
	frame->width  = encodeVideoCxt->width;
	frame->height = encodeVideoCxt->height;

    int ret;

    /* send the frame to the encoder */
    if (frame)
        Log("Send frame %lld\n", frame->pts);

    ret = avcodec_send_frame(encodeVideoCxt, frame);
    if (ret < 0) {
        Log("Error sending a frame for encoding\n");
        return;
    }

    while (ret >= 0) {
        ret = avcodec_receive_packet(encodeVideoCxt, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0) {
            Log("Error during encoding\n");
            return;
        }

        Log("Write packet %lld (size=%5d)\n", pkt->pts, pkt->size);
        av_packet_unref(pkt);
    }
}

int encode_video_init()
{
    const AVCodec *codec;
    int i, ret;

    /* find the mpeg1video encoder */
    codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!codec) {
        Log("Codec '%d' not found\n", AV_CODEC_ID_H264);
        return 1;
    }

    encodeVideoCxt = avcodec_alloc_context3(codec);
    if (!encodeVideoCxt) {
        Log("Could not allocate video codec context\n");
        return 1;
    }

    pkt = av_packet_alloc();
    if (!pkt)
        return 1;

    /* put sample parameters */
    encodeVideoCxt->bit_rate = 400000;
    /* resolution must be a multiple of two */
    encodeVideoCxt->width = 352;
    encodeVideoCxt->height = 288;
    /* frames per second */
    encodeVideoCxt->time_base = (AVRational){1, 25};
    encodeVideoCxt->framerate = (AVRational){25, 1};

    /* emit one intra frame every ten frames
     * check frame pict_type before passing frame
     * to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
     * then gop_size is ignored and the output of encoder
     * will always be I frame irrespective to gop_size
     */
    encodeVideoCxt->gop_size = 10;
    encodeVideoCxt->max_b_frames = 1;
    encodeVideoCxt->pix_fmt = AV_PIX_FMT_YUV420P;

    if (codec->id == AV_CODEC_ID_H264)
        av_opt_set(encodeVideoCxt->priv_data, "preset", "slow", 0);

    /* open it */
    ret = avcodec_open2(encodeVideoCxt, codec, NULL);
    if (ret < 0) {
        Log("Could not open codec: %s\n", av_err2str(ret));
        return 1;
    }

    frame = av_frame_alloc();
    if (!frame) {
        Log("Could not allocate video frame\n");
        return 1;
    }
    frame->format = encodeVideoCxt->pix_fmt;
    frame->width  = encodeVideoCxt->width;
    frame->height = encodeVideoCxt->height;

    ret = av_frame_get_buffer(frame, 32);
    if (ret < 0) {
        Log("Could not allocate the video frame data\n");
        return 1;
    }

    return 0;
}

void encode_video_destroy()
{
	avcodec_free_context(&encodeVideoCxt);
	av_frame_free(&frame);
	av_packet_free(&pkt);
}
