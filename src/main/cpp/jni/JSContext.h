//
// Created by linroid on 2019-10-20.
//

#ifndef NODE_JS_CONTEXT_H
#define NODE_JS_CONTEXT_H

#include <jni.h>
#include "../NodeRuntime.h"

class JSContext {
public:
    static NodeRuntime *GetRuntime(JNIEnv *env, jobject javaObject);

    static jobject Wrap(JNIEnv *env, NodeRuntime *runtime);

    static JNICALL jobject Get(JNIEnv *env, jobject thiz, jstring key);

    static JNICALL jobject Get(JNIEnv *env, jobject thiz, jstring key, jobject value);

    static JNICALL jlong Bind(JNIEnv *env, jobject thiz, jlong contextPtr);

    static JNICALL jobject ParseJSON(JNIEnv *env, jstring thiz, jstring str);

    static JNICALL jobject Eval(JNIEnv *env, jstring thiz, jstring jcode, jstring jsource, jint jline);

    static jint OnLoad(JNIEnv *env);
};


//    auto v8Runtime = reinterpret_cast<JSContext *>(v8RuntimePtr);
//    auto runtime = reinterpret_cast<JSContext *>(v8RuntimePtr);
//    v8::Isolate::Scope isolateScope(runtime->isolate);
//    v8::HandleScope handle_scope(runtime->isolate);
//    auto context = v8::Local<v8::Context>::New(runtime->isolate, runtime->context);
//    v8::Context::Scope context_scope(context);
//    v8::Object::New(runtime->isolate);
////    auto value = new JSValue(env, v8RuntimePtr);
//    auto container = new v8::Persistent<v8::Object>;
////    container->Reset(v8Runtime->isolate, );
//    return reinterpret_cast<jlong>(container);
//    return 0;
//}


#endif //NODE_JS_CONTEXT_H
