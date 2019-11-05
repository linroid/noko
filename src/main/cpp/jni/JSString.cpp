//
// Created by linroid on 2019-10-21.
//

#include <string>
#include "JSString.h"
#include "JSValue.h"
#include "macros.h"
#include "JSContext.h"

JNIClass stringClass;

jint JSString::OnLoad(JNIEnv *env) {
    jclass clazz = env->FindClass("com/linroid/knode/js/JSString");
    if (!clazz) {
        return JNI_ERR;
    }
    stringClass.clazz = (jclass) env->NewGlobalRef(clazz);
    stringClass.constructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/knode/js/JSContext;J)V");

    JNINativeMethod methods[] = {
            {"nativeNew", "(Ljava/lang/String;)V", (void *) JSString::New},
    };

    int rc = env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
    if (rc != JNI_OK) {
        return rc;
    }

    return JNI_OK;
}

v8::Local<v8::String> JSString::ToV8(JNIEnv *env, v8::Isolate *isolate, jstring &string) {
    const uint16_t *unicodeString = env->GetStringChars(string, nullptr);
    int length = env->GetStringLength(string);
    v8::EscapableHandleScope handleScope(isolate);
    v8::Local<v8::String> result = v8::String::NewFromTwoByte(isolate, unicodeString, v8::String::NewStringType::kNormalString, length);
    env->ReleaseStringChars(string, unicodeString);
    return handleScope.Escape(result);
}

jobject JSString::Wrap(JNIEnv *env, NodeRuntime *runtime, v8::Local<v8::String> &value) {
    auto reference = new v8::Persistent<v8::Value>(runtime->isolate, value);
    return env->NewObject(stringClass.clazz, stringClass.constructor, runtime->jcontext, reference, JSString::From(env, value));
}

void JSString::New(JNIEnv *env, jobject jthis, jstring content) {
    auto runtime = JSContext::GetRuntime(env, jthis);
    auto value = JSString::ToV8(env, runtime->isolate, content);
    auto reference = new v8::Persistent<v8::Value>(runtime->isolate, value);
    JSValue::SetReference(env, jthis, (jlong) reference);
}

jstring JSString::From(JNIEnv *env, v8::Local<v8::String> &value) {
    v8::String::Value unicodeString(value);
    return env->NewString(*unicodeString, unicodeString.length());
}
