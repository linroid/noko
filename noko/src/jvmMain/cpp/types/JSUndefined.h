#ifndef NODE_JSUNDEFINED_H
#define NODE_JSUNDEFINED_H

#include <jni.h>
#include <v8.h>

class JSUndefined {
private:
  static jclass jClazz;
  static jmethodID jConstructor;

public:
  inline static jobject Wrap(JNIEnv *env, jobject jNoko, v8::Persistent <v8::Value> *value) {
    return env->NewObject(jClazz, jConstructor, jNoko, (jlong) value);
  }

  static jint OnLoad(JNIEnv *env);
};

#endif //NODE_JSUNDEFINED_H
