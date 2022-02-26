#ifndef NODE_JSNULL_H
#define NODE_JSNULL_H

#include <jni.h>
#include "../Node.h"

class JsNull {
private:
  static jclass jClazz;
  static jmethodID jConstructor;

public:
  inline static jobject Wrap(JNIEnv *env, jobject jNode, v8::Persistent<v8::Value> *value) {
    return env->NewObject(jClazz, jConstructor, jNode, (jlong) value);
  }

  JNICALL static void New(JNIEnv *env, jobject jThis);

  static jint OnLoad(JNIEnv *env);
};

#endif //NODE_JSNULL_H
