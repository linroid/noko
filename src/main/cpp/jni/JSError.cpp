//
// Created by linroid on 2019/11/5.
//

#include "JSContext.h"
#include "JSValue.h"
#include "JSString.h"
#include "JSError.h"
#include "macros.h"

JNIClass errorClass;
JNIClass exceptionClass;

jint JSError::OnLoad(JNIEnv *env) {
    jclass clazz = env->FindClass("com/linroid/knode/js/JSError");
    if (!clazz) {
        return JNI_ERR;
    }
    errorClass.clazz = (jclass) env->NewGlobalRef(clazz);
    errorClass.constructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/knode/js/JSContext;J)V");

    JNINativeMethod methods[] = {
            {"nativeNew", "(Ljava/lang/String;)V", (void *) JSError::New},
    };

    int rc = env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
    if (rc != JNI_OK) {
        return rc;
    }


    clazz = env->FindClass("com/linroid/knode/JSException");
    if (!clazz) {
        return JNI_ERR;
    }
    exceptionClass.clazz = (jclass) env->NewGlobalRef(clazz);
    exceptionClass.constructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/knode/js/JSError;)V");

    return JNI_OK;
}

void JSError::New(JNIEnv *env, jobject jthis, jstring jmessage) {
    auto runtime = JSContext::GetRuntime(env, jthis);
    auto message = JSString::ToV8(env, runtime->isolate, jmessage);
    auto value = v8::Exception::Error(message);
    auto reference = new v8::Persistent<v8::Value>(runtime->isolate, value);
    JSValue::SetReference(env, jthis, (jlong) reference);
}

jobject JSError::Wrap(JNIEnv *env, NodeRuntime *runtime, v8::Local<v8::Value> &value) {
    auto reference = new v8::Persistent<v8::Value>(runtime->isolate, value);
    return env->NewObject(errorClass.clazz, errorClass.constructor, runtime->jcontext, reference);
}

void JSError::Throw(JNIEnv *env, NodeRuntime *runtime, v8::TryCatch &tryCatch) {
    auto exception = tryCatch.Exception();
    v8::String::Utf8Value exceptionStr(exception);
    auto error = Wrap(env, runtime, exception);
    auto jexception = (jthrowable) env->NewObject(exceptionClass.clazz, exceptionClass.constructor, error);
    env->Throw(jexception);
}
