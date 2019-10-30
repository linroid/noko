//
// Created by linroid on 2019-10-21.
//

#include "JSString.h"
#include "JSValue.h"
#include "macros.h"
#include <string>

JNIClass stringClass;

jint JSString::OnLoad(JNIEnv *env) {
    jclass clazz = env->FindClass("com/linroid/knode/js/JSString");
    if (!clazz) {
        return JNI_ERR;
    }
    stringClass.clazz = (jclass) env->NewGlobalRef(clazz);
    stringClass.constructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/knode/js/JSContext;J)V");

    JNINativeMethod methods[] = {
            {"nativeInit", "(Ljava/lang/String;)V", (void *) JSString::NativeInit},
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

jstring JSString::Empty(JNIEnv *env) {
    std::string str;
    return env->NewStringUTF(str.c_str());
}

jobject JSString::New(JNIEnv *env, NodeRuntime *runtime, v8::Local<v8::Value> &value) {
    auto reference = new v8::Persistent<v8::Value>(runtime->isolate, value);
    return env->NewObject(stringClass.clazz, stringClass.constructor, runtime->javaContext, reference, value->BooleanValue());
}

void JSString::NativeInit(JNIEnv *env, jobject thiz, jstring content) {
    auto runtime = JSContext::Runtime(env, thiz);
    auto value = JSString::ToV8(env, runtime->isolate, content);
    auto reference = new v8::Persistent<v8::Value>(runtime->isolate, value);
    JSValue::SetReference(env, thiz, (jlong) reference);
}
