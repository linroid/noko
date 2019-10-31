//
// Created by linroid on 2019/10/31.
//

#include "JSFunction.h"
#include "macros.h"
#include "JSValue.h"
#include "JSContext.h"

JNIClass functionClass;

jobject JSFunction::New(JNIEnv *env, NodeRuntime *runtime, v8::Local<v8::Value> &value) {
    auto reference = new v8::Persistent<v8::Value>(runtime->isolate, value);
    return env->NewObject(functionClass.clazz, functionClass.constructor, runtime->javaContext, reference);
}

jint JSFunction::OnLoad(JNIEnv *env) {
    jclass clazz = env->FindClass("com/linroid/knode/js/JSFunction");
    if (!clazz) {
        return JNI_ERR;
    }

    JNINativeMethod methods[] = {
            {"nativeCall", "(Lcom/linroid/knode/js/JSValue;[Lcom/linroid/knode/js/JSValue;)Lcom/linroid/knode/js/JSValue;", (void *) (Call)},
    };
    functionClass.clazz = (jclass) env->NewGlobalRef(clazz);
    functionClass.constructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/knode/js/JSContext;J)V");
    env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
    return JNI_OK;
}

jobject JSFunction::Call(JNIEnv *env, jobject thiz, jobject j_recv, jobjectArray j_parameters) {
    V8_ENV(env, thiz, v8::Function)
    int argc = env->GetArrayLength(j_parameters);
    v8::Local<v8::Value> *argv = new v8::Local<v8::Value>[argc];
    for (int i = 0; i < argc; ++i) {
        auto j_element = env->GetObjectArrayElement(j_parameters, i);
        argv[i] = JSValue::GetV8Value(env, runtime->isolate, j_element);
    }
    auto recv = JSValue::GetV8Value(env, runtime->isolate, j_recv);
    v8::TryCatch tryCatch(runtime->isolate);
    auto ret = that->Call(context, recv, argc, argv);
    if (tryCatch.HasCaught()) {
        runtime->throwJSError(env, tryCatch);
        return 0;
    }
    if (ret.IsEmpty()) {
        return 0;
    };
    auto retValue = ret.ToLocalChecked();
    return runtime->Wrap(env, retValue);
}
