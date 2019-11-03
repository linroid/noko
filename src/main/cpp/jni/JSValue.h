//
// Created by linroid on 2019-10-20.
//

#ifndef NODE_JSVALUE_H
#define NODE_JSVALUE_H

#include <jni.h>
#include "../NodeRuntime.h"

class JSValue {
public:
    static jobject Wrap(JNIEnv *env, NodeRuntime *runtime, v8::Local<v8::Value> &value);

    JNICALL static jstring ToString(JNIEnv *env, jobject jthis);

    JNICALL static jstring TypeOf(JNIEnv *env, jobject jthis);

    JNICALL static jstring ToJson(JNIEnv *env, jobject jthis);

    JNICALL static void Dispose(JNIEnv *env, jobject jthis);

    static jint OnLoad(JNIEnv *env);

    static jobject GetContext(JNIEnv *env, jobject javaObj);

    static jlong GetReference(JNIEnv *env, jobject javaObj);

    static v8::Local<v8::Value> GetReference(JNIEnv *env, v8::Isolate *isolate, jobject javaObj);

    static void SetReference(JNIEnv *env, jobject javaObj, jlong value);

};

#endif //NODE_JSVALUE_H
