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

jobject JSObject::Wrap(JNIEnv *env, NodeRuntime *runtime, v8::Local<v8::Value> &value) {
    auto reference = new v8::Persistent<v8::Value>(runtime->isolate, value);
    return env->NewObject(objectClass.clazz, objectClass.constructor, runtime->jcontext, (jlong) reference);
}

JNICALL void JSObject::Set(JNIEnv *env, jobject jthis, jstring j_key, jobject j_value) {
    V8_START(env, jthis, v8::Object)
        auto target = reinterpret_cast<v8::Persistent<v8::Value> *>(JSValue::GetReference(env, j_value));
        v8::Local<v8::String> key = JSString::ToV8(env, runtime->isolate, j_key);
        that->Set(key, target->Get(runtime->isolate));
    V8_END()
}

JNICALL jobject JSObject::Get(JNIEnv *env, jobject jthis, jstring j_key) {
    jobject result = nullptr;

    V8_START(env, jthis, v8::Object)
        v8::Local<v8::String> key = JSString::ToV8(env, runtime->isolate, j_key);
        auto ret = that->Get(key);
        if (ret->IsUndefined()) {
            result = JSUndefined::Wrap(env, runtime);
        } else if (ret->IsNull()) {
            result = JSNull::Wrap(env, runtime);
        } else {
            result = runtime->Wrap(env, ret);
        }
    V8_END()
    return result;
}

void JSObject::New(JNIEnv *env, jobject jthis) {
    auto runtime = JSContext::GetRuntime(env, jthis);
    auto value = v8::Object::New(runtime->isolate);
    auto reference = new v8::Persistent<v8::Value>(runtime->isolate, value);
    JSValue::SetReference(env, jthis, (jlong) reference);
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

