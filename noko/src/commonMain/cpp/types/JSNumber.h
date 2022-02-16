#ifndef NODE_JSNUMBER_H
#define NODE_JSNUMBER_H

#include <jni.h>
#include <v8.h>

class JSNumber {
private:
  static jclass jClazz;
  static jmethodID jConstructor;

public:
  inline static jobject Wrap(JNIEnv *env, jobject jNoko, v8::Persistent<v8::Value> *value) {
    return env->NewObject(jClazz, jConstructor, jNoko, (jlong) value);
  }

  JNICALL static void New(JNIEnv *env, jobject jThis, jdouble jData);

  static jint OnLoad(JNIEnv *env);
};

#endif //NODE_JSNUMBER_H
