//
// Created by linroid on 2019-10-22.
//

#ifndef NODE_MACROS_H
#define NODE_MACROS_H

#include <android/log.h>
#include <jni.h>
#include "v8.h"


#define LOG_TAG "KNode_Native"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define ALOG(level, ...) __android_log_print(level, LOG_TAG, __VA_ARGS__)

// LOGD("%s(%d)-<%s>",  __FILE__, __LINE__, __FUNCTION__); \

#define V8_SCOPE(env, jThis) \
    auto runtime = JSValue::GetRuntime(env, jThis); \
    auto isolate = runtime->isolate_; \
    auto reference = JSValue::Unwrap(env, jThis); \
    auto runnable = [&]() { \
        v8::Locker locker(runtime->isolate_); \
        v8::HandleScope handleScope(runtime->isolate_); \

// CHECK(_reference != 0); \

#define V8_CONTEXT(env, jThis, type) \
        V8_SCOPE(env, jThis) \
        auto context = runtime->context_.Get(isolate); \
        auto persistent = reinterpret_cast<v8::Persistent<type> *>(reference); \
        auto that = v8::Local<type>::New(runtime->isolate_, *persistent);

#define V8_END()  \
    }; \
    runtime->Await(runnable);

#define V8_UTF_STRING(isolate, str) v8::String::NewFromUtf8(isolate, str, v8::NewStringType::kNormal).ToLocalChecked()
#define V8_STRING(isolate, str, length) v8::String::NewFromTwoByte(isolate, str, v8::NewStringType::kNormal, length).ToLocalChecked()


#define ENTER_JNI(vm)                                                           \
    {                                                                           \
        JNIEnv *env;                                                            \
        auto stat = vm->GetEnv((void **) (&env), JNI_VERSION_1_6);              \
        if (stat == JNI_EDETACHED) {                                            \
            vm->AttachCurrentThread(&env, nullptr);                             \
        }                                                                       \
        if (env->ExceptionCheck()) {                                            \
            LOGE("Enter jni env, but with pending Exception", __FILE__);        \
            env->Throw(env->ExceptionOccurred());                               \
        }

#define EXIT_JNI(vm)                                                            \
        if (env->ExceptionCheck()) {                                            \
            LOGE("Exit jni env, but with pending Exception, %s:%d", __FILE__, __LINE__); \
            env->Throw(env->ExceptionOccurred());                               \
        }                                                                       \
        if (stat == JNI_EDETACHED) {                                            \
            vm->DetachCurrentThread();                                          \
        }                                                                       \
    }
#endif //NODE_MACROS_H
