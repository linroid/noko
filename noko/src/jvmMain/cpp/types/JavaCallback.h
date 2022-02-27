#ifndef NOKO_JNICALLBACK_H
#define NOKO_JNICALLBACK_H

#include <jni.h>
#include "../Node.h"
#include "JsValue.h"

class JavaCallback {
private:
  JavaVM *vm_ = nullptr;
  jclass clazz_;
  jmethodID methodId_;
public:
  Node *node = nullptr;
  jobject that;

  JavaCallback(Node *node, JNIEnv *env, jobject that, jclass clazz, jmethodID methodId);

  ~JavaCallback();

  void Call(const v8::FunctionCallbackInfo<v8::Value> &info);
};

#endif //NOKO_JNICALLBACK_H
