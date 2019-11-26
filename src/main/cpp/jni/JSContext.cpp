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

jclass JSContext::jClazz;
jmethodID JSContext::jConstructor;
jfieldID JSContext::jNullId;
jfieldID JSContext::jUndefinedId;
jfieldID JSContext::jTrueId;
jfieldID JSContext::jFalseId;

jint JSContext::OnLoad(JNIEnv *env) {
    jclass clazz = env->FindClass("com/linroid/knode/js/JSContext");
    if (clazz == nullptr) {
        return JNI_ERR;
    }
    jClazz = (jclass) (env->NewGlobalRef(clazz));
    jConstructor = env->GetMethodID(clazz, "<init>", "(JJ)V");
    jNullId = env->GetFieldID(clazz, "sharedNull", "Lcom/linroid/knode/js/JSNull;");
    jUndefinedId = env->GetFieldID(clazz, "sharedUndefined", "Lcom/linroid/knode/js/JSUndefined;");
    jTrueId = env->GetFieldID(clazz, "sharedTrue", "Lcom/linroid/knode/js/JSBoolean;");
    jFalseId = env->GetFieldID(clazz, "sharedFalse", "Lcom/linroid/knode/js/JSBoolean;");

    JNINativeMethod methods[] = {
            {"nativeEval",       "(Ljava/lang/String;Ljava/lang/String;I)Lcom/linroid/knode/js/JSValue;", (void *) Eval},
            {"nativeParseJson",  "(Ljava/lang/String;)Lcom/linroid/knode/js/JSValue;",                    (void *) ParseJson},
            {"nativeThrowError", "(Ljava/lang/String;)V",                                                 (void *) ThrowError},
    };

    int rc = env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
    if (rc != JNI_OK) {
        return rc;
    }
    return JNI_OK;
}

jobject JSContext::Eval(JNIEnv *env, jstring jThis, jstring jcode, jstring jsource, jint jline) {
    v8::Persistent<v8::Value> *result = nullptr;
    JSType type = None;
    v8::Persistent<v8::Value> *error = nullptr;

    jint codeLen = env->GetStringLength(jcode);
    auto code = env->GetStringChars(jcode, nullptr);

    jint sourceLen = env->GetStringLength(jsource);
    auto source = env->GetStringChars(jsource, nullptr);

    V8_CONTEXT(env, jThis, v8::Object)
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
        auto checkedResult = returned.ToLocalChecked();
        type = runtime->GetType(checkedResult);
        result = new v8::Persistent<v8::Value>(isolate, checkedResult);
    V8_END()
    if (error) {
        JSError::Throw(env, runtime, error);
        return nullptr;
    }
    env->ReleaseStringChars(jcode, code);
    env->ReleaseStringChars(jsource, source);
    return runtime->Wrap(env, result, type);
}

jobject JSContext::ParseJson(JNIEnv *env, jstring jThis, jstring jjson) {
    v8::Persistent<v8::Value> *result = nullptr;
    JSType type = None;
    v8::Persistent<v8::Value> *error = nullptr;
    const uint16_t *json = env->GetStringChars(jjson, nullptr);
    const jint jsonLen = env->GetStringLength(jjson);
    V8_CONTEXT(env, jThis, v8::Object)
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


void JSContext::ThrowError(JNIEnv *env, jstring jThis, jstring jmessage) {
    const uint16_t *message = env->GetStringChars(jmessage, nullptr);
    const jint messageLen = env->GetStringLength(jmessage);
    V8_CONTEXT(env, jThis, v8::Object)
        auto error = v8::Exception::Error(V8_STRING(message, messageLen));
        isolate->ThrowException(error);
    V8_END()
    env->ReleaseStringChars(jmessage, message);
}

void JSContext::SetShared(JNIEnv *env, NodeRuntime *runtime) {
    env->SetObjectField(runtime->jContext, jNullId, runtime->jNull);
    env->SetObjectField(runtime->jContext, jUndefinedId, runtime->jUndefined);
    env->SetObjectField(runtime->jContext, jTrueId, runtime->jTrue);
    env->SetObjectField(runtime->jContext, jFalseId, runtime->jFalse);
}
