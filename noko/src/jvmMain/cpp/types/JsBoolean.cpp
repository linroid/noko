#include "JsBoolean.h"

jclass JsBoolean::class_id_;
jmethodID JsBoolean::init_method_id_;

jint JsBoolean::OnLoad(JNIEnv *env) {
  jclass clazz = env->FindClass("com/linroid/noko/types/JsBoolean");
  if (!clazz) {
    return JNI_ERR;
  }
  class_id_ = (jclass) env->NewGlobalRef(clazz);
  init_method_id_ = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/noko/Node;JZ)V");
  return JNI_OK;
}
