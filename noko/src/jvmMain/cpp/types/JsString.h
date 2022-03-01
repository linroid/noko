#ifndef NODE_JSSTRING_H
#define NODE_JSSTRING_H

#include <jni.h>
#include "v8.h"
#include "../NodeRuntime.h"
#include "JsValue.h"

class JsString {
private:
  static jclass class_;
  static jmethodID init_method_id;

public:
  static jobject ToJava(JNIEnv *env, jobject node, jlong pointer, jstring value) {
    return env->NewObject(class_, init_method_id, node, pointer, value);
  }

  JNICALL static void New(JNIEnv *env, jobject j_this, jstring j_value);

  static jint OnLoad(JNIEnv *env);
};

#endif //NODE_JSSTRING_H
