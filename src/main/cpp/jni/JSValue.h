//
// Created by linroid on 2019-10-20.
//

#ifndef NODE_JSVALUE_H
#define NODE_JSVALUE_H

#include <jni.h>
#include "../NodeRuntime.h"

class JSValue {
public:
    static jobject Wrap(JNIEnv *env, NodeRuntime *runtime, v8::Persistent<v8::Value> *value);

    JNICALL static jstring ToString(JNIEnv *env, jobject jthis);

    JNICALL static jstring TypeOf(JNIEnv *env, jobject jthis);

    JNICALL static jstring ToJson(JNIEnv *env, jobject jthis);

    JNICALL static jdouble ToNumber(JNIEnv *env, jobject jthis);

    JNICALL static void Dispose(JNIEnv *env, jobject jthis);

    static jint OnLoad(JNIEnv *env);

    static jclass &JVMClass();

    static jlong GetReference(JNIEnv *env, jobject jobj);

    static inline v8::Persistent<v8::Value> *Unwrap(JNIEnv *env, jobject jobj) {
        return reinterpret_cast<v8::Persistent<v8::Value> *>(GetReference(env, jobj));
    }

    static void SetReference(JNIEnv *env, jobject jobj, jlong value);

    static NodeRuntime *GetRuntime(JNIEnv *env, jobject jobj);
};

#endif //NODE_JSVALUE_H
