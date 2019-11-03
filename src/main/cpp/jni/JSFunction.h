//
// Created by linroid on 2019/10/31.
//

#ifndef DORA_JSFUNCTION_H
#define DORA_JSFUNCTION_H

#include <jni.h>
#include "../NodeRuntime.h"

class JSFunction {
public:
    static jobject Wrap(JNIEnv *env, NodeRuntime *runtime, v8::Local<v8::Function> &value);

    static jint OnLoad(JNIEnv *env);

    JNICALL static void New(JNIEnv *env, jobject jthis, jstring jname);

    JNICALL static jobject Call(JNIEnv *env, jobject jthis, jobject jreceiver, jobjectArray jparameters);
};
#endif //DORA_JSFUNCTION_H
