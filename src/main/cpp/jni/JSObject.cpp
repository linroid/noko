//
// Created by linroid on 2019-10-20.
//

#include <jni.h>
#include "util.h"
#include "env.h"
#include "macros.h"
#include "JSObject.h"
#include "JSValue.h"
#include "JSString.h"
#include "JSUndefined.h"
#include "JSNull.h"

JNIClass objectClass;

jobject JSObject::Wrap(JNIEnv *env, NodeRuntime *runtime, v8::Persistent<v8::Value> *value) {
    return env->NewObject(objectClass.clazz, objectClass.constructor, runtime->jcontext, (jlong) value);
}

JNICALL void JSObject::Set(JNIEnv *env, jobject jthis, jstring jkey, jobject jvalue) {
    const uint16_t *key = env->GetStringChars(jkey, nullptr);
    auto value = JSValue::Unwrap(env, jvalue);
    const jint keyLen = env->GetStringLength(jkey);
    V8_CONTEXT(env, jthis, v8::Object)
        assert(!that->IsNull());
        that->Set(V8_STRING(key, keyLen), value->Get(isolate));
    V8_END()
    env->ReleaseStringChars(jkey, key);
}

JNICALL jobject JSObject::Get(JNIEnv *env, jobject jthis, jstring jkey) {
    v8::Persistent<v8::Value> *result = nullptr;
    JSType type = None;
    const uint16_t *key = env->GetStringChars(jkey, nullptr);
    const jint keyLen = env->GetStringLength(jkey);
    V8_CONTEXT(env, jthis, v8::Object)
        auto value = that->Get(V8_STRING(key, keyLen));
        type = runtime->GetType(value);
        result = new v8::Persistent<v8::Value>(isolate, value);
    V8_END()
    env->ReleaseStringChars(jkey, key);
    return runtime->Wrap(env, result, type);
}

void JSObject::New(JNIEnv *env, jobject jthis) {
    v8::Persistent<v8::Value> *result = nullptr;
    V8_SCOPE(env, jthis)
        auto value = v8::Object::New(runtime->isolate);
        result = new v8::Persistent<v8::Value>(runtime->isolate, value);
    V8_END()
    JSValue::SetReference(env, jthis, (jlong) result);
}

jint JSObject::OnLoad(JNIEnv *env) {
    jclass clazz = env->FindClass("com/linroid/knode/js/JSObject");
    if (!clazz) {
        return JNI_ERR;
    }

    JNINativeMethod methods[] = {
            {"nativeGet", "(Ljava/lang/String;)Lcom/linroid/knode/js/JSValue;",  (void *) (JSObject::Get)},
            {"nativeSet", "(Ljava/lang/String;Lcom/linroid/knode/js/JSValue;)V", (void *) (JSObject::Set)},
            {"nativeNew", "()V",                                                 (void *) (JSObject::New)},
    };
    objectClass.clazz = (jclass) env->NewGlobalRef(clazz);
    objectClass.constructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/knode/js/JSContext;J)V");
    env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
    return JNI_OK;
}

