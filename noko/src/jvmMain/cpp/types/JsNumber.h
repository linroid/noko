#ifndef NODE_JSNUMBER_H
#define NODE_JSNUMBER_H

#include <jni.h>
#include <v8.h>

class JsNumber {
private:
  static jclass jClazz;
  static jmethodID jConstructor;

public:
  inline static jobject Wrap(JNIEnv *env, jobject jNode, v8::Persistent<v8::Value> *value) {
    return env->NewObject(jClazz, jConstructor, jNode, (jlong) value);
  }

  JNICALL static void New(JNIEnv *env, jobject jThis, jdouble jData);

  static jint OnLoad(JNIEnv *env);
};

#endif //NODE_JSNUMBER_H
