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
  JavaVM *vm = nullptr;
  jclass clazz;
  jmethodID methodId;
public:
  NodeRuntime *runtime = nullptr;
  jobject that;

  JniCallback(NodeRuntime *runtime, JNIEnv *env, jobject that, jclass clazz, jmethodID methodId);

  ~JniCallback();

  void Call(const v8::FunctionCallbackInfo<v8::Value> &info);
};

#endif //DORA_JNICALLBACK_H
