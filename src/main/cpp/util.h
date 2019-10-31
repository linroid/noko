//
// Created by linroid on 2019-10-19.
//

#ifndef NODE_LOG_H
#define NODE_LOG_H

#include <android/log.h>
#include <jni.h>

#define LOG_TAG "KNode"

#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define ALOG(level, ...) __android_log_print(level, LOG_TAG, __VA_ARGS__)


inline void throwError(JNIEnv *env, const char *message) {
    env->ThrowNew(env->FindClass("java/lang/Exception"), message);
}


#endif //NODE_LOG_H
