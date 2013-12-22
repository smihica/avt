#include <assert.h>
#include <string.h>
#include <algorithm>

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "main.h"
#include "Streamer.h"
// #include "Recorder.h"
#include "log.h"

// #define CALLBACK_TARGET_CLASS  "jp/smapo/model/StreamerManager"
// #define CALLBACK_TARGET_METHOD "CallbackFromNative"
// #define CALLBACK_TARGET_METHOD_TYPE "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V"

// static uav::Recorder* recorder = NULL;
static uav::Streamer* streamer = NULL;
static JavaVM* g_JavaVM = NULL;
// static jobject g_InterfaceObject = NULL;


void on_recorded(short* buf, unsigned long frames, void* context)
{
    if (streamer != NULL) streamer->on_recorded(buf, frames, context);
}

void on_user(int user_num)
{
    if (streamer != NULL) streamer->on_user(user_num);
}

void Java_com_smihica_uav_Streamer_init(JNIEnv* env, jclass jobj, jint port)
{
    if (streamer == NULL) {
        streamer = new uav::Streamer();
        /*
        recorder = new uav::Recorder(callbackfn);
        PLOG("created recorder.\n");
        recorder->init();
        PLOG("initialized recorder.\n");
        */
        PLOG("created streamer.\n");
        streamer->init(port, on_recorded, on_user);
        PLOG("initialized streamer.\n");
    }
}

void Java_com_smihica_uav_Streamer_exit(JNIEnv* env, jclass clazz)
{
    streamer->exit();
    delete streamer;
    streamer = NULL;
}

/*
// create the engine and output mix objects
void Java_com_smihica_uav_Streamer_startStreaming(JNIEnv* env, jclass clazz, jint port)
{
    streamer->startStreaming();
    recorder->startRecording();
    PLOG("start.\n");
}

// create buffer queue audio player
void Java_com_smihica_uav_Streamer_stopStreaming(JNIEnv* env, jclass clazz)
{
    streamer->stopStreaming();
    recorder->stopRecording();
}
/ *
 * For JNI codes.
 * Call java method with no parameter that returns void.
void call_java(const char* name, const char* type, const char* id1, const char* id2, const char* id3, const char* id4)
{
    JNIEnv *env = NULL;
    bool isAttached = false;
    int status;

    if (g_JavaVM == NULL) return;

    status = g_JavaVM->GetEnv((void**)&env, JNI_VERSION_1_6);
    if (status < 0) {
        PLOG("callback_handler: failed to get JNI environment, "
             "assuming native thread");
        status = g_JavaVM->AttachCurrentThread(&env, NULL);
        if(status < 0) {
            PLOG("callback_handler: failed to attach "
                 "current thread");
            return;
        }
        isAttached = true;
    }

    jclass cls = env->GetObjectClass(g_InterfaceObject);
    if(!cls) {
        PLOG("callback_handler: failed to get class reference");
        if(isAttached) g_JavaVM->DetachCurrentThread();
        return;
    }

    jmethodID mid = env->GetStaticMethodID(cls, name, type);
    if(!mid) {
        PLOG("callback_handler: failed to get method ID");
        if(isAttached) g_JavaVM->DetachCurrentThread();
        return;
    }

    jstring id1strj = env->NewStringUTF(id1);
    jstring id2strj = env->NewStringUTF(id2);
    jstring id3strj = env->NewStringUTF(id3);
    jstring id4strj = env->NewStringUTF(id4);

    env->CallStaticVoidMethod(cls, mid, id1strj, id2strj, id3strj, id4strj);

    if(isAttached) g_JavaVM->DetachCurrentThread();
}

void initClassHelper(JNIEnv *env, const char *path, jobject *objptr) {
    jclass cls = env->FindClass(path);
    if(!cls) {
        PLOG("initClassHelper: failed to get %s class reference", path);
        return;
    }
    jmethodID constr = env->GetMethodID(cls, "<init>", "()V");
    if(!constr) {
        PLOG("initClassHelper: failed to get %s constructor", path);
        return;
    }
    jobject obj = env->NewObject(cls, constr);
    if(!obj) {
        PLOG("initClassHelper: failed to create a %s object", path);
        return;
    }
    (*objptr) = env->NewGlobalRef(obj);
}
*/

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv *env;
    g_JavaVM = vm;

    if (vm->GetEnv((void**) &env, JNI_VERSION_1_6) != JNI_OK) {
        PLOG("Failed to get the environment using GetEnv()");
        return -1;
    }

    // initClassHelper(env, CALLBACK_TARGET_CLASS, &g_InterfaceObject);

    return JNI_VERSION_1_6;
}
