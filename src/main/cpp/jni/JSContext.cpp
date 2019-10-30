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

JNIClass contextClass;

jint JSContext::OnLoad(JNIEnv *env) {
    jclass clazz = env->FindClass(kContextClass);
    if (clazz == nullptr) {
        return JNI_ERR;
    }
    contextClass.clazz = (jclass) (env->NewGlobalRef(clazz));
    contextClass.constructor = env->GetMethodID(clazz, "<init>", "(JJ)V");
    contextClass.runtimePtr = env->GetFieldID(clazz, "runtimePtr", "J");

    JNINativeMethod methods[] = {
            {"nativeEval", "(Ljava/lang/String;Ljava/lang/String;I)Ljava/lang/Object;", (void *) Eval},
    };

    int rc = env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
    if (rc != JNI_OK) {
        return rc;
    }
    return JNI_OK;
}

jobject JSContext::New(JNIEnv *env, NodeRuntime *runtime) {
    return env->NewObject(contextClass.clazz,
                          contextClass.constructor,
                          reinterpret_cast<jlong>(runtime),
                          reinterpret_cast<jlong>(runtime->global));
}

NodeRuntime *JSContext::Runtime(JNIEnv *env, jobject javaObject) {
    jobject javaContext = JSValue::GetContext(env, javaObject);
    if (javaContext == nullptr) {
        javaContext = javaObject;
    }
    jlong runtimePtr = env->GetLongField(javaContext, contextClass.runtimePtr);
    return reinterpret_cast<NodeRuntime *>(runtimePtr);
}


jobject JSContext::Get(JNIEnv *env, jobject thiz, jstring key) {
    return nullptr;
}

jobject JSContext::Get(JNIEnv *env, jobject thiz, jstring key, jobject value) {
    return nullptr;
}

jlong JSContext::Bind(JNIEnv *env, jobject thiz, jlong contextPtr) {
    return 0;
}

jobject JSContext::Eval(JNIEnv *env, jstring thiz, jstring jcode, jstring jsource, jint jline) {
    V8_ENV(env, thiz, v8::Object)
    v8::TryCatch tryCatch(runtime->isolate);
    v8::ScriptOrigin scriptOrigin(JSString::ToV8(env, runtime->isolate, jsource),
                                  v8::Integer::New(runtime->isolate, jline));
    auto code = JSString::ToV8(env, runtime->isolate, jcode);
    auto script = v8::Script::Compile(context, code, &scriptOrigin);
    if (script.IsEmpty()) {
        auto message = tryCatch.Message()->Get();
        v8::String::Value unicodeString(message);
        LOGE("Eval with exception: %s", message);
        // TODO: throw exception
        return env->NewString(*unicodeString, unicodeString.length());;
    }
    auto result = script.ToLocalChecked()->Run(context);
    if (result.IsEmpty()) {
        auto message = tryCatch.Message()->Get();
        v8::String::Value unicodeString(message);
        LOGE("Eval with exception: %s", message);
        // TODO: throw exception
        return env->NewString(*unicodeString, unicodeString.length());;
    }
    auto checkedResult = result.ToLocalChecked();
    return runtime->Wrap(env, checkedResult);
}
