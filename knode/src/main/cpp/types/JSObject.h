//
// Created by linroid on 2019-10-20.
//

#ifndef NODE_JSOBJECT_H
#define NODE_JSOBJECT_H

#include <jni.h>
#include "JSContext.h"

class JSObject {
private:
  static jclass jClazz;
  static jmethodID jConstructor;

public:
  inline static jobject Wrap(JNIEnv *env, NodeRuntime *runtime, v8::Persistent<v8::Value> *value) {
    return env->NewObject(jClazz, jConstructor, runtime->jContext_, (jlong) value);
  }

  static jint OnLoad(JNIEnv *env);

  static JNICALL jobject Get(JNIEnv *env, jobject jThis, jstring jKey);

  static JNICALL void Delete(JNIEnv *env, jobject jThis, jstring jKey);

  static JNICALL void Set(JNIEnv *env, jobject jThis, jstring jKey, jobject jValue);

  static JNICALL jboolean Has(JNIEnv *env, jobject jThis, jstring jKey);

  static JNICALL jobjectArray Keys(JNIEnv *env, jobject jThis);

  static JNICALL void Watch(JNIEnv *env, jobject jThis, jobjectArray jKeys, jobject jObserver);

  static JNICALL void New(JNIEnv *env, jobject jThis);
};

#endif //NODE_JSOBJECT_H
