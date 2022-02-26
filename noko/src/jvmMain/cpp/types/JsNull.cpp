#include "JsNull.h"
#include "JsValue.h"

jclass JsNull::jClazz;
jmethodID JsNull::jConstructor;

jint JsNull::OnLoad(JNIEnv *env) {
  jclass clazz = env->FindClass("com/linroid/noko/types/JsNull");
  if (!clazz) {
    return JNI_ERR;
  }
  jClazz = (jclass) env->NewGlobalRef(clazz);
  jConstructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/noko/Node;J)V");

  return JNI_OK;
}

void JsNull::New(JNIEnv *env, jobject jThis) {
  V8_SCOPE(env, jThis)
  auto value = v8::Null(isolate);
  auto result = new v8::Persistent<v8::Value>(node->isolate_, value);
  JsValue::SetPointer(env, jThis, (jlong) result);
}
