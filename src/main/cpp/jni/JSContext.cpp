//
// Created by linroid on 2019-10-20.
//

#include "JSContext.h"
#include "JSUndefined.h"
#include "JSBoolean.h"
#include "JSNumber.h"
#include "JSObject.h"
#include "JSValue.h"
#include "macros.h"
#include "JSString.h"
#include "../NodeRuntime.h"
#include "JSError.h"

jclass JSContext::jclazz;
jmethodID JSContext::jconstructor;
jfieldID JSContext::jruntimePtr;

jint JSContext::OnLoad(JNIEnv *env) {
    jclass clazz = env->FindClass("com/linroid/knode/js/JSContext");
    if (clazz == nullptr) {
        return JNI_ERR;
    }
    jclazz = (jclass) (env->NewGlobalRef(clazz));
    jconstructor = env->GetMethodID(clazz, "<init>", "(JJ)V");
    jruntimePtr = env->GetFieldID(clazz, "runtimePtr", "J");

    JNINativeMethod methods[] = {
            {"nativeEval",      "(Ljava/lang/String;Ljava/lang/String;I)Lcom/linroid/knode/js/JSValue;", (void *) Eval},
            {"nativeParseJson", "(Ljava/lang/String;)Lcom/linroid/knode/js/JSValue;",                    (void *) ParseJson},
    };

    int rc = env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
    if (rc != JNI_OK) {
        return rc;
    }
    return JNI_OK;
}

jobject JSContext::Eval(JNIEnv *env, jstring jthis, jstring jcode, jstring jsource, jint jline) {
    v8::Persistent<v8::Value> *result = nullptr;
    JSType type = None;
    v8::Persistent<v8::Value> *error = nullptr;

    jint codeLen = env->GetStringLength(jcode);
    auto code = env->GetStringChars(jcode, nullptr);

    jint sourceLen = env->GetStringLength(jsource);
    auto source = env->GetStringChars(jsource, nullptr);

    V8_CONTEXT(env, jthis, v8::Object)
        v8::TryCatch tryCatch(runtime->isolate);
        v8::ScriptOrigin scriptOrigin(V8_STRING(source, sourceLen), v8::Integer::New(runtime->isolate, jline));
        auto script = v8::Script::Compile(context, V8_STRING(code, codeLen), &scriptOrigin);
        if (script.IsEmpty()) {
            LOGE("Compile script with an exception");
            error = new v8::Persistent<v8::Value>(isolate, tryCatch.Exception());
            return;
        }
        auto returned = script.ToLocalChecked()->Run(context);
        if (returned.IsEmpty()) {
            LOGE("Run script with an exception");
            error = new v8::Persistent<v8::Value>(isolate, tryCatch.Exception());
            return;
        }
        LOGI("Eval success");
        auto checkedResult = returned.ToLocalChecked();
        type = runtime->GetType(checkedResult);
        result = new v8::Persistent<v8::Value>(isolate, checkedResult);
    V8_END()
    LOGE("EVAL result: %p", result);
    if (error) {
        JSError::Throw(env, runtime, error);
        return nullptr;
    }
    env->ReleaseStringChars(jcode, code);
    env->ReleaseStringChars(jsource, source);
    return runtime->Wrap(env, result, type);
}

jobject JSContext::ParseJson(JNIEnv *env, jstring jthis, jstring jjson) {
    v8::Persistent<v8::Value> *result = nullptr;
    JSType type = None;
    v8::Persistent<v8::Value> *error = nullptr;
    const uint16_t *json = env->GetStringChars(jjson, nullptr);
    const jint jsonLen = env->GetStringLength(jjson);
    V8_CONTEXT(env, jthis, v8::Object)
        v8::TryCatch tryCatch(isolate);
        auto returned = v8::JSON::Parse(isolate, V8_STRING(json, jsonLen));
        if (returned.IsEmpty()) {
            error = new v8::Persistent<v8::Value>(isolate, tryCatch.Exception());
            return;
        }
        auto checkedResult = returned.ToLocalChecked();
        type = runtime->GetType(checkedResult);
        result = new v8::Persistent<v8::Value>(isolate, checkedResult);
    V8_END()
    if (error) {
        JSError::Throw(env, runtime, error);
        return nullptr;
    }
    env->ReleaseStringChars(jjson, json);
    return runtime->Wrap(env, result, type);
}
