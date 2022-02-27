#ifndef NOKO_JSERROR_H
#define NOKO_JSERROR_H

#include <jni.h>
#include <v8.h>

class JsError {
private:
  static jclass jClazz;
  static jmethodID jConstructor;
  static jclass jExceptionClazz;
  static jmethodID jExceptionConstructor;

public:
  inline static jobject Wrap(JNIEnv *env, jobject jNode, v8::Persistent<v8::Value> *value) {
    return env->NewObject(jClazz, jConstructor, jNode, (jlong) value);
  }

  JNICALL static void New(JNIEnv *env, jobject jThis, jstring jMessage);

  static jthrowable ToException(JNIEnv *env, jobject jError) {
    return (jthrowable) env->NewObject(jExceptionClazz, jExceptionConstructor, jError);
  }

  static jint OnLoad(JNIEnv *env);
};


#endif //NOKO_JSERROR_H
