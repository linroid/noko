//
// Created by linroid on 2019-10-21.
//

#ifndef NODE_V8RUNTIME_H
#define NODE_V8RUNTIME_H

#include <jni.h>
#include "v8.h"
#include "node.h"
#include "uv.h"

// #define JS_ENV(env, javaObject) getIsolate(env, v8RuntimePtr);\
//     if ( isolate == NULL ) {\
//       return errorReturnResult;\
//                                 }\
//     V8Runtime* runtime = reinterpret_cast<V8Runtime*>(v8RuntimePtr);\
//     Isolate::Scope isolateScope(isolate);\
//     HandleScope handle_scope(isolate);\
//     Local<Context> context = Local<Context>::New(isolate,runtime->context_);\
//     Context::Scope context_scope(context);


class V8Runtime {
public:
    jobject javaContext;

    v8::Isolate *isolate;
    v8::Persistent<v8::Context> context;
    v8::Persistent<v8::Object> *global;
    v8::Locker *locker;
    node::Environment *nodeEnv;
    node::IsolateData *isolateData;
    uv_loop_t *uvLoop;
    bool running;

    jobject Wrap(JNIEnv *env, v8::Local<v8::Value> &value);
};


#endif //NODE_V8RUNTIME_H
