#ifndef NOKO_ERROR_H
#define NOKO_ERROR_H

#include <jni.h>
#include <v8.h>

class JsError {
private:
  static jclass class_;
  static jmethodID init_method_id_;
  static jclass exception_class_;
  static jmethodID exception_init_method_id_;

public:
  static jobject ToJava(JNIEnv *env, jobject node, jlong pointer) {
    return env->NewObject(class_, init_method_id_, node, pointer);
  }

  JNICALL static void New(JNIEnv *env, jobject j_this, jstring j_message);

  static jthrowable ToException(JNIEnv *env, jobject j_error) {
    return (jthrowable) env->NewObject(exception_class_, exception_init_method_id_, j_error);
  }

  static jint OnLoad(JNIEnv *env);
};


#endif //NOKO_ERROR_H
