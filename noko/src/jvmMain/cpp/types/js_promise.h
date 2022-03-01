#ifndef NOKO_PROMISE_H
#define NOKO_PROMISE_H

#include <jni.h>
#include "../node_runtime.h"

class JsPromise {
private:
  static jclass class_;
  static jmethodID init_method_id_;
  static jfieldID resolver_field_id_;

public:
  static jobject ToJava(JNIEnv *env, jobject node, jlong pointer) {
    return env->NewObject(class_, init_method_id_, node, pointer);
  }

  static jint OnLoad(JNIEnv *env);

  static JNICALL void New(JNIEnv *env, jobject j_this);

  static JNICALL void Reject(JNIEnv *env, jobject j_this, jobject j_error);

  static JNICALL void Resolve(JNIEnv *env, jobject j_this, jobject j_value);

  static JNICALL void Then(JNIEnv *env, jobject j_this, jobject j_callback);

  static JNICALL void Catch(JNIEnv *env, jobject j_this, jobject j_callback);
};


#endif //NOKO_PROMISE_H
