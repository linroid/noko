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

jobject JSObject::New(JNIEnv *env, NodeRuntime *runtime, v8::Local<v8::Value> &value) {
    auto reference = new v8::Persistent<v8::Value>(runtime->isolate, value);
    return env->NewObject(objectClass.clazz, objectClass.constructor, runtime->javaContext, (jlong) reference);
}

JNICALL void JSObject::Set(JNIEnv *env, jobject thiz, jstring j_key, jobject j_value) {
    V8_ENV(env, thiz, v8::Object)
    auto target = reinterpret_cast<v8::Persistent<v8::Value> *>(JSValue::GetReference(env, j_value));
    v8::Local<v8::String> key = JSString::ToV8(env, runtime->isolate, j_key);
    that->Set(key, target->Get(runtime->isolate));
}

JNICALL jobject JSObject::Get(JNIEnv *env, jobject thiz, jstring j_key) {
    V8_ENV(env, thiz, v8::Object)
    v8::Local<v8::String> key = JSString::ToV8(env, runtime->isolate, j_key);
    auto result = that->Get(key);
    if (result->IsUndefined()) {
        return JSUndefined::New(env, runtime);
    } else if (result->IsNull()) {
        return JSNull::New(env, runtime);
    }
    // LOGI("JSObject::Get value.isUndefined()=%d", that->IsUndefined());
    // v8::Local<v8::String> type = value->TypeOf(runtime->isolate);
    // v8::String::Value unicodeString(type);
    // LOGI("type=%s", *unicodeString);
    return runtime->Wrap(env, result);
}

jint JSObject::OnLoad(JNIEnv *env) {
    jclass clazz = env->FindClass("com/linroid/knode/js/JSObject");
    if (!clazz) {
        return JNI_ERR;
    }

    JNINativeMethod methods[] = {
            {"nativeGet", "(Ljava/lang/String;)Lcom/linroid/knode/js/JSValue;", (void *) (JSObject::Get)},
            {"nativeSet", "(Ljava/lang/String;Lcom/linroid/knode/js/JSValue;)V", (void *) (JSObject::Set)},
    };
    objectClass.clazz = (jclass) env->NewGlobalRef(clazz);
    objectClass.constructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/knode/js/JSContext;J)V");
    env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
    return JNI_OK;
}
