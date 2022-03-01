#ifndef NODE_JSNUMBER_H
#define NODE_JSNUMBER_H

#include <jni.h>
#include <v8.h>
#include "../macros.h"

class JsNumber {
private:
  static jclass class_;
  static jmethodID constructor_id_;
  static jclass double_class_;
  static jmethodID double_constructor_id_;

public:
  static jobject ToJava(JNIEnv *env, jobject node, jlong pointer, jdouble value) {
    jobject value_obj = env->NewObject(double_class_, double_constructor_id_, value);
    return env->NewObject(class_, constructor_id_, node, pointer, value_obj);
  }

  JNICALL static jlong New(JNIEnv *env, __attribute__((unused)) jclass clazz, jlong node_pointer, jdouble j_value);

  static jint OnLoad(JNIEnv *env);
};

#endif //NODE_JSNUMBER_H
