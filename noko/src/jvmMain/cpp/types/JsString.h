#ifndef NODE_JSSTRING_H
#define NODE_JSSTRING_H

#include <jni.h>
#include "v8.h"
#include "../Node.h"
#include "JsValue.h"

class JsString {
private:
  static jclass jClazz;
  static jmethodID jConstructor;

public:
  JNICALL static void New(JNIEnv *env, jobject jThis, jstring jValue);

  inline static jobject Wrap(JNIEnv *env, jobject jNode, v8::Persistent<v8::Value> *jPointer, jstring jValue) {
    return env->NewObject(jClazz, jConstructor, jNode, (jlong) jPointer, jValue);
  }

  static jint OnLoad(JNIEnv *env);
};

#endif //NODE_JSSTRING_H
