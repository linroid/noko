//
// Created by linroid on 2019-10-21.
//

#include "JSBoolean.h"
#include "JSContext.h"
#include "macros.h"

JNIClass booleanClass;

jobject JSBoolean::Wrap(JNIEnv *env, NodeRuntime *runtime, v8::Persistent<v8::Value> *value) {
    return env->NewObject(booleanClass.clazz, booleanClass.constructor, runtime->jcontext, value);
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
    v8::Persistent<v8::Value> *result = nullptr;
    V8_SCOPE(env, jthis)
        auto value = v8::Boolean::New(runtime->isolate, jdata);
        result = new v8::Persistent<v8::Value>(runtime->isolate, value);
    V8_END()
    JSValue::SetReference(env, jthis, (jlong) result);
}
