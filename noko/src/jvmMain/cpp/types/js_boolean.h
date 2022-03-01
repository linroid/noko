#ifndef NODE_JSBOOLEAN_H
#define NODE_JSBOOLEAN_H

#include <jni.h>
#include "../node_runtime.h"
#include "js_value.h"

class JsBoolean {
private:
  static jclass class_id_;
  static jmethodID init_method_id_;

public:
  static jobject ToJava(JNIEnv *env, jobject node, jlong pointer, bool value) {
    return env->NewObject(class_id_, init_method_id_, node, pointer, value);
  }

  static jint OnLoad(JNIEnv *env);
};

#endif //NODE_JSBOOLEAN_H
