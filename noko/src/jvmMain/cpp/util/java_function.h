#ifndef NOKO_JAVAFUNCTION_H
#define NOKO_JAVAFUNCTION_H

#include <jni.h>
#include "../runtime.h"
#include "../types/js_value.h"

class JavaFunction {
private:
  jclass class_;
  jmethodID method_id_;
public:
  Runtime *runtime_ = nullptr;
  jobject that_;

  JavaFunction(Runtime *runtime, JNIEnv *env, jobject that, jclass clazz, jmethodID method_id);

  ~JavaFunction();

  void Call(const v8::FunctionCallbackInfo<v8::Value> &info);

  static void CleanupHook(void *arg);
};

#endif //NOKO_JAVAFUNCTION_H
