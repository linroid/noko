//
// Created by linroid on 2019-10-19.
//

#include <jni.h>
#include "../NodeRuntime.h"
#include "JSValue.h"
#include "macros.h"
#include "JSString.h"
#include "JSContext.h"
#include "JSError.h"

JNIClass valueClass;

jint JSValue::OnLoad(JNIEnv *env) {
    jclass clazz = env->FindClass("com/linroid/knode/js/JSValue");
    if (!clazz) {
        return JNI_ERR;
    }
    valueClass.clazz = (jclass) env->NewGlobalRef(clazz);
    valueClass.constructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/knode/js/JSContext;J)V");
    valueClass.reference = env->GetFieldID(clazz, "reference", "J");
    valueClass.context = env->GetFieldID(clazz, "context", "Lcom/linroid/knode/js/JSContext;");

    JNINativeMethod methods[] = {
            {"nativeToString", "()Ljava/lang/String;", (void *) JSValue::ToString},
            {"nativeTypeOf",   "()Ljava/lang/String;", (void *) JSValue::TypeOf},
            {"nativeToJson",   "()Ljava/lang/String;", (void *) JSValue::ToJson},
            {"nativeDispose",  "()V",                  (void *) JSValue::Dispose},
            {"nativeToNumber", "()D",                  (void *) JSValue::ToNumber},
    };

    int rc = env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
    if (rc != JNI_OK) {
        return rc;
    }
    return JNI_OK;
}

jobject JSValue::GetContext(JNIEnv *env, jobject jobj) {
    return env->GetObjectField(jobj, valueClass.context);
}

jlong JSValue::GetReference(JNIEnv *env, jobject jobj) {
    return env->GetLongField(jobj, valueClass.reference);
}

void JSValue::SetReference(JNIEnv *env, jobject jobj, jlong value) {
    env->SetLongField(jobj, valueClass.reference, value);
}

jobject JSValue::Wrap(JNIEnv *env, NodeRuntime *runtime, v8::Local<v8::Value> &value) {
    auto reference = new v8::Persistent<v8::Value>(runtime->isolate, value);
    return env->NewObject(valueClass.clazz, valueClass.constructor, runtime->jcontext, reference);
}

jstring JSValue::ToString(JNIEnv *env, jobject jthis) {
    jstring result = nullptr;
    V8_CONTEXT(env, jthis, v8::Value)
        v8::MaybeLocal<v8::String> str = that->ToString(context);
        if (str.IsEmpty()) {
            const char *bytes = new char[0];
            result = env->NewStringUTF(bytes);
        } else {
            v8::String::Value unicodeString(str.ToLocalChecked());
            result = env->NewString(*unicodeString, unicodeString.length());
        }
    V8_END()
    return result;
}

jstring JSValue::TypeOf(JNIEnv *env, jobject jthis) {
    jstring result = nullptr;
    V8_CONTEXT(env, jthis, v8::Value)
        auto type = that->TypeOf(runtime->isolate);
        result = JSString::From(env, type);
    V8_END()
    return result;
}

jstring JSValue::ToJson(JNIEnv *env, jobject jthis) {
    jstring result = nullptr;
    V8_CONTEXT(env, jthis, v8::Value)
        auto str = v8::JSON::Stringify(context, that);
        if (str.IsEmpty()) {
            const char *bytes = new char[0];
            result = env->NewStringUTF(bytes);
        } else {
            v8::String::Value unicodeString(str.ToLocalChecked());
            result = env->NewString(*unicodeString, unicodeString.length());
        }
    V8_END()
    return result;
}

jdouble JSValue::ToNumber(JNIEnv *env, jobject jthis) {
    jdouble result = 0;
    V8_CONTEXT(env, jthis, v8::Value)
        v8::TryCatch tryCatch(runtime->isolate);

        auto number = that->ToNumber(context);
        if (number.IsEmpty()) {
            JSError::Throw(env, runtime, tryCatch);
            result = 0.0;
            return;
        }
        result = number.ToLocalChecked()->Value();
    V8_END()
    return result;
}

v8::Local<v8::Value> JSValue::GetReference(JNIEnv *env, v8::Isolate *isolate, jobject jobj) {
    v8::EscapableHandleScope handleScope(isolate);
    jlong reference = JSValue::GetReference(env, jobj);
    auto persistent = reinterpret_cast<v8::Persistent<v8::Value> *>(reference);
    auto that = v8::Local<v8::Value>::New(isolate, *persistent);
    return handleScope.Escape(that);
}

void JSValue::Dispose(JNIEnv *env, jobject jthis) {
    auto runtime = JSContext::GetRuntime(env, jthis);
    v8::Locker locker_(runtime->isolate);
    jlong reference_ = JSValue::GetReference(env, jthis);
    auto persistent = reinterpret_cast<v8::Persistent<v8::Value> *>(reference_);
    persistent->Reset();
    JSValue::SetReference(env, jthis, 0);
}

jclass &JSValue::JVMClass() {
    return valueClass.clazz;
}
