//
// Created by linroid on 2019-10-22.
//

#ifndef NODE_MACROS_H
#define NODE_MACROS_H

#include "v8.h"

#define V8_ENV(env, thiz, type) \
    auto runtime = JSContext::Runtime(env, thiz); \
    v8::Locker locker_(runtime->isolate); \
    v8::HandleScope handleScope_(runtime->isolate); \
    auto context = runtime->context.Get(runtime->isolate); \
    jlong reference_ = JSValue::ReadReference(env, thiz); \
    auto persistent = reinterpret_cast<v8::Persistent<type> *>(reference_); \
    auto value = v8::Local<type>::New(runtime->isolate, *persistent);
//
// #define V8_ENV(env, thiz) \
//     auto runtime = JSContext::Runtime(env, thiz); \
//     v8::Locker locker_(runtime->isolate); \
//     v8::HandleScope handleScope_(runtime->isolate); \
//     auto context = runtime->context.Get(runtime->isolate); \
//     jlong reference = JSValue::ReadReference(env, javaObj); \
//     auto value = v8::Local<v8::Value>::New(runtime->isolate, reinterpret_cast<v8::Persistent<v8::Value> *>());

#endif //NODE_MACROS_H
