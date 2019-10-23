//
// Created by linroid on 2019-10-21.
//

#include "JSBoolean.h"

struct JSBooleanClass {
    jclass clazz;
    jmethodID constructor;
} booleanClass;

jobject JSBoolean::New(JNIEnv *env, V8Runtime *runtime) {
    return env->NewObject(booleanClass.clazz, booleanClass.constructor, reinterpret_cast<jlong>(runtime));
}

jint JSBoolean::OnLoad(JNIEnv *env) {
    jclass clazz = env->FindClass("com/linroid/knode/js/JSBoolean");
    if (!clazz) {
        return JNI_ERR;
    }
    booleanClass.clazz = (jclass) env->NewGlobalRef(clazz);
    booleanClass.constructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/knode/js/JSContext;J)V");
    return JNI_OK;
}
