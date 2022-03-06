#ifndef NODE_JSOBJECT_H
#define NODE_JSOBJECT_H

#include <jni.h>

class JsObject {
private:
  static jclass class_;
  static jclass string_class_;
  static jmethodID init_method_id_;

public:
  static jobject Of(JNIEnv *env, jobject node, jlong pointer) {
    return env->NewObject(class_, init_method_id_, node, pointer);
  }

  static jint OnLoad(JNIEnv *env);

  static JNICALL jobject Get(JNIEnv *env, jobject j_this, jstring j_key);

  static JNICALL void Delete(JNIEnv *env, jobject j_this, jstring j_key);

  static JNICALL void Set(JNIEnv *env, jobject j_this, jstring j_key, jobject j_value);

  static JNICALL jboolean Has(JNIEnv *env, jobject j_this, jstring j_key);

  static JNICALL jobjectArray Keys(JNIEnv *env, jobject j_this);

  static JNICALL void Watch(JNIEnv *env, jobject j_this, jobjectArray j_keys, jobject j_observer);

  static JNICALL void New(JNIEnv *env, jobject j_this);
};

#endif //NODE_JSOBJECT_H
