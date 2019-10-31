//
// Created by linroid on 2019-10-23.
//

#include "JSArray.h"
#include "JSValue.h"
#include "JSContext.h"

static JNIClass arrayClass;

jint JSArray::Size(JNIEnv *env, jobject thiz) {
    V8_ENV(env, thiz, v8::Array)
    // auto array = v8::Local<v8::Array>::Cast(value);
    return that->Length();
}

jobject JSArray::New(JNIEnv *env, NodeRuntime *runtime, v8::Local<v8::Array> &value) {
    auto reference = new v8::Persistent<v8::Array>(runtime->isolate, value);
    return env->NewObject(arrayClass.clazz, arrayClass.constructor, runtime->javaContext, (jlong) reference);
}

jint JSArray::OnLoad(JNIEnv *env) {
    jclass clazz = env->FindClass("com/linroid/knode/js/JSArray");
    if (!clazz) {
        return JNI_ERR;
    }

    JNINativeMethod methods[] = {
            {"nativeSize", "()I", (void *) (Size)},
    };
    arrayClass.clazz = (jclass) env->NewGlobalRef(clazz);
    arrayClass.constructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/knode/js/JSContext;J)V");
    env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
    return JNI_OK;
}
