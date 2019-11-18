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

jmethodID JSValue::jconstructor;
jclass JSValue::jclazz;
jfieldID JSValue::jreference;
jfieldID JSValue::jcontext;
jmethodID JSValue::jruntime;

jint JSValue::OnLoad(JNIEnv *env) {
    jclass clazz = env->FindClass("com/linroid/knode/js/JSValue");
    if (!clazz) {
        return JNI_ERR;
    }
    jclazz = (jclass) env->NewGlobalRef(clazz);
    jconstructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/knode/js/JSContext;J)V");
    jreference = env->GetFieldID(clazz, "reference", "J");
    jcontext = env->GetFieldID(clazz, "context", "Lcom/linroid/knode/js/JSContext;");
    jruntime = env->GetMethodID(clazz, "runtime", "()J");

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

jstring JSValue::ToString(JNIEnv *env, jobject jthis) {
    uint16_t *unicodeChars = nullptr;
    jsize length = 0;
    auto runtime = JSValue::GetRuntime(env, jthis);
    auto isolate = runtime->isolate;
    auto value = JSValue::Unwrap(env, jthis);
    auto _runnable = [&]() {
        v8::Locker _locker(runtime->isolate);
        v8::HandleScope _handleScope(runtime->isolate);
        auto context = runtime->context.Get(isolate);
        v8::MaybeLocal<v8::String> str = value->Get(isolate)->ToString(context);
        if (str.IsEmpty()) {
            unicodeChars = new uint16_t[0];
        } else {
            v8::String::Value unicodeString(isolate, str.ToLocalChecked());
            unicodeChars = *unicodeString;
            length = unicodeString.length();
        }

    };
    runtime->Run(_runnable);
    return env->NewString(unicodeChars, length);
}

jstring JSValue::TypeOf(JNIEnv *env, jobject jthis) {
    uint16_t *unicodeChars = nullptr;
    jsize length = 0;
    V8_CONTEXT(env, jthis, v8::Value)
        auto type = that->TypeOf(isolate);
        v8::String::Value unicodeString(isolate, type);
        unicodeChars = *unicodeString;
        length = unicodeString.length();
    V8_END()
    return env->NewString(unicodeChars, length);
}

jstring JSValue::ToJson(JNIEnv *env, jobject jthis) {
    uint16_t *unicodeChars = nullptr;
    jsize length = 0;
    V8_CONTEXT(env, jthis, v8::Value)
        auto str = v8::JSON::Stringify(context, that);
        if (str.IsEmpty()) {
            unicodeChars = new uint16_t[0];
        } else {
            auto value = str.ToLocalChecked();
            v8::String::Value unicodeString(isolate, value);
            unicodeChars = *unicodeString;
            length = unicodeString.length();
        }
    V8_END()
    return env->NewString(unicodeChars, length);
}

jdouble JSValue::ToNumber(JNIEnv *env, jobject jthis) {
    jdouble result = 0;
    v8::Persistent<v8::Value> *error;
    V8_CONTEXT(env, jthis, v8::Value)
        v8::TryCatch tryCatch(runtime->isolate);
        auto number = that->ToNumber(context);
        if (number.IsEmpty()) {
            error = new v8::Persistent<v8::Value>(runtime->isolate, tryCatch.Exception());
            result = 0.0;
            return;
        }
        v8::Local<v8::Number> checked = number.ToLocalChecked();
        result = checked->Value();
    V8_END()
    if (error) {
        JSError::Throw(env, runtime, error);
        return 0.0;
    }
    return result;
}

void JSValue::Dispose(JNIEnv *env, jobject jthis) {
    auto value = JSValue::Unwrap(env, jthis);
    value->Reset();
    delete value;
    JSValue::SetReference(env, jthis, 0);
}
