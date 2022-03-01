#ifndef NODE_JSVALUE_H
#define NODE_JSVALUE_H

#include <jni.h>
#include "../node_runtime.h"

class JsValue {
private:
  static jfieldID pointer_field_id_;
  static jmethodID init_method_id_;
  static jmethodID get_node_pointer_id_;
public:
  static jclass class_;

  inline static jlong GetPointer(JNIEnv *env, jobject jObj) {
    return env->GetLongField(jObj, pointer_field_id_);
  }

  inline static void SetPointer(JNIEnv *env, jobject obj, jlong value) {
    env->SetLongField(obj, pointer_field_id_, value);
  }

  inline static jobject ToJava(JNIEnv *env, jobject node, jlong pointer) {
    return env->NewObject(class_, init_method_id_, node, pointer);
  }

  inline static v8::Persistent<v8::Value> *Unwrap(JNIEnv *env, jobject obj) {
    return reinterpret_cast<v8::Persistent<v8::Value> *>(GetPointer(env, obj));
  }

  inline static NodeRuntime *GetRuntime(JNIEnv *env, jobject obj) {
    auto pointer = env->CallLongMethod(obj, get_node_pointer_id_);
    if (pointer == 0) return nullptr;
    return reinterpret_cast<NodeRuntime *>(pointer);
  }

  JNICALL static jstring ToString(JNIEnv *env, jobject j_this);

  JNICALL static jstring TypeOf(JNIEnv *env, jobject j_this);

  JNICALL static jstring ToJson(JNIEnv *env, jobject j_this);

  JNICALL static jdouble ToNumber(JNIEnv *env, jobject j_this);

  JNICALL static void Dispose(JNIEnv *env, jobject j_this);

  static jint OnLoad(JNIEnv *env);
};

#endif //NODE_JSVALUE_H
