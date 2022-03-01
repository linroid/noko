#ifndef NODE_JSNULL_H
#define NODE_JSNULL_H

#include <jni.h>
#include "../node_runtime.h"

class JsNull {
private:
  static jclass class_;
  static jmethodID init_method_id_;

public:
  inline static jobject ToJava(JNIEnv *env, jobject node, jlong value) {
    return env->NewObject(class_, init_method_id_, node, value);
  }

  JNICALL static void New(JNIEnv *env, jobject j_this);

  static jint OnLoad(JNIEnv *env);
};

#endif //NODE_JSNULL_H
