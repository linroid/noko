//
// Created by linroid on 2019/10/31.
//

#include "JSFunction.h"
#include "macros.h"
#include "JSValue.h"
#include "JSContext.h"
#include "JSString.h"
#include "JSError.h"
#include "JniCallback.h"

jclass JSFunction::jClazz;
jmethodID JSFunction::jConstructor;
jmethodID JSFunction::jonCall;

void staticCallback(const v8::FunctionCallbackInfo<v8::Value> &info) {
    CHECK(info.Data()->IsExternal());
    auto external = info.Data().As<v8::External>();
    JniCallback *callback = reinterpret_cast<JniCallback * >(external->Value());
    callback->Call(info);
}

void JSFunction::New(JNIEnv *env, jobject jThis, jstring jname) {
    v8::Persistent<v8::Value> *result = nullptr;
    const uint16_t *name = env->GetStringChars(jname, nullptr);
    const jint nameLen = env->GetStringLength(jname);
    auto callback = new JniCallback(env, jThis, JSValue::jClazz, jonCall);
    V8_SCOPE(env, jThis)
        callback->runtime = runtime;
        auto data = v8::External::New(runtime->isolate, callback);
        auto func = v8::FunctionTemplate::New(runtime->isolate, staticCallback, data)->GetFunction();
        func->SetName(V8_STRING(name, nameLen));
        result = new v8::Persistent<v8::Value>(runtime->isolate, func);
    V8_END()
    env->ReleaseStringChars(jname, name);
    JSValue::SetReference(env, jThis, (jlong) result);
}

jobject JSFunction::Call(JNIEnv *env, jobject jThis, jobject jreceiver, jobjectArray jparameters) {
    v8::Persistent<v8::Value> *result = nullptr;
    v8::Persistent<v8::Value> *error = nullptr;
    JSType type = None;

    auto receiver = JSValue::Unwrap(env, jreceiver);

    int argc = env->GetArrayLength(jparameters);
    v8::Persistent<v8::Value> *parameters[argc];
    for (int i = 0; i < argc; ++i) {
        auto jelement = env->GetObjectArrayElement(jparameters, i);
        parameters[i] = JSValue::Unwrap(env, jelement);
    }

    V8_CONTEXT(env, jThis, v8::Function)
        v8::Local<v8::Value> *argv = new v8::Local<v8::Value>[argc];
        for (int i = 0; i < argc; ++i) {
            argv[i] = parameters[i]->Get(isolate);
        }
        v8::TryCatch tryCatch(runtime->isolate);
        auto ret = that->Call(context, receiver->Get(isolate), argc, argv);
        if (ret.IsEmpty()) {
            error = new v8::Persistent<v8::Value>(isolate, tryCatch.Exception());
            return;
        }
        auto checked = ret.ToLocalChecked();
        type = runtime->GetType(checked);
        result = new v8::Persistent<v8::Value>(isolate, checked);
    V8_END()
    if (error) {
        JSError::Throw(env, runtime, error);
        return nullptr;
    }
    return runtime->Wrap(env, result, type);
}

jint JSFunction::OnLoad(JNIEnv *env) {
    jclass clazz = env->FindClass("com/linroid/knode/js/JSFunction");
    if (!clazz) {
        return JNI_ERR;
    }

    JNINativeMethod methods[] = {
            {"nativeCall", "(Lcom/linroid/knode/js/JSValue;[Lcom/linroid/knode/js/JSValue;)Lcom/linroid/knode/js/JSValue;", (void *) JSFunction::Call},
            {"nativeNew",  "(Ljava/lang/String;)V",                                                                         (void *) JSFunction::New},
    };
    jClazz = (jclass) env->NewGlobalRef(clazz);
    jConstructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/knode/js/JSContext;J)V");
    jonCall = env->GetMethodID(clazz, "onCall",
                               "(Lcom/linroid/knode/js/JSValue;[Lcom/linroid/knode/js/JSValue;)Lcom/linroid/knode/js/JSValue;");
    env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
    return JNI_OK;
}
