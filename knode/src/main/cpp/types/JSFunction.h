//
// Created by linroid on 2019/10/31.
//

#ifndef DORA_JSFUNCTION_H
#define DORA_JSFUNCTION_H

#include <jni.h>
#include "../NodeRuntime.h"

class JSFunction {
private:
  static jclass jClazz;
  static jmethodID jConstructor;
  static jmethodID jCall;

public:
  inline static jobject Wrap(JNIEnv *env, NodeRuntime *runtime, v8::Persistent<v8::Value> *value) {
    return env->NewObject(jClazz, jConstructor, runtime->jContext_, (jlong) value);
  }

  static jint OnLoad(JNIEnv *env);

  JNICALL static void New(JNIEnv *env, jobject jThis, jstring jName);

  JNICALL static jobject Call(JNIEnv *env, jobject jThis, jobject jReceiver, jobjectArray jParameters);
};

#endif //DORA_JSFUNCTION_H
