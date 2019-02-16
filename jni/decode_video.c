#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <android/log.h>

#include <libavcodec/avcodec.h>

#define LOGI(TAG,...)  __android_log_print(ANDROID_LOG_INFO,TAG,__VA_ARGS__)
#define Log(fmt...)   LOGI("Gview",fmt)

AVCodecContext *decodeVideoCxt= NULL;

static void decode_video(AVFrame *frame, AVPacket *pkt)
{
    char buf[1024];
    int ret;

    ret = avcodec_send_packet(decodeVideoCxt, pkt);
    if (ret < 0) {
        Log("Error sending a packet for decoding\n");
        return;
    }

    while (ret >= 0) {
        ret = avcodec_receive_frame(decodeVideoCxt, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0) {
            Log("Error during decoding\n");
            return;
        }

        Log("saving frame %3d\n", decodeVideoCxt->frame_number);
        fflush(stdout);
    }
	av_frame_free(&frame);
	av_packet_free(&pkt);
}

int decode_video_init()
{
    const AVCodec *codec;
    int ret;

    /* find the MPEG-1 video decoder */
    codec = avcodec_find_decoder(AV_CODEC_ID_MPEG1VIDEO);
    if (!codec) {
        Log("Codec not found\n");
        return 1;
    }

    decodeVideoCxt = avcodec_alloc_context3(codec);
    if (!decodeVideoCxt) {
        Log("Could not allocate video codec context\n");
        return 1;
    }

    /* For some codecs, such as msmpeg4 and mpeg4, width and height
       MUST be initialized there because this information is not
       available in the bitstream. */

    /* open it */
    if (avcodec_open2(decodeVideoCxt, codec, NULL) < 0) {
        Log("Could not open codec\n");
        return 1;
    }

    return 0;
}

void decode_video_destroy()
{
	avcodec_free_context(&decodeVideoCxt);
}
