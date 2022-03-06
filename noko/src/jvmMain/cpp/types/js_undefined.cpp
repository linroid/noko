#include "js_undefined.h"

namespace JsUndefined {

jclass class_;
jmethodID init_method_id_;

jobject Of(JNIEnv *env, jobject node, jlong pointer) {
  return env->NewObject(class_, init_method_id_, node, pointer);
}

jint OnLoad(JNIEnv *env) {
  jclass clazz = env->FindClass("com/linroid/noko/types/JsUndefined");
  if (clazz == nullptr) {
    return JNI_ERR;
  }
  class_ = (jclass) env->NewGlobalRef(clazz);
  init_method_id_ = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/noko/Node;J)V");
  return JNI_OK;
}

}
