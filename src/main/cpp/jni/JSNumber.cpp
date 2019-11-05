//
// Created by linroid on 2019-10-21.
//

#include "JSNumber.h"
#include "JSValue.h"

JNIClass numberClass;

jobject JSNumber::Wrap(JNIEnv *env, NodeRuntime *runtime, v8::Local<v8::Number> &value) {
    auto reference = new v8::Persistent<v8::Value>(runtime->isolate, value);
    return env->NewObject(numberClass.clazz, numberClass.constructor, runtime->jcontext, reference);
}

jint JSNumber::OnLoad(JNIEnv *env) {
    jclass clazz = env->FindClass("com/linroid/knode/js/JSNumber");
    if (!clazz) {
        return JNI_ERR;
    }
    numberClass.clazz = (jclass) env->NewGlobalRef(clazz);
    numberClass.constructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/knode/js/JSContext;J)V");
    JNINativeMethod methods[] = {
            {"nativeNew", "(D)V", (void *) JSNumber::New},
    };

    int rc = env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
    if (rc != JNI_OK) {
        return rc;
    }
    return JNI_OK;
}

void JSNumber::New(JNIEnv *env, jobject jthis, jdouble jdata) {
    auto runtime = JSContext::GetRuntime(env, jthis);
    auto value = v8::Number::New(runtime->isolate, jdata);
    auto reference = new v8::Persistent<v8::Value>(runtime->isolate, value);
    JSValue::SetReference(env, jthis, (jlong) reference);
}
