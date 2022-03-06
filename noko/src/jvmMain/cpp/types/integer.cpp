#include "integer.h"
#include "../util/log.h"

namespace Integer {

jclass class_;
jmethodID int_value_method_id_;
jmethodID value_of_method_id_;

bool Is(JNIEnv *env, jobject obj) {
  return env->IsInstanceOf(obj, class_);
}

jobject Of(JNIEnv *env, v8::Local<v8::Value> &value) {
  jint raw_value = value.As<v8::Int32>()->Value();
  return env->CallStaticObjectMethod(class_, value_of_method_id_, raw_value);
}

jint Value(JNIEnv *env, jobject obj) {
  return env->CallIntMethod(obj, int_value_method_id_);
}

jint OnLoad(JNIEnv *env) {
  jclass clazz = env->FindClass("java/lang/Integer");
  if (!clazz) {
    return JNI_ERR;
  }
  class_ = (jclass) env->NewGlobalRef(clazz);
  int_value_method_id_ = env->GetMethodID(clazz, "intValue", "()I");
  value_of_method_id_ = env->GetStaticMethodID(clazz, "valueOf", "(I)Ljava/lang/Integer;");
  return JNI_OK;
}

}