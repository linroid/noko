#include "JsNull.h"
#include "JsValue.h"

jclass JsNull::class_;
jmethodID JsNull::init_method_id_;

jint JsNull::OnLoad(JNIEnv *env) {
  jclass clazz = env->FindClass("com/linroid/noko/types/JsNull");
  if (!clazz) {
    return JNI_ERR;
  }
  class_ = (jclass) env->NewGlobalRef(clazz);
  init_method_id_ = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/noko/Node;J)V");

  return JNI_OK;
}

void JsNull::New(JNIEnv *env, jobject j_this) {
  V8_SCOPE(env, j_this)
  auto value = v8::Null(isolate);
  auto result = new v8::Persistent<v8::Value>(runtime->isolate_, value);
  JsValue::SetPointer(env, j_this, (jlong) result);
}
