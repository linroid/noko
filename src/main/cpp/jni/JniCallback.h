//
// Created by linroid on 2019/11/3.
//

#ifndef DORA_JNICALLBACK_H
#define DORA_JNICALLBACK_H

#include <jni.h>
#include "../NodeRuntime.h"
#include "JSValue.h"

class JniCallback {
private:
    NodeRuntime *runtime;
    JavaVM *vm;
    jobject that;
    jmethodID methodId;
public:
    JniCallback(NodeRuntime *runtime, JNIEnv *env, jobject that, jmethodID methodId);

    ~JniCallback();

    void call(const v8::FunctionCallbackInfo<v8::Value> &info);
};

#endif //DORA_JNICALLBACK_H
