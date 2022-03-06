#include "long.h"

namespace Long {

static jclass class_;
static jmethodID int_value_method_id_;
static jmethodID value_of_method_id_;

bool Is(JNIEnv *env, jobject obj) {
  return env->IsInstanceOf(obj, class_);
}

jobject Of(JNIEnv *env, v8::Local<v8::Value> &value) {
  auto raw_value = value.As<v8::BigInt>()->Int64Value();
  return env->CallStaticObjectMethod(class_, value_of_method_id_, raw_value);
}

inline jlong Value(JNIEnv *env, jobject obj) {
  return env->CallLongMethod(obj, int_value_method_id_);
}

jint OnLoad(JNIEnv *env) {
  jclass clazz = env->FindClass("java/lang/Long");
  if (!clazz) {
    return JNI_ERR;
  }
  class_ = (jclass) env->NewGlobalRef(clazz);
  int_value_method_id_ = env->GetMethodID(clazz, "longValue", "()J");
  value_of_method_id_ = env->GetStaticMethodID(clazz, "valueOf", "(J)Ljava/lang/Long;");
  return JNI_OK;
}

}