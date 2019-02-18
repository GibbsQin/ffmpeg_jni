LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := avcodec
LOCAL_SRC_FILES := prebuilt/libavcodec.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := avformat
LOCAL_SRC_FILES := prebuilt/libavformat.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := avutil
LOCAL_SRC_FILES := prebuilt/libavutil.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := swresample
LOCAL_SRC_FILES := prebuilt/libswresample.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := swscale
LOCAL_SRC_FILES := prebuilt/libswscale.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := opencore-amrnb
LOCAL_SRC_FILES := prebuilt/libopencore-amrnb.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := x264
LOCAL_SRC_FILES := prebuilt/libx264.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_SRC_FILES:=decode_audio.c \
		encode_audio.c \
		decode_video.c \
		encode_video.c \
		DecodeAndEncode.c \

LOCAL_C_INCLUDES += $(LOCAL_PATH)/include \
		include \

LOCAL_MODULE := ffmpeg

LOCAL_STATIC_LIBRARIES := avcodec avformat avutil swresample swscale opencore-amrnb x264

LOCAL_LDLIBS := -llog

LOCAL_CFLAGS:= -DHAVE_CONFIG_H

include $(BUILD_SHARED_LIBRARY)

