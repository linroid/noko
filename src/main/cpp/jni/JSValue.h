//
// Created by linroid on 2019-10-20.
//

#ifndef NODE_JSVALUE_H
#define NODE_JSVALUE_H

#include <jni.h>
#include "../NodeRuntime.h"

class JSValue {
private:
    static jfieldID jReference;
    static jfieldID jContext;
    static jmethodID jConstructor;
    static jmethodID jRuntime;
public:
    static jclass jClazz;

    inline static jlong GetReference(JNIEnv *env, jobject jObj) {
        return env->GetLongField(jObj, jReference);
    }

    inline static void SetReference(JNIEnv *env, jobject jObj, jlong value) {
        env->SetLongField(jObj, jReference, value);
    }

    inline static jobject Wrap(JNIEnv *env, NodeRuntime *runtime, v8::Persistent<v8::Value> *value) {
        return env->NewObject(jClazz, jConstructor, runtime->jContext, (jlong) value);
    }

    inline static v8::Persistent<v8::Value> *Unwrap(JNIEnv *env, jobject jObj) {
        return reinterpret_cast<v8::Persistent<v8::Value> *>(GetReference(env, jObj));
    }

    inline static NodeRuntime *GetRuntime(JNIEnv *env, jobject jObj) {
        auto ptr = env->CallLongMethod(jObj, jRuntime);
        return reinterpret_cast<NodeRuntime *>(ptr);
    }

    JNICALL static jstring ToString(JNIEnv *env, jobject jThis);

    JNICALL static jstring TypeOf(JNIEnv *env, jobject jThis);

    JNICALL static jstring ToJson(JNIEnv *env, jobject jThis);

    JNICALL static jdouble ToNumber(JNIEnv *env, jobject jThis);

    JNICALL static void Dispose(JNIEnv *env, jobject jThis);

    static jint OnLoad(JNIEnv *env);
};

#endif //NODE_JSVALUE_H
