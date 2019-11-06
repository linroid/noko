//
// Created by linroid on 2019-10-23.
//

#include "JSArray.h"
#include "JSValue.h"
#include "JSContext.h"

static JNIClass arrayClass;

jint JSArray::Size(JNIEnv *env, jobject jthis) {
    int result = 0;
    V8_START(env, jthis, v8::Array)
        result = that->Length();
    V8_END();
    return result;
}

jobject JSArray::Wrap(JNIEnv *env, NodeRuntime *runtime, v8::Local<v8::Array> &value) {
    auto reference = new v8::Persistent<v8::Array>(runtime->isolate, value);
    return env->NewObject(arrayClass.clazz, arrayClass.constructor, runtime->jcontext, (jlong) reference);
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
