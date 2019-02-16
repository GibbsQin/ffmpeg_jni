#include <libavcodec/avcodec.h>

#define BOOL     int
#define UINT64   unsigned long long int
#define DWORD    unsigned long
#define BYTE     unsigned char

static BYTE* short2BYTE(short *input, int length)
{
	int ret;
	unsigned int i;
    BYTE *output = (BYTE*)malloc(sizeof(BYTE));

	for (i = 0; i < length; i++)
	{
		output[i*2] = (char)(input[i] & 0xFF);
		output[i*2+1] = (char)((input[i] >> 8) & 0xFF);
	}

	return output;
}

static short* BYTE2short(BYTE *input, int length)
{
	int ret;
	unsigned int i;
	short *output = (short*)malloc(sizeof(short));

	for (i = 0; i < length; i+=2)
	{
		output[i/2] = input[i] & 0xFF;
		output[i/2] << 8;
		output[i/2] = input[i+1] & 0xFF;
	}

	return output;
}

static BOOL fgInitEncoder(void)
{
	encode_audio_init();
	encode_video_init();
}

static DWORD dwEncodeAudio(short *pInputBuf, short* pOutputBuf,UINT64 uPTS)
{
    /* packet for holding encoded output */
    AVPacket *pkt = av_packet_alloc();
    if (!pkt) {
        Log("could not allocate the packet\n");
        return 1;
    }

    /* frame containing input raw audio */
    AVFrame *frame = av_frame_alloc();
    if (!frame) {
        Log("Could not allocate audio frame\n");
        return 1;
    }

	int length = sizeof(pInputBuf) / sizeof(short);
    frame->data[0] = short2BYTE(pInputBuf, length);
	encode_audio(frame, pkt);
	pOutputBuf = BYTE2short(pkt->data, pkt->size);

    av_frame_free(&frame);
    av_packet_free(&pkt);
}

static BOOL  fgEncodeVideoFrame(BYTE *pInput,           BYTE *pOutput, DWORD dwMaxSize)
{
    /* packet for holding encoded output */
    AVPacket *pkt = av_packet_alloc();
    if (!pkt) {
        Log("could not allocate the packet\n");
        return 1;
    }

    /* frame containing input raw audio */
    AVFrame *frame = av_frame_alloc();
    if (!frame) {
        Log("Could not allocate audio frame\n");
        return 1;
    }

    frame->data[0] = pInput;
	encode_video(frame, pkt);
	pOutput = pkt->data;

    av_frame_free(&frame);
    av_packet_free(&pkt);
}

static BOOL fgDestoryEncoder(void)
{
  	encode_audio_destroy();
	encode_video_destroy();
}

static  BOOL fgInitDecoder (void)
{
	decode_audio_init();
	decode_video_init();
}

static  BOOL  fgDecodePictureFrame(BYTE *pInputData ,  DWORD dwInputSize, UINT64 u64InputPTS,  BYTE *pOutputFrame )
{
    /* packet for holding encoded output */
    AVPacket *pkt = av_packet_alloc();
    if (!pkt) {
        Log("could not allocate the packet\n");
        return 1;
    }

    /* frame containing input raw audio */
    AVFrame *frame = av_frame_alloc();
    if (!frame) {
        Log("Could not allocate audio frame\n");
        return 1;
    }

    frame->data[0] = pInputData;
	decode_video(frame, pkt);
	pOutputFrame = pkt->data;

    av_frame_free(&frame);
    av_packet_free(&pkt);
}

static BOOL fgDecodeAudio(short *pInputBuf, DWORD dwInputSize, short* pOutputBuf, DWORD* dwOutputSize)
{
    /* packet for holding encoded output */
    AVPacket *pkt = av_packet_alloc();
    if (!pkt) {
        Log("could not allocate the packet\n");
        return 1;
    }

    /* frame containing input raw audio */
    AVFrame *frame = av_frame_alloc();
    if (!frame) {
        Log("Could not allocate audio frame\n");
        return 1;
    }

	int length = sizeof(pInputBuf) / sizeof(short);
    frame->data[0] = short2BYTE(pInputBuf, length);
	decode_audio(frame, pkt);
	pOutputBuf = BYTE2short(pkt->data, pkt->size);

    av_frame_free(&frame);
    av_packet_free(&pkt);

}

static BOOL fgDestoryDecoder(void)
{
  	decode_audio_destroy();
	decode_video_destroy();
}

