//
// Created by linroid on 2019-10-20.
//

#include <jni.h>
#include <node.h>
#include "JSObject.h"
#include "JSValue.h"
#include "JSString.h"
#include "JSUndefined.h"
#include "JSNull.h"

jclass JSObject::jClazz;
jmethodID JSObject::jConstructor;

JNICALL void JSObject::Set(JNIEnv *env, jobject jThis, jstring jKey, jobject jvalue) {
    const jchar *key = env->GetStringChars(jKey, nullptr);
    const jint keyLen = env->GetStringLength(jKey);

    auto value = JSValue::Unwrap(env, jvalue);
    V8_CONTEXT(env, jThis, v8::Object)
        // CHECK_NOT_NULL(*that);
        UNUSED(that->Set(context, V8_STRING(isolate, key, keyLen), value->Get(isolate)));
    V8_END()
    env->ReleaseStringChars(jKey, key);
}

JNICALL jobject JSObject::Get(JNIEnv *env, jobject jThis, jstring jKey) {
    v8::Persistent<v8::Value> *result = nullptr;
    JSType type = kNone;
    const uint16_t *key = env->GetStringChars(jKey, nullptr);
    const jint keyLen = env->GetStringLength(jKey);
    V8_CONTEXT(env, jThis, v8::Object)
        auto value = that->Get(context, V8_STRING(isolate, key, keyLen)).ToLocalChecked();
        type = NodeRuntime::GetType(value);
        result = new v8::Persistent<v8::Value>(isolate, value);
    V8_END()
    env->ReleaseStringChars(jKey, key);
    return runtime->Wrap(env, result, type);
}

jboolean JSObject::Has(JNIEnv *env, jobject jThis, jstring jKey) {
    bool result = false;
    const uint16_t *key = env->GetStringChars(jKey, nullptr);
    const jint keyLen = env->GetStringLength(jKey);
    V8_CONTEXT(env, jThis, v8::Object)
        result = that->Has(context, V8_STRING(isolate, key, keyLen)).ToChecked();
    V8_END()
    env->ReleaseStringChars(jKey, key);
    return static_cast<jboolean>(result);
}

void JSObject::New(JNIEnv *env, jobject jThis) {
    v8::Persistent<v8::Value> *result = nullptr;
    V8_SCOPE(env, jThis)
        auto value = v8::Object::New(runtime->isolate_);
        result = new v8::Persistent<v8::Value>(runtime->isolate_, value);
    V8_END()
    JSValue::SetReference(env, jThis, (jlong) result);
}

void JSObject::Delete(JNIEnv *env, jobject jThis, jstring jKey) {
    const uint16_t *key = env->GetStringChars(jKey, nullptr);
    const jint keyLen = env->GetStringLength(jKey);
    V8_CONTEXT(env, jThis, v8::Object)
        UNUSED(that->Delete(context, V8_STRING(isolate, key, keyLen)));
    V8_END()
    env->ReleaseStringChars(jKey, key);
}

jint JSObject::OnLoad(JNIEnv *env) {
    jclass clazz = env->FindClass("com/linroid/knode/js/JSObject");
    if (!clazz) {
        return JNI_ERR;
    }

    JNINativeMethod methods[] = {
            {"nativeGet",    "(Ljava/lang/String;)Lcom/linroid/knode/js/JSValue;",  (void *) (JSObject::Get)},
            {"nativeSet",    "(Ljava/lang/String;Lcom/linroid/knode/js/JSValue;)V", (void *) (JSObject::Set)},
            {"nativeNew",    "()V",                                                 (void *) (JSObject::New)},
            {"nativeHas",    "(Ljava/lang/String;)Z",                               (void *) (JSObject::Has)},
            {"nativeDelete", "(Ljava/lang/String;)V",                               (void *) (JSObject::Delete)},
    };
    jClazz = (jclass) env->NewGlobalRef(clazz);
    jConstructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/knode/js/JSContext;J)V");
    env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
    return JNI_OK;
}
