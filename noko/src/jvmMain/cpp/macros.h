#ifndef NODE_MACROS_H
#define NODE_MACROS_H

#include <jni.h>
#include "v8.h"
#include "log.h"

#define V8_SCOPE_NEW(env, node_pointer) \
  if (node_pointer == 0) { \
    LOGE("Invalid node_pointer %s(%d)-<%s>", __FILE__, __LINE__, __FUNCTION__); \
    env->FatalError("Invalid node_pointer"); \
  } \
  Node *node = reinterpret_cast<Node *>(node_pointer); \
  auto isolate = node->isolate_; \
  v8::Locker locker(isolate); \
  v8::HandleScope handleScope(isolate); \

#define V8_SCOPE(env, jThis) \
  auto node = JsValue::GetNode(env, jThis); \
  if (node == nullptr) { \
    LOGE("GetNode node nullptr %s(%d)-<%s>", __FILE__, __LINE__, __FUNCTION__); \
    env->FatalError("GetRuntime returns nullptr"); \
  } \
  auto isolate = node->isolate_; \
  v8::Locker locker(isolate); \
  v8::HandleScope handleScope(isolate); \

#define SETUP(env, jThis, type) \
  V8_SCOPE(env, jThis) \
  auto reference = JsValue::Unwrap(env, jThis); \
  auto context = node->context_.Get(isolate); \
  auto persistent = reinterpret_cast<v8::Persistent<type> *>(reference); \
  auto that = v8::Local<type>::New(node->isolate_, *persistent);


#define V8_UTF_STRING(isolate, str) v8::String::NewFromUtf8(isolate, str, v8::NewStringType::kNormal).ToLocalChecked()
#define V8_STRING(isolate, str, length) v8::String::NewFromTwoByte(isolate, str, v8::NewStringType::kNormal, length).ToLocalChecked()

#define UNUSED(expr) do { (void)(expr); } while (0)

#endif //NODE_MACROS_H
