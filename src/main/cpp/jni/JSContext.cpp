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

JNIClass contextClass;

jint JSContext::OnLoad(JNIEnv *env) {
    jclass clazz = env->FindClass("com/linroid/knode/js/JSContext");
    if (clazz == nullptr) {
        return JNI_ERR;
    }
    contextClass.clazz = (jclass) (env->NewGlobalRef(clazz));
    contextClass.constructor = env->GetMethodID(clazz, "<init>", "(JJ)V");
    contextClass.runtimePtr = env->GetFieldID(clazz, "runtimePtr", "J");

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

jobject JSContext::Wrap(JNIEnv *env, NodeRuntime *runtime) {
    return env->NewObject(contextClass.clazz,
                          contextClass.constructor,
                          reinterpret_cast<jlong>(runtime),
                          reinterpret_cast<jlong>(runtime->global));
}

NodeRuntime *JSContext::GetRuntime(JNIEnv *env, jobject jobj) {
    jobject jcontext = JSValue::GetContext(env, jobj);
    if (jcontext == nullptr) {
        jcontext = jobj;
    }
    jlong runtimePtr = env->GetLongField(jcontext, contextClass.runtimePtr);
    return reinterpret_cast<NodeRuntime *>(runtimePtr);
}

jobject JSContext::Get(JNIEnv *env, jobject jthis, jstring key) {
    return nullptr;
}

jobject JSContext::Get(JNIEnv *env, jobject jthis, jstring key, jobject value) {
    return nullptr;
}

jlong JSContext::Bind(JNIEnv *env, jobject jthis, jlong contextPtr) {
    return 0;
}

jobject JSContext::Eval(JNIEnv *env, jstring jthis, jstring jcode, jstring jsource, jint jline) {
    V8_ENV(env, jthis, v8::Object)
    v8::TryCatch tryCatch(runtime->isolate);
    v8::ScriptOrigin scriptOrigin(JSString::ToV8(env, runtime->isolate, jsource),
                                  v8::Integer::New(runtime->isolate, jline));
    auto code = JSString::ToV8(env, runtime->isolate, jcode);
    auto script = v8::Script::Compile(context, code, &scriptOrigin);
    if (script.IsEmpty()) {
        LOGE("Compile script with an exception");
        JSError::Throw(env, runtime, tryCatch);
        return 0;
    }
    auto result = script.ToLocalChecked()->Run(context);
    if (result.IsEmpty()) {
        LOGE("Run script with an exception");
        JSError::Throw(env, runtime, tryCatch);
        return 0;
    }
    auto checkedResult = result.ToLocalChecked();
    return runtime->Wrap(env, checkedResult);
}

jobject JSContext::ParseJson(JNIEnv *env, jstring jthis, jstring jjson) {
    V8_ENV(env, jthis, v8::Object)
    auto json = JSString::ToV8(env, runtime->isolate, jjson);
    v8::TryCatch tryCatch(runtime->isolate);
    auto result = v8::JSON::Parse(runtime->isolate, json);
    if (result.IsEmpty()) {
        JSError::Throw(env, runtime, tryCatch);
        return 0;
    }
    auto checkedResult = result.ToLocalChecked();
    return runtime->Wrap(env, checkedResult);
}
