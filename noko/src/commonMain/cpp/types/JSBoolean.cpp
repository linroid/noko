#include "JSBoolean.h"
#include "JSContext.h"

jclass JSBoolean::jClazz;
jmethodID JSBoolean::jConstructor;

jint JSBoolean::OnLoad(JNIEnv *env) {
  jclass clazz = env->FindClass("com/linroid/noko/types/JSBoolean");
  if (!clazz) {
    return JNI_ERR;
  }
  jClazz = (jclass) env->NewGlobalRef(clazz);
  jConstructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/noko/types/JSContext;JZ)V");
  return JNI_OK;
}
