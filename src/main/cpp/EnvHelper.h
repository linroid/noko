//
// Created by Lin Zhang on 9/3/21.
//
#include <jni.h>
#include "log.h"

#ifndef DORA_JS_JNIENV_H
#define DORA_JS_JNIENV_H


class EnvHelper {
private:
  JavaVM *vm_;
  JNIEnv *env_ = nullptr;
  int stat_ = JNI_EDETACHED;

public:
  EnvHelper(JavaVM *vm);

  ~EnvHelper();

  bool IsDetached() {
    return stat_ == JNI_EDETACHED;
  }

  JNIEnv *operator->() {
    return env_;
  }

  JNIEnv *operator*() {
    return env_;
  }
};


#endif //DORA_JS_JNIENV_H
