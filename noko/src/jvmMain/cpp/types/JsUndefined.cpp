#include "JsUndefined.h"

jclass JsUndefined::class_;
jmethodID JsUndefined::constructor_id_;

jint JsUndefined::OnLoad(JNIEnv *env) {
  jclass clazz = env->FindClass("com/linroid/noko/types/JsUndefined");
  if (clazz == nullptr) {
    return JNI_ERR;
  }
  class_ = (jclass) env->NewGlobalRef(clazz);
  constructor_id_ = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/noko/Node;J)V");
  return JNI_OK;
}
