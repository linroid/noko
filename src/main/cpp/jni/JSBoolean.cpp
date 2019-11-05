//
// Created by linroid on 2019-10-21.
//

#include "JSBoolean.h"
#include "JSContext.h"

JNIClass booleanClass;

jobject JSBoolean::Wrap(JNIEnv *env, NodeRuntime *runtime, v8::Local<v8::Value> &value) {
    auto reference = new v8::Persistent<v8::Value>(runtime->isolate, value);
    return env->NewObject(booleanClass.clazz, booleanClass.constructor, runtime->jcontext, reference, value->BooleanValue());
}

jint JSBoolean::OnLoad(JNIEnv *env) {
    jclass clazz = env->FindClass("com/linroid/knode/js/JSBoolean");
    if (!clazz) {
        return JNI_ERR;
    }
    booleanClass.clazz = (jclass) env->NewGlobalRef(clazz);
    booleanClass.constructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/knode/js/JSContext;JZ)V");

    JNINativeMethod methods[] = {
            {"nativeNew", "(Z)V", (void *) JSBoolean::New},
    };

    int rc = env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
    if (rc != JNI_OK) {
        return rc;
    }
    return JNI_OK;
}

void JSBoolean::New(JNIEnv *env, jobject jthis, jboolean jdata) {
    auto runtime = JSContext::GetRuntime(env, jthis);
    auto value = v8::Boolean::New(runtime->isolate, jdata);
    auto reference = new v8::Persistent<v8::Value>(runtime->isolate, value);
    JSValue::SetReference(env, jthis, (jlong) reference);
}
