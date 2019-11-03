//
// Created by linroid on 2019/11/3.
//

#ifndef DORA_JAVACALLBACK_H
#define DORA_JAVACALLBACK_H

#include <jni.h>
#include "../NodeRuntime.h"
#include "JSValue.h"

class JavaCallback {
private:
    NodeRuntime *runtime;
    JavaVM *vm;
    jobject that;
    jclass clazz;
    jmethodID methodId;
public:
    JavaCallback(NodeRuntime *runtime, JNIEnv *env, jobject that, jclass clazz, jmethodID methodId);

    ~JavaCallback();

    void Call(const v8::FunctionCallbackInfo<v8::Value> &info);
};

#endif //DORA_JAVACALLBACK_H
