#ifndef NODE_JSSTRING_H
#define NODE_JSSTRING_H

#include <jni.h>
#include "v8.h"
#include "../Noko.h"
#include "JSValue.h"

class JSString {
private:
  static jclass jClazz;
  static jmethodID jConstructor;

public:
  JNICALL static void New(JNIEnv *env, jobject jThis, jstring jContent);

  inline static jobject Wrap(JNIEnv *env, jobject jNoko, v8::Persistent<v8::Value> *value) {
    return env->NewObject(jClazz, jConstructor, jNoko, (jlong) value);
  }

  static jint OnLoad(JNIEnv *env);
};

#endif //NODE_JSSTRING_H
