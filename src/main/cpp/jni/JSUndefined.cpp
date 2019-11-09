//
// Created by linroid on 2019-10-20.
//

#include "JSUndefined.h"
#include "JSContext.h"


jclass JSUndefined::jclazz;
jmethodID JSUndefined::jconstructor;

jint JSUndefined::OnLoad(JNIEnv *env) {
    jclass clazz = env->FindClass("com/linroid/knode/js/JSUndefined");
    if (clazz == nullptr) {
        return JNI_ERR;
    }
    jclazz = (jclass) env->NewGlobalRef(clazz);
    jconstructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/knode/js/JSContext;)V");
    return JNI_OK;
}
