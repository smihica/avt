LOCAL_PATH:=$(call my-dir)
RELEASE:= -D_RELEASE_

#----------------------------------
include $(CLEAR_VARS)

LOCAL_MODULE := streamer
LOCAL_CFLAGS := -O2 -Wall -DANDROID -DSTDC_HEADERS -I$(LOCAL_PATH)/libmp3lame/

LOCAL_SRC_FILES := Streamer.cpp		\
                   Recorder.cpp		\
                   Encoder.cpp		\
                   StreamingServer.cpp	\
                   main.cpp

LOCAL_SRC_FILES += libmp3lame/bitstream.c\
                   libmp3lame/fft.c\
                   libmp3lame/id3tag.c\
                   libmp3lame/mpglib_interface.c\
                   libmp3lame/presets.c\
                   libmp3lame/quantize.c\
                   libmp3lame/reservoir.c\
                   libmp3lame/tables.c\
                   libmp3lame/util.c\
                   libmp3lame/VbrTag.c\
                   libmp3lame/encoder.c\
                   libmp3lame/gain_analysis.c\
                   libmp3lame/lame.c\
                   libmp3lame/newmdct.c\
                   libmp3lame/psymodel.c\
                   libmp3lame/quantize_pvt.c\
                   libmp3lame/set_get.c\
                   libmp3lame/takehiro.c\
                   libmp3lame/vbrquantize.c\
                   libmp3lame/version.c

LOCAL_CPPFLAGS:= -D_ANDROID_ $(RELEASE)

LOCAL_LDLIBS +=			\
	-lOpenSLES		\
	-llog			\
	-landroid

include $(BUILD_SHARED_LIBRARY)

#----------------------------------
#   mp3 encoder
#
# include $(CLEAR_VARS)
# LOCAL_MODULE := encoder
# LOCAL_CFLAGS := -O2 -Wall -DANDROID -DSTDC_HEADERS -I./libmp3lame/

# LOCAL_SRC_FILES += libmp3lame/bitstream.c\
                   libmp3lame/fft.c\
                   libmp3lame/id3tag.c\
                   libmp3lame/mpglib_interface.c\
                   libmp3lame/presets.c\
                   libmp3lame/quantize.c\
                   libmp3lame/reservoir.c\
                   libmp3lame/tables.c\
                   libmp3lame/util.c\
                   libmp3lame/VbrTag.c\
                   libmp3lame/encoder.c\
                   libmp3lame/gain_analysis.c\
                   libmp3lame/lame.c\
                   libmp3lame/newmdct.c\
                   libmp3lame/psymodel.c\
                   libmp3lame/quantize_pvt.c\
                   libmp3lame/set_get.c\
                   libmp3lame/takehiro.c\
                   libmp3lame/vbrquantize.c\
                   libmp3lame/version.c\
                   Encoder.cpp

# LOCAL_LDLIBS := -llog

# include $(BUILD_SHARED_LIBRARY)
