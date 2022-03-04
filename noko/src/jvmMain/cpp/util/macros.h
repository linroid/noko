#ifndef NODE_MACROS_H
#define NODE_MACROS_H

#include "jni.h"
#include "../prebuilt/android/x86/include/node/v8.h"
#include "log.h"

#define V8_SCOPE_NEW(env, runtime_pointer) \
  if (runtime_pointer == 0) { \
    LOGE("Invalid runtime_pointer %s(%d)-<%s>", __FILE__, __LINE__, __FUNCTION__); \
    env->FatalError("Invalid runtime_pointer"); \
  } \
  NodeRuntime *runtime = reinterpret_cast<NodeRuntime *>(runtime_pointer); \
  auto isolate = runtime->isolate_; \
  v8::Locker locker(isolate); \
  v8::HandleScope handle_scope(isolate); \

#define V8_SCOPE(env, j_this) \
  auto runtime = JsValue::GetRuntime(env, j_this); \
  if (runtime == nullptr) { \
    LOGE("GetRuntime runtime nullptr %s(%d)-<%s>", __FILE__, __LINE__, __FUNCTION__); \
    env->FatalError("GetRuntime returns nullptr"); \
  } \
  auto isolate = runtime->isolate_; \
  v8::Locker locker(isolate); \
  v8::HandleScope handle_scope(isolate); \

#define SETUP(env, j_this, type) \
  V8_SCOPE(env, j_this) \
  auto reference = JsValue::Unwrap(env, j_this); \
  auto context = runtime->context_.Get(isolate); \
  auto persistent = reinterpret_cast<v8::Persistent<type> *>(reference); \
  auto that = v8::Local<type>::New(runtime->isolate_, *persistent);


#define V8_UTF_STRING(isolate, str) v8::String::NewFromUtf8(isolate, str, v8::NewStringType::kNormal).ToLocalChecked()
#define V8_STRING(isolate, str, length) v8::String::NewFromTwoByte(isolate, str, v8::NewStringType::kNormal, length).ToLocalChecked()

#define UNUSED(expr) do { (void)(expr); } while (0)

#endif //NODE_MACROS_H