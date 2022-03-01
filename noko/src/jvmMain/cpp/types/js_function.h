#ifndef NOKO_FUNCTION_H
#define NOKO_FUNCTION_H

#include <jni.h>
#include "../node_runtime.h"

class JsFunction {
private:
  static jclass class_;
  static jmethodID init_method_id_;
  static jmethodID call_method_id_;

public:
  static jobject ToJava(JNIEnv *env, jobject node, jlong pointer) {
    return env->NewObject(class_, init_method_id_, node, pointer);
  }

  static jint OnLoad(JNIEnv *env);

  JNICALL static void New(JNIEnv *env, jobject j_this, jstring jName);

  JNICALL static jobject Call(JNIEnv *env, jobject j_this, jobject j_receiver, jobjectArray j_parameters);
};

#endif //NOKO_FUNCTION_H
