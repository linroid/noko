//
// Created by linroid on 2019-10-23.
//

#include "JSNull.h"
#include "JSValue.h"

jclass JSNull::jClazz;
jmethodID JSNull::jConstructor;

jint JSNull::OnLoad(JNIEnv *env) {
    jclass clazz = env->FindClass("com/linroid/knode/js/JSNull");
    if (!clazz) {
        return JNI_ERR;
    }
    jClazz = (jclass) env->NewGlobalRef(clazz);
    jConstructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/knode/js/JSContext;J)V");

    return JNI_OK;
}

void JSNull::New(JNIEnv *env, jobject jThis) {
    v8::Persistent<v8::Value> *result = nullptr;
    V8_SCOPE(env, jThis)
        auto value = v8::Null(isolate);
        result = new v8::Persistent<v8::Value>(runtime->isolate_, value);
    V8_END()
    JSValue::SetReference(env, jThis, (jlong) result);
}
