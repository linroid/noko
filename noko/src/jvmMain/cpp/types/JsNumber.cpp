#include "JsNumber.h"
#include "JsValue.h"

jclass JsNumber::jClazz;
jmethodID JsNumber::jConstructor;

jint JsNumber::OnLoad(JNIEnv *env) {
  jclass clazz = env->FindClass("com/linroid/noko/types/JsNumber");
  if (!clazz) {
    return JNI_ERR;
  }
  jClazz = (jclass) env->NewGlobalRef(clazz);
  jConstructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/noko/Node;JLjava/lang/Number;)V");
  JNINativeMethod methods[] = {
      {"nativeNew", "(D)V", (void *) JsNumber::New},
  };

  int rc = env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
  if (rc != JNI_OK) {
    return rc;
  }
  return JNI_OK;
}

void JsNumber::New(JNIEnv *env, jobject jThis, jdouble jData) {
  V8_SCOPE(env, jThis)
  auto value = v8::Number::New(node->isolate_, jData);
  auto result = new v8::Persistent<v8::Value>(node->isolate_, value);
  JsValue::SetPointer(env, jThis, (jlong) result);
}
