#ifndef NOKO_UTIL_ENVHELPER_H
#define NOKO_UTIL_ENVHELPER_H

#include <jni.h>
#include <string>
#include "log.h"

class EnvHelper {
 private:
  JavaVM *vm_;
  JNIEnv *env_ = nullptr;
  int stat_ = JNI_OK;

 public:
  EnvHelper(JavaVM *vm);

  ~EnvHelper();

  bool HasAttached() {
    // `stat_ == JNI_EDETACHED` represents that `AttachCurrentThread` has been called in the constructor
    return stat_ == JNI_EDETACHED;
  }

  JNIEnv *operator->() {
    return env_;
  }

  JNIEnv *operator*() {
    return env_;
  }
};

#endif //NOKO_UTIL_ENVHELPER_H
