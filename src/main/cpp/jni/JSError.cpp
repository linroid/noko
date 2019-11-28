//
// Created by linroid on 2019/11/5.
//

#include "JSContext.h"
#include "JSValue.h"
#include "JSString.h"
#include "JSError.h"

jclass JSError::jClazz;
jmethodID JSError::jConstructor;
jclass JSError::jExceptionClazz;
jmethodID JSError::jExceptionConstructor;

jint JSError::OnLoad(JNIEnv *env) {
    jclass clazz = env->FindClass("com/linroid/knode/js/JSError");
    if (!clazz) {
        return JNI_ERR;
    }
    jClazz = (jclass) env->NewGlobalRef(clazz);
    jConstructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/knode/js/JSContext;J)V");

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
    jExceptionClazz = (jclass) env->NewGlobalRef(clazz);
    jExceptionConstructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/knode/js/JSError;)V");

    return JNI_OK;
}

void JSError::New(JNIEnv *env, jobject jThis, jstring jmessage) {
    auto messageChars = env->GetStringChars(jmessage, nullptr);
    const jint messageLen = env->GetStringLength(jmessage);
    v8::Persistent<v8::Value> *result = nullptr;

    V8_SCOPE(env, jThis)
        auto message = V8_STRING(messageChars, messageLen);
        auto value = v8::Exception::Error(message);
        result = new v8::Persistent<v8::Value>(runtime->isolate, value);
    V8_END()
    env->ReleaseStringChars(jmessage, messageChars);
    JSValue::SetReference(env, jThis, (jlong) result);
}

void JSError::Throw(JNIEnv *env, NodeRuntime *runtime, v8::Persistent<v8::Value> *error) {
    LOGE("Throw exception");
    auto jerror = env->NewObject(jClazz, jConstructor, runtime->jContext, (jlong) error);
    auto jexception = (jthrowable) env->NewObject(jExceptionClazz, jExceptionConstructor, jerror);
    env->Throw(jexception);
}
