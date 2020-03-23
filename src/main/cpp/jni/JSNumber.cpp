//
// Created by linroid on 2019-10-21.
//

#include "JSNumber.h"
#include "JSValue.h"

jclass JSNumber::jClazz;
jmethodID JSNumber::jConstructor;

jint JSNumber::OnLoad(JNIEnv *env) {
    jclass clazz = env->FindClass("com/linroid/knode/js/JSNumber");
    if (!clazz) {
        return JNI_ERR;
    }
    jClazz = (jclass) env->NewGlobalRef(clazz);
    jConstructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/knode/js/JSContext;J)V");
    JNINativeMethod methods[] = {
            {"nativeNew", "(D)V", (void *) JSNumber::New},
    };

    int rc = env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
    if (rc != JNI_OK) {
        return rc;
    }
    return JNI_OK;
}

void JSNumber::New(JNIEnv *env, jobject jThis, jdouble jData) {
    v8::Persistent<v8::Value> *result = nullptr;
    V8_SCOPE(env, jThis)
        auto value = v8::Number::New(runtime->_isolate, jData);
        result = new v8::Persistent<v8::Value>(runtime->_isolate, value);
    V8_END()
    JSValue::SetReference(env, jThis, (jlong) result);
}
