//
// Created by linroid on 2019/11/8.
//

#ifndef DORA_JSPROMISE_H
#define DORA_JSPROMISE_H

#include <jni.h>
#include "../NodeRuntime.h"

class JSPromise {
private:
    static jclass jclazz;
    static jmethodID jconstructor;

public:
    inline static jobject Wrap(JNIEnv *env, NodeRuntime *runtime, v8::Persistent<v8::Value> *value) {
        return env->NewObject(jclazz, jconstructor, runtime->jcontext, (jlong) value);
    }

    static jint OnLoad(JNIEnv *env);

    static JNICALL void New(JNIEnv *env, jobject jthis);

    static JNICALL void Reject(JNIEnv *env, jobject jthis, jobject jerror);

    static JNICALL void Resolve(JNIEnv *env, jobject jthis, jobject jvalue);

    static JNICALL void Then(JNIEnv *env, jobject jthis, jobject jcallback);

    static JNICALL void Catch(JNIEnv *env, jobject jthis, jobject jcallback);
};


#endif //DORA_JSPROMISE_H
