#ifndef NODE_MACROS_H
#define NODE_MACROS_H

#include <jni.h>
#include "v8.h"
#include "log.h"

#define V8_SCOPE_NEW(env) \
  auto runtime = Runtime::Current(); \
  auto isolate = runtime->isolate_; \
  v8::Locker locker(isolate); \
  v8::HandleScope handle_scope(isolate); \

#define V8_SCOPE(env, j_this) \
  auto runtime = Runtime::Current(); \
  if (runtime == nullptr) { \
    LOGE("GetRuntime runtime nullptr %s(%d)-<%s>", __FILE__, __LINE__, __FUNCTION__); \
    env->FatalError("GetRuntime returns nullptr"); \
  } \
  auto isolate = runtime->isolate_; \
  v8::Locker locker(isolate); \
  v8::HandleScope handle_scope(isolate); \

#define SETUP(env, j_this, type) \
  V8_SCOPE(env, j_this) \
  auto context = runtime->context_.Get(isolate); \
  auto reference = reinterpret_cast<v8::Persistent<type> *>(JsValue::GetPointer(env, j_this)); \
  auto that = reference->Get(isolate);


#define V8_UTF_STRING(isolate, str) v8::String::NewFromUtf8(isolate, str, v8::NewStringType::kNormal).ToLocalChecked()
#define V8_STRING(isolate, str, length) v8::String::NewFromTwoByte(isolate, str, v8::NewStringType::kNormal, length).ToLocalChecked()

#define UNUSED(expr) do { (void)(expr); } while (0)

#endif //NODE_MACROS_H
