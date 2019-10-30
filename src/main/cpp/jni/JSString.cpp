//
// Created by linroid on 2019-10-21.
//

#include "JSString.h"
#include <string>

JNIClass stringClass;

jint JSString::OnLoad(JNIEnv *env) {
    jclass clazz = env->FindClass("com/linroid/knode/js/JSString");
    if (!clazz) {
        return JNI_ERR;
    }
    stringClass.clazz = (jclass) env->NewGlobalRef(clazz);
    stringClass.constructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/knode/js/JSContext;J)V");
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
