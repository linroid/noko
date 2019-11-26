//
// Created by linroid on 2019/11/8.
//

#ifndef DORA_JSPROMISE_H
#define DORA_JSPROMISE_H

#include <jni.h>
#include "../NodeRuntime.h"

class JSPromise {
private:
    static jclass jClazz;
    static jmethodID jConstructor;
    static jfieldID jResolver;

public:
    inline static jobject Wrap(JNIEnv *env, NodeRuntime *runtime, v8::Persistent<v8::Value> *value) {
        return env->NewObject(jClazz, jConstructor, runtime->jContext, (jlong) value);
    }

    static jint OnLoad(JNIEnv *env);

    static JNICALL void New(JNIEnv *env, jobject jThis);

    static JNICALL void Reject(JNIEnv *env, jobject jThis, jobject jerror);

    static JNICALL void Resolve(JNIEnv *env, jobject jThis, jobject jvalue);

    static JNICALL void Then(JNIEnv *env, jobject jThis, jobject jcallback);

    static JNICALL void Catch(JNIEnv *env, jobject jThis, jobject jcallback);
};


#endif //DORA_JSPROMISE_H
