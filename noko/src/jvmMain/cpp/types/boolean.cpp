#include "boolean.h"

namespace Boolean {

jclass class_;
jobject true_;
jobject false_;
jmethodID boolean_value_method_id_;

jint OnLoad(JNIEnv *env) {
  jclass clazz = env->FindClass("java/lang/Boolean");
  if (!clazz) {
    return JNI_ERR;
  }
  class_ = (jclass) env->NewGlobalRef(clazz);
  jmethodID value_of_method_id = env->GetStaticMethodID(clazz, "valueOf", "(Z)Ljava/lang/Boolean;");
  boolean_value_method_id_ = env->GetMethodID(clazz, "booleanValue", "()Z");

  true_ = env->NewGlobalRef(env->CallStaticObjectMethod(clazz, value_of_method_id, true));
  false_ = env->NewGlobalRef(env->CallStaticObjectMethod(clazz, value_of_method_id, false));
  return JNI_OK;
}

jboolean Value(JNIEnv *env, jobject obj) {
  return env->CallBooleanMethod(obj, boolean_value_method_id_);
}

jobject Of(v8::Local<v8::Value> &value) {
  return value.As<v8::Boolean>()->Value() ? true_ : false_;
}

bool Is(JNIEnv *env, jobject obj) {
  return env->IsInstanceOf(obj, class_);
}

}