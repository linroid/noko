//
// Created by linroid on 2019-10-19.
//

#include <jni.h>
#include "../NodeRuntime.h"
#include "JSValue.h"
#include "macros.h"
#include "JSString.h"
#include "JSContext.h"

const constexpr char *kJSValueClass = "com/linroid/knode/js/JSValue";

JNIClass valueClass;

jint JSValue::OnLoad(JNIEnv *env) {
    jclass clazz = env->FindClass(kJSValueClass);
    if (!clazz) {
        return JNI_ERR;
    }
    valueClass.clazz = (jclass) env->NewGlobalRef(clazz);
    valueClass.constructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/knode/js/JSContext;J)V");
    valueClass.reference = env->GetFieldID(clazz, "reference", "J");
    valueClass.context = env->GetFieldID(clazz, "context", "Lcom/linroid/knode/js/JSContext;");

    JNINativeMethod methods[] = {
            {"nativeToString", "()Ljava/lang/String;", (void *) JSValue::ToString},
            {"nativeToJson",   "()Ljava/lang/String;", (void *) JSValue::ToJson},
            {"nativeDispose",  "()V",                  (void *) JSValue::Dispose},
    };

    int rc = env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
    if (rc != JNI_OK) {
        return rc;
    }
    return JNI_OK;
}

jobject JSValue::GetContext(JNIEnv *env, jobject javaObj) {
    return env->GetObjectField(javaObj, valueClass.context);
}

jlong JSValue::GetReference(JNIEnv *env, jobject javaObj) {
    return env->GetLongField(javaObj, valueClass.reference);
}

void JSValue::SetReference(JNIEnv *env, jobject javaObj, jlong value) {
    env->SetLongField(javaObj, valueClass.reference, value);
}

jobject JSValue::New(JNIEnv *env, NodeRuntime *runtime, v8::Local<v8::Value> &value) {
    auto reference = new v8::Persistent<v8::Value>(runtime->isolate, value);
    return env->NewObject(valueClass.clazz, valueClass.constructor, runtime->javaContext, reference);
}

jstring JSValue::ToString(JNIEnv *env, jobject thiz) {
    V8_ENV(env, thiz, v8::Value)
    v8::MaybeLocal<v8::String> str = that->ToString(context);
    if (str.IsEmpty()) {
        return JSString::Empty(env);
    }
    v8::String::Value unicodeString(str.ToLocalChecked());
    return env->NewString(*unicodeString, unicodeString.length());
}

jstring JSValue::ToJson(JNIEnv *env, jobject thiz) {
    V8_ENV(env, thiz, v8::Value)
    auto str = v8::JSON::Stringify(context, that);
    if (str.IsEmpty()) {
        return JSString::Empty(env);
    }
    v8::String::Value unicodeString(str.ToLocalChecked());
    return env->NewString(*unicodeString, unicodeString.length());
}

v8::Local<v8::Value> JSValue::GetV8Value(JNIEnv *env, v8::Isolate *isolate, jobject javaObj) {
    v8::EscapableHandleScope handleScope(isolate);
    jlong reference = JSValue::GetReference(env, javaObj);
    auto persistent = reinterpret_cast<v8::Persistent<v8::Value> *>(reference);
    auto that = v8::Local<v8::Value>::New(isolate, *persistent);
    return handleScope.Escape(that);
}

JNIClass &JSValue::Class() {
    return valueClass;
}

void JSValue::Dispose(JNIEnv *env, jobject thiz) {
    auto runtime = JSContext::Runtime(env, thiz);
    v8::Locker locker_(runtime->isolate);
    jlong reference_ = JSValue::GetReference(env, thiz);
    auto persistent = reinterpret_cast<v8::Persistent<v8::Value> *>(reference_);
    persistent->Reset();
    JSValue::SetReference(env, thiz, 0);
}
