#ifndef NODE_JSNULL_H
#define NODE_JSNULL_H

#include <jni.h>
#include "../Noko.h"

class JSNull {
private:
  static jclass jClazz;
  static jmethodID jConstructor;

public:
  inline static jobject Wrap(JNIEnv *env, jobject jNoko, v8::Persistent<v8::Value> *value) {
    return env->NewObject(jClazz, jConstructor, jNoko, (jlong) value);
  }

  JNICALL static void New(JNIEnv *env, jobject jThis);

  static jint OnLoad(JNIEnv *env);
};

#endif //NODE_JSNULL_H