//
// Created by linroid on 2019-10-20.
//

#include "JSContext.h"
#include "JSUndefined.h"
#include "JSBoolean.h"
#include "JSNumber.h"
#include "JSObject.h"
#include "JSValue.h"
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
            {"nativeThrowError", "(Ljava/lang/String;)Lcom/linroid/knode/js/JSError;",                    (void *) ThrowError},
            {"nativeRequire",    "(Ljava/lang/String;)Lcom/linroid/knode/js/JSObject;",                   (void *) Require},
    };

    int rc = env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
    if (rc != JNI_OK) {
        return rc;
    }
    return JNI_OK;
}

jobject JSContext::Eval(JNIEnv *env, jobject jThis, jstring jCode, jstring jSource, jint jLine) {
    v8::Persistent<v8::Value> *result = nullptr;
    JSType type = kNone;
    v8::Persistent<v8::Value> *error = nullptr;

    jint codeLen = env->GetStringLength(jCode);
    auto code = env->GetStringChars(jCode, nullptr);

    jint sourceLen = env->GetStringLength(jSource);
    auto source = env->GetStringChars(jSource, nullptr);

    V8_CONTEXT(env, jThis, v8::Object)
        v8::TryCatch tryCatch(runtime->isolate_);
        v8::ScriptOrigin scriptOrigin(V8_STRING(isolate, source, sourceLen), v8::Integer::New(runtime->isolate_, jLine));
        auto script = v8::Script::Compile(context, V8_STRING(isolate, code, codeLen), &scriptOrigin);
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
        type = NodeRuntime::GetType(checkedResult);
        result = new v8::Persistent<v8::Value>(isolate, checkedResult);
    V8_END()
    if (error) {
        JSError::Throw(env, runtime, error);
        return nullptr;
    }
    env->ReleaseStringChars(jCode, code);
    env->ReleaseStringChars(jSource, source);
    return runtime->Wrap(env, result, type);
}

jobject JSContext::ParseJson(JNIEnv *env, jobject jThis, jstring jJson) {
    v8::Persistent<v8::Value> *result = nullptr;
    JSType type = kNone;
    v8::Persistent<v8::Value> *error = nullptr;
    const uint16_t *json = env->GetStringChars(jJson, nullptr);
    const jint jsonLen = env->GetStringLength(jJson);
    V8_CONTEXT(env, jThis, v8::Object)
        v8::Context::Scope contextScope(context);
        v8::TryCatch tryCatch(isolate);
        auto returned = v8::JSON::Parse(context, V8_STRING(isolate, json, jsonLen));
        if (returned.IsEmpty()) {
            error = new v8::Persistent<v8::Value>(isolate, tryCatch.Exception());
            return;
        }
        auto checkedResult = returned.ToLocalChecked();
        type = NodeRuntime::GetType(checkedResult);
        result = new v8::Persistent<v8::Value>(isolate, checkedResult);
    V8_END()
    if (error) {
        JSError::Throw(env, runtime, error);
        return nullptr;
    }
    env->ReleaseStringChars(jJson, json);
    return runtime->Wrap(env, result, type);
}


jobject JSContext::ThrowError(JNIEnv *env, jobject jThis, jstring jMessage) {
    const uint16_t *message = env->GetStringChars(jMessage, nullptr);
    const jint messageLen = env->GetStringLength(jMessage);
    v8::Persistent<v8::Value> *result = nullptr;
    V8_CONTEXT(env, jThis, v8::Object)
        auto error = v8::Exception::Error(V8_STRING(isolate, message, messageLen));
        isolate->ThrowException(error);
        result = new v8::Persistent<v8::Value>(isolate, error);
    V8_END()
    env->ReleaseStringChars(jMessage, message);
    return runtime->Wrap(env, result, JSType::kError);
}

// TODO: Not working, illegal context
jobject JSContext::Require(JNIEnv *env, jobject jThis, jstring jPath) {
    v8::Persistent<v8::Value> *result = nullptr;
    JSType type = kUndefined;

    jint pathLen = env->GetStringLength(jPath);
    auto path = env->GetStringChars(jPath, nullptr);

    V8_CONTEXT(env, jThis, v8::Object)
        auto *argv = new v8::Local<v8::Value>[1];
        argv[0] = V8_STRING(isolate, path, pathLen);

        auto global = runtime->global_->Get(isolate);

        auto require = global->Get(context, V8_UTF_STRING(isolate, "require")).ToLocalChecked()->ToObject(context).ToLocalChecked();
        assert(require->IsFunction());

        auto returned = require->CallAsFunction(context, global, 1, argv);
        if (!returned.IsEmpty()) {
            auto checkedResult = returned.ToLocalChecked();
            type = NodeRuntime::GetType(checkedResult);
            result = new v8::Persistent<v8::Value>(isolate, checkedResult);
        }
    V8_END()

    env->ReleaseStringChars(jPath, path);
    return runtime->Wrap(env, result, type);
}

void JSContext::SetShared(JNIEnv *env, NodeRuntime *runtime) {
    env->SetObjectField(runtime->jContext_, jNullId, runtime->jNull_);
    env->SetObjectField(runtime->jContext_, jUndefinedId, runtime->jUndefined_);
    env->SetObjectField(runtime->jContext_, jTrueId, runtime->jTrue_);
    env->SetObjectField(runtime->jContext_, jFalseId, runtime->jFalse_);
}
