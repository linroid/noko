#ifndef NODE_JSBOOLEAN_H
#define NODE_JSBOOLEAN_H

#include <jni.h>
#include "../Noko.h"
#include "JSValue.h"

class JSBoolean {
private:
  static jclass jClazz;
  static jmethodID jConstructor;

public:
  inline static jobject Wrap(JNIEnv *env, jobject jNoko, v8::Persistent<v8::Value> *value, bool data) {
    return env->NewObject(jClazz, jConstructor, jNoko, (jlong) value, data);
  }

  static jint OnLoad(JNIEnv *env);
};

#endif //NODE_JSBOOLEAN_H
