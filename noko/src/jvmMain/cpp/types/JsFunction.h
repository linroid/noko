#ifndef NOKO_JSFUNCTION_H
#define NOKO_JSFUNCTION_H

#include <jni.h>
#include "../Node.h"

class JsFunction {
private:
  static jclass jClazz;
  static jmethodID jConstructor;
  static jmethodID jCall;

public:
  inline static jobject Wrap(JNIEnv *env, jobject jNode, v8::Persistent<v8::Value> *value) {
    return env->NewObject(jClazz, jConstructor, jNode, (jlong) value);
  }

  static jint OnLoad(JNIEnv *env);

  JNICALL static void New(JNIEnv *env, jobject jThis, jstring jName);

  JNICALL static jobject Call(JNIEnv *env, jobject jThis, jobject jReceiver, jobjectArray jParameters);
};

#endif //NOKO_JSFUNCTION_H
