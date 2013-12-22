#ifndef MAIN_H_
#define MAIN_H_

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif
    void on_recorded(short* buf, unsigned long frames, void* context);
    void on_user(int user_num);
    void Java_com_smihica_uav_Streamer_init(JNIEnv* env, jclass jobj, jint port);
    void Java_com_smihica_uav_Streamer_exit(JNIEnv* env, jclass clazz);
    // void callbackfn(short* buf, unsigned long frames, void* context);
    // void Java_com_smihica_uav_Streamer_startStreaming(JNIEnv* env, jclass clazz, jint port);
    // void Java_com_smihica_uav_Streamer_stopStreaming(JNIEnv* env, jclass clazz);
    jint JNI_OnLoad(JavaVM* vm, void* reserved);
#ifdef __cplusplus
}
#endif

#endif
