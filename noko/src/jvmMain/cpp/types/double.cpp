#include "double.h"
namespace Double {

jclass class_;
jclass float_class_;
jmethodID int_value_method_id_;
jmethodID value_of_method_id_;

bool Is(JNIEnv *env, jobject obj) {
  return env->IsInstanceOf(obj, class_) || env->IsInstanceOf(obj, float_class_);
}

jobject Of(JNIEnv *env, v8::Local<v8::Value> &value) {
  jdouble raw_value = value.As<v8::Number>()->Value();
  return env->CallStaticObjectMethod(class_, value_of_method_id_, raw_value);
}

jdouble Value(JNIEnv *env, jobject obj) {
  auto value = env->CallDoubleMethod(obj, int_value_method_id_);
  return value;
}

jint OnLoad(JNIEnv *env) {
  jclass clazz = env->FindClass("java/lang/Double");
  if (!clazz) {
    return JNI_ERR;
  }
  class_ = (jclass) env->NewGlobalRef(clazz);
  float_class_ = (jclass) env->NewGlobalRef(env->FindClass("java/lang/Float"));
  jclass number_class = env->FindClass("java/lang/Number");
  int_value_method_id_ = env->GetMethodID(number_class, "doubleValue", "()D");
  value_of_method_id_ = env->GetStaticMethodID(clazz, "valueOf", "(D)Ljava/lang/Double;");
  return JNI_OK;
}

}