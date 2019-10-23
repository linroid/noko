//
// Created by linroid on 2019-10-20.
//

#include "JSUndefined.h"
#include "JSContext.h"

struct JSUndefinedClass {
    jclass clazz;
    jmethodID constructor;
} undefinedClass;

jobject JSUndefined::New(JNIEnv *env, V8Runtime *runtime) {
    return env->NewObject(undefinedClass.clazz, undefinedClass.constructor, runtime->javaContext);
}

jint JSUndefined::OnLoad(JNIEnv *env) {
    jclass clazz = env->FindClass("com/linroid/knode/js/JSUndefined");
    if (clazz == nullptr) {
        return JNI_ERR;
    }
    undefinedClass.clazz = (jclass) env->NewGlobalRef(clazz);
    undefinedClass.constructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/knode/js/JSContext;)V");
    return JNI_OK;
}
