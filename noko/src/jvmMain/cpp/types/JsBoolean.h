#ifndef NODE_JSBOOLEAN_H
#define NODE_JSBOOLEAN_H

#include <jni.h>
#include "../Node.h"
#include "JsValue.h"

class JsBoolean {
private:
  static jclass jClazz;
  static jmethodID jConstructor;

public:
  inline static jobject Wrap(JNIEnv *env, jobject jNode, v8::Persistent<v8::Value> *value, bool data) {
    return env->NewObject(jClazz, jConstructor, jNode, (jlong) value, data);
  }

  static jint OnLoad(JNIEnv *env);
};

#endif //NODE_JSBOOLEAN_H
