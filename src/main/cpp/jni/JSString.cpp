//
// Created by linroid on 2019-10-21.
//

#include <string>
#include "JSString.h"
#include "JSValue.h"
#include "JSContext.h"

jclass JSString::jClazz;
jmethodID JSString::jConstructor;

jint JSString::OnLoad(JNIEnv *env) {
    jclass clazz = env->FindClass("com/linroid/knode/js/JSString");
    if (!clazz) {
        return JNI_ERR;
    }
    jClazz = (jclass) env->NewGlobalRef(clazz);
    jConstructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/knode/js/JSContext;J)V");

    JNINativeMethod methods[] = {
            {"nativeNew", "(Ljava/lang/String;)V", (void *) JSString::New},
    };

    int rc = env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
    if (rc != JNI_OK) {
        return rc;
    }

    return JNI_OK;
}

void JSString::New(JNIEnv *env, jobject jThis, jstring jcontent) {
    v8::Persistent<v8::Value> *result = nullptr;
    const uint16_t *content = env->GetStringChars(jcontent, nullptr);
    const jint contentLen = env->GetStringLength(jcontent);
    V8_SCOPE(env, jThis)
        result = new v8::Persistent<v8::Value>(isolate, V8_STRING(content, contentLen));
    V8_END()
    env->ReleaseStringChars(jcontent, content);
    JSValue::SetReference(env, jThis, (jlong) result);
}
