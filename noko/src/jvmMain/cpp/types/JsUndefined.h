#ifndef NODE_JSUNDEFINED_H
#define NODE_JSUNDEFINED_H

#include <jni.h>
#include <v8.h>

class JsUndefined {
private:
  static jclass class_;
  static jmethodID constructor_id_;

public:

  inline static jobject ToJava(JNIEnv *env, jobject node, jlong pointer) {
    return env->NewObject(class_, constructor_id_, node, pointer);
  }

  static jint OnLoad(JNIEnv *env);
};

#endif //NODE_JSUNDEFINED_H
