//
// Created by linroid on 2019-10-20.
//

#ifndef NODE_JSVALUE_H
#define NODE_JSVALUE_H

#include <jni.h>
#include "../NodeRuntime.h"

class JSValue {
public:
    static jobject New(JNIEnv *env, NodeRuntime *runtime, v8::Local<v8::Value> &value);

    JNICALL static jstring ToString(JNIEnv *env, jobject thiz);

    JNICALL static jstring ToJson(JNIEnv *env, jobject thiz);

    JNICALL static void Dispose(JNIEnv *env, jobject thiz);

    static jint OnLoad(JNIEnv *env);

    static JNIClass& Class();

    static jobject GetContext(JNIEnv *env, jobject javaObj);

    static jlong GetReference(JNIEnv *env, jobject javaObj);

    static v8::Local<v8::Value> GetV8Value(JNIEnv *env, v8::Isolate *isolate, jobject javaObj);

    static void SetReference(JNIEnv *env, jobject javaObj, jlong value);
};

#endif //NODE_JSVALUE_H
