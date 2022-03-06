#ifndef NOKO_JNICALLBACK_H
#define NOKO_JNICALLBACK_H

#include <jni.h>
#include "../runtime.h"
#include "../types/js_value.h"

class JavaCallback {
private:
  jclass class_;
  jmethodID method_id_;
public:
  Runtime *runtime_ = nullptr;
  jobject that_;

  JavaCallback(Runtime *runtime, JNIEnv *env, jobject that, jclass clazz, jmethodID method_id);

  ~JavaCallback();

  void Call(const v8::FunctionCallbackInfo<v8::Value> &info);
};

#endif //NOKO_JNICALLBACK_H
