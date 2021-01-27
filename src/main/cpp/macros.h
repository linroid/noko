//
// Created by linroid on 2019-10-22.
//

#ifndef NODE_MACROS_H
#define NODE_MACROS_H

#include <jni.h>
#include "v8.h"
#include "log.h"

#define LOG_TAG "KNode_Native"

// LOGD("%s(%d)-<%s>",  __FILE__, __LINE__, __FUNCTION__); \

#define V8_SCOPE(env, jThis) \
  auto runtime = JSValue::GetRuntime(env, jThis); \
  auto isolate = runtime->isolate_; \
  auto reference = JSValue::Unwrap(env, jThis); \
  v8::Locker locker(runtime->isolate_); \
  v8::HandleScope handleScope(runtime->isolate_); \

// CHECK(_reference != 0); \

#define SETUP(env, jThis, type) \
  V8_SCOPE(env, jThis) \
  auto context = runtime->context_.Get(isolate); \
  auto persistent = reinterpret_cast<v8::Persistent<type> *>(reference); \
  auto that = v8::Local<type>::New(runtime->isolate_, *persistent);


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
      LOGE("Enter jni env  with pending Exception, %s: %d", __FILE__, __LINE__);        \
      env->Throw(env->ExceptionOccurred());                               \
    }

#define EXIT_JNI(vm)                                                            \
    if (env->ExceptionCheck()) {                                            \
      LOGE("Exit jni env with pending Exception, %s: %d", __FILE__, __LINE__); \
      env->Throw(env->ExceptionOccurred());                               \
    }                                                                       \
    if (stat == JNI_EDETACHED) {                                            \
      vm->DetachCurrentThread();                                          \
    }                                                                       \
  }
#endif //NODE_MACROS_H


// #define UNUSED __attribute__((unused))
#define UNUSED(expr) do { (void)(expr); } while (0)
