//
// Created by linroid on 2019/11/3.
//

#ifndef DORA_JNICALLBACK_H
#define DORA_JNICALLBACK_H

#include <jni.h>
#include "../NodeRuntime.h"
#include "JSValue.h"

class JavaCallback {
private:
  JavaVM *vm_ = nullptr;
  jclass clazz_;
  jmethodID methodId_;
public:
  NodeRuntime *runtime = nullptr;
  jobject that;

  JavaCallback(NodeRuntime *runtime, JNIEnv *env, jobject that, jclass clazz, jmethodID methodId);

  ~JavaCallback();

  void Call(const v8::FunctionCallbackInfo<v8::Value> &info);
};

#endif //DORA_JNICALLBACK_H
