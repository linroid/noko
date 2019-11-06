//
// Created by linroid on 2019-10-22.
//

#ifndef NODE_MACROS_H
#define NODE_MACROS_H

#include "v8.h"

#define V8_SCOPE(env, jthis) \
    auto runtime = JSContext::GetRuntime(env, jthis); \
    auto _runnable = [&]() { \
        v8::Locker _locker(runtime->isolate); \
        v8::HandleScope _handleScope(runtime->isolate); \


#define V8_CONTEXT(env, jthis, type) \
        V8_SCOPE(env, jthis) \
        auto context = runtime->context.Get(runtime->isolate); \
        jlong _reference = JSValue::GetReference(env, jthis); \
        auto _persistent = reinterpret_cast<v8::Persistent<type> *>(_reference); \
        auto that = v8::Local<type>::New(runtime->isolate, *_persistent);

#define V8_END()  \
    }; \
    runtime->Run(_runnable);

#define V8_UTF_STRING(str) v8::String::NewFromUtf8(isolate, str, v8::NewStringType::kNormal).ToLocalChecked()
#define V8_STRING(str) v8::String::NewFromTwoByte(isolate, str, v8::NewStringType::kNormal).ToLocalChecked()

#endif //NODE_MACROS_H
