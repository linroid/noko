//
// Created by linroid on 2019-10-23.
//

#include "JSNull.h"

JNIClass nullClass;

jobject JSNull::New(JNIEnv *env, NodeRuntime *runtime) {
    return env->NewObject(nullClass.clazz, nullClass.constructor, runtime->javaContext);
}

jint JSNull::OnLoad(JNIEnv *env) {
    jclass clazz = env->FindClass("com/linroid/knode/js/JSNull");
    if (!clazz) {
        return JNI_ERR;
    }
    nullClass.clazz = (jclass) env->NewGlobalRef(clazz);
    nullClass.constructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/knode/js/JSContext;)V");

    // JNINativeMethod methods[] = {
    //         {"nativeSize", "()I", (void *) (Size)},
    // };
    // env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
    return JNI_OK;
}
