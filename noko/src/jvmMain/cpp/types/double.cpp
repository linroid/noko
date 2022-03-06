#include "double.h"

namespace Double {

jclass class_;
jmethodID int_value_method_id_;
jmethodID value_of_method_id_;

bool Is(JNIEnv *env, jobject obj) {
  return env->IsInstanceOf(obj, class_);
}

jobject Of(JNIEnv *env, v8::Local<v8::Value> &value) {
  auto raw_value = value.As<v8::Number>()->Value();
  return env->CallStaticObjectMethod(class_, value_of_method_id_, raw_value);
}

inline jdouble Value(JNIEnv *env, jobject obj) {
  return env->CallDoubleMethod(obj, int_value_method_id_);
}

jint OnLoad(JNIEnv *env) {
  jclass clazz = env->FindClass("java/lang/Double");
  if (!clazz) {
    return JNI_ERR;
  }
  class_ = (jclass) env->NewGlobalRef(clazz);
  int_value_method_id_ = env->GetMethodID(clazz, "doubleValue", "()D");
  value_of_method_id_ = env->GetStaticMethodID(clazz, "valueOf", "(D)Ljava/lang/Double;");
  return JNI_OK;
}

}