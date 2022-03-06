#ifndef NODE_JSARRAY_H
#define NODE_JSARRAY_H

#include <jni.h>

class JsArray {
private:
  static jclass class_;
  static jmethodID init_method_id_;

public:
  inline static jobject Of(JNIEnv *env, jobject node, jlong pointer) {
    return env->NewObject(class_, init_method_id_, node, pointer);
  }

  JNICALL static jint Size(JNIEnv *env, jobject j_this);

  JNICALL static void New(JNIEnv *env, jobject j_this);

  JNICALL static jobject Get(JNIEnv *env, jobject j_this, jint j_index);

  JNICALL static jboolean Add(JNIEnv *env, jobject j_this, jobject j_element);

  JNICALL static jboolean AddAll(JNIEnv *env, jobject j_this, jobjectArray j_elements);

  static jint OnLoad(JNIEnv *env);
};


#endif //NODE_JSARRAY_H
