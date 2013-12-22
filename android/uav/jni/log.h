#ifndef _LOG_H_
#define _LOG_H_

#include <android/log.h>

#define  LOG_TAG    "com.smihica.uav"
#define  PLOG(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

#endif
