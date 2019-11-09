//
// Created by linroid on 2019/10/31.
//

#ifndef DORA_JSFUNCTION_H
#define DORA_JSFUNCTION_H

#include <jni.h>
#include "../NodeRuntime.h"

class JSFunction {
private:
    static jclass jclazz;
    static jmethodID jconstructor;
    static jmethodID jonCall;

public:
    inline static jobject Wrap(JNIEnv *env, NodeRuntime *runtime, v8::Persistent<v8::Value> *value) {
        return env->NewObject(jclazz, jconstructor, runtime->jcontext, (jlong) value);
    }

    static jint OnLoad(JNIEnv *env);

    JNICALL static void New(JNIEnv *env, jobject jthis, jstring jname);

    JNICALL static jobject Call(JNIEnv *env, jobject jthis, jobject jreceiver, jobjectArray jparameters);
};

#endif //DORA_JSFUNCTION_H
