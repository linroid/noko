//
// Created by linroid on 2019-10-22.
//

#ifndef NODE_MACROS_H
#define NODE_MACROS_H

#include <jni.h>
#include "v8.h"
#include "log.h"

#define V8_SCOPE(env, jThis) \
  auto runtime = JSValue::GetRuntime(env, jThis); \
  if (runtime == nullptr) { \
    LOGE("GetRuntime returns nullptr %s(%d)-<%s>", __FILE__, __LINE__, __FUNCTION__); \
    env->FatalError("GetRuntime returns nullptr"); \
  } \
  auto isolate = runtime->isolate_; \
  v8::Locker locker(runtime->isolate_); \
  v8::HandleScope handleScope(runtime->isolate_); \

#define SETUP(env, jThis, type) \
  V8_SCOPE(env, jThis) \
  auto reference = JSValue::Unwrap(env, jThis); \
  auto context = runtime->context_.Get(isolate); \
  auto persistent = reinterpret_cast<v8::Persistent<type> *>(reference); \
  auto that = v8::Local<type>::New(runtime->isolate_, *persistent);


#define V8_UTF_STRING(isolate, str) v8::String::NewFromUtf8(isolate, str, v8::NewStringType::kNormal).ToLocalChecked()
#define V8_STRING(isolate, str, length) v8::String::NewFromTwoByte(isolate, str, v8::NewStringType::kNormal, length).ToLocalChecked()

#define UNUSED(expr) do { (void)(expr); } while (0)

#endif //NODE_MACROS_H