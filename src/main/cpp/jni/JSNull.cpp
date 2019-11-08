//
// Created by linroid on 2019-10-23.
//

#include "JSNull.h"
#include "macros.h"
#include "JSValue.h"

JNIClass nullClass;

jobject JSNull::Wrap(JNIEnv *env, NodeRuntime *runtime) {
    return env->NewObject(nullClass.clazz, nullClass.constructor, runtime->jcontext);
}

jint JSNull::OnLoad(JNIEnv *env) {
    jclass clazz = env->FindClass("com/linroid/knode/js/JSNull");
    if (!clazz) {
        return JNI_ERR;
    }
    nullClass.clazz = (jclass) env->NewGlobalRef(clazz);
    nullClass.constructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/knode/js/JSContext;)V");

    JNINativeMethod methods[] = {
            {"nativeNew", "()V", (void *) (JSNull::New)},
    };
    env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
    return JNI_OK;
}

void JSNull::New(JNIEnv *env, jobject jthis) {
    v8::Persistent<v8::Value> *result = nullptr;
    V8_SCOPE(env, jthis)
        auto value = v8::Null(isolate);
        result = new v8::Persistent<v8::Value>(runtime->isolate, value);
    V8_END()
    JSValue::SetReference(env, jthis, (jlong) result);
}
