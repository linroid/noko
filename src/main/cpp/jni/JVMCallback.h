//
// Created by linroid on 2019/11/3.
//

#ifndef DORA_JVMCALLBACK_H
#define DORA_JVMCALLBACK_H

#include <jni.h>
#include "../NodeRuntime.h"
#include "JSValue.h"

class JVMCallback {
private:
    JavaVM *vm;
    jobject that;
    jclass clazz;
    jmethodID methodId;
public:
    NodeRuntime *runtime;

    JVMCallback(JNIEnv *env, jobject that, jclass clazz, jmethodID methodId);

    ~JVMCallback();

    void Call(const v8::FunctionCallbackInfo<v8::Value> &info);
};

#endif //DORA_JVMCALLBACK_H
