#ifndef NOKO_JNICALLBACK_H
#define NOKO_JNICALLBACK_H

#include <jni.h>
#include "../NodeRuntime.h"
#include "JsValue.h"

class JavaCallback {
private:
  jclass class_;
  jmethodID method_id_;
public:
  NodeRuntime *runtime_ = nullptr;
  jobject that_;

  JavaCallback(NodeRuntime *node, JNIEnv *env, jobject that, jclass clazz, jmethodID method_id);

  ~JavaCallback();

  void Call(const v8::FunctionCallbackInfo<v8::Value> &info);
};

#endif //NOKO_JNICALLBACK_H
