#include "js_number.h"
#include "js_value.h"

jclass JsNumber::class_;
jmethodID JsNumber::init_method_id_;
jclass JsNumber::double_class_;
jmethodID JsNumber::double_init_method_id_;

jint JsNumber::OnLoad(JNIEnv *env) {
  jclass clazz = env->FindClass("com/linroid/noko/types/JsNumber");
  if (!clazz) {
    return JNI_ERR;
  }
  class_ = (jclass) env->NewGlobalRef(clazz);
  init_method_id_ = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/noko/Node;JLjava/lang/Number;)V");

  jclass double_class = env->FindClass("java/lang/Double");
  double_class_ = (jclass) env->NewGlobalRef(double_class);
  double_init_method_id_ = env->GetMethodID(double_class, "<init>", "(D)V");
  JNINativeMethod methods[] = {
      {"nativeNew", "(JD)J", (void *) JsNumber::New},
  };

  int result = env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
  if (result != JNI_OK) {
    return result;
  }
  return JNI_OK;
}

jlong JsNumber::New(JNIEnv *env, __attribute__((unused)) jclass clazz, jlong node_pointer, jdouble j_value) {
  V8_SCOPE_NEW(env, node_pointer)
  auto value = v8::Number::New(isolate, j_value);
  return reinterpret_cast<jlong>(new v8::Persistent<v8::Value>(isolate, value));
}

