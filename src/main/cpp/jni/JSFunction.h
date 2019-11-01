//
// Created by linroid on 2019/10/31.
//

#ifndef DORA_JSFUNCTION_H
#define DORA_JSFUNCTION_H

#include <jni.h>
#include "../NodeRuntime.h"

class JSFunction {
public:
    static jobject New(JNIEnv *env, NodeRuntime *runtime, v8::Local<v8::Value> &value);

    static jint OnLoad(JNIEnv *env);

    JNICALL static void Init(JNIEnv *env, jobject thiz);

    JNICALL static jobject Call(JNIEnv *env, jobject thiz, jobject j_recv, jobjectArray j_parameters);
};
#endif //DORA_JSFUNCTION_H
