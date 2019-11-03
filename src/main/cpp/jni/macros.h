//
// Created by linroid on 2019-10-22.
//

#ifndef NODE_MACROS_H
#define NODE_MACROS_H

#include "v8.h"

#define V8_ENV(env, jthis, type) \
    auto runtime = JSContext::GetRuntime(env, jthis); \
    v8::Locker locker_(runtime->isolate); \
    v8::HandleScope handleScope_(runtime->isolate); \
    auto context = runtime->context.Get(runtime->isolate); \
    jlong reference_ = JSValue::GetReference(env, jthis); \
    auto persistent = reinterpret_cast<v8::Persistent<type> *>(reference_); \
    auto that = v8::Local<type>::New(runtime->isolate, *persistent);
//
// #define V8_ENV(env, jthis) \
//     auto runtime = JSContext::Runtime(env, jthis); \
//     v8::Locker locker_(runtime->isolate); \
//     v8::HandleScope handleScope_(runtime->isolate); \
//     auto context = runtime->context.Get(runtime->isolate); \
//     jlong reference = JSValue::ReadReference(env, jobj); \
//     auto value = v8::Local<v8::Value>::New(runtime->isolate, reinterpret_cast<v8::Persistent<v8::Value> *>());


#define V8_UTF_STRING(str) v8::String::NewFromUtf8(isolate, str, v8::NewStringType::kNormal).ToLocalChecked()
#define V8_STRING(str) v8::String::NewFromTwoByte(isolate, str, v8::NewStringType::kNormal).ToLocalChecked()

#endif //NODE_MACROS_H
