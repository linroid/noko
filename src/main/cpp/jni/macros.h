//
// Created by linroid on 2019-10-22.
//

#ifndef NODE_MACROS_H
#define NODE_MACROS_H

#include "v8.h"

#define V8_SCOPE(env, jThis) \
    auto runtime = JSValue::GetRuntime(env, jThis); \
    auto isolate = runtime->isolate; \
    auto _reference = JSValue::Unwrap(env, jThis); \
    auto _runnable = [&]() { \
        v8::Locker _locker(runtime->isolate); \
        v8::HandleScope _handleScope(runtime->isolate); \

#define V8_CONTEXT(env, jThis, type) \
        V8_SCOPE(env, jThis) \
        CHECK(_reference != 0); \
        auto context = runtime->context.Get(isolate); \
        auto _persistent = reinterpret_cast<v8::Persistent<type> *>(_reference); \
        auto that = v8::Local<type>::New(runtime->isolate, *_persistent);

#define V8_END()  \
    }; \
    runtime->Await(_runnable);

#define V8_UTF_STRING(isolate, str) v8::String::NewFromUtf8(isolate, str, v8::NewStringType::kNormal).ToLocalChecked()
#define V8_STRING(str, length) v8::String::NewFromTwoByte(isolate, str, v8::NewStringType::kNormal, length).ToLocalChecked()

#endif //NODE_MACROS_H
