#ifndef NODE_JSARRAY_H
#define NODE_JSARRAY_H

#include <jni.h>
#include "../NodeRuntime.h"

class JSArray {
private:
  static jclass jClazz;
  static jmethodID jConstructor;

public:
  inline static jobject Wrap(JNIEnv *env, NodeRuntime *runtime, v8::Persistent<v8::Value> *value) {
    return env->NewObject(jClazz, jConstructor, runtime->jContext_, (jlong) value);
  }

  JNICALL static jint Size(JNIEnv *env, jobject jThis);

  JNICALL static void New(JNIEnv *env, jobject jThis);

  JNICALL static jobject Get(JNIEnv *env, jobject jThis, jint jIndex);

  JNICALL static jboolean Add(JNIEnv *env, jobject jThis, jobject jElement);

  JNICALL static jboolean AddAll(JNIEnv *env, jobject jThis, jobjectArray jElements);

  static jint OnLoad(JNIEnv *env);
};


#endif //NODE_JSARRAY_H
