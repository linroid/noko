//
// Created by linroid on 2019/10/31.
//

#include "JSFunction.h"
#include "macros.h"
#include "JSValue.h"
#include "JSContext.h"
#include "JSString.h"
#include "JSError.h"
#include "JVMCallback.h"

JNIClass functionClass;

jmethodID functionOnCallMethod;

void staticCallback(const v8::FunctionCallbackInfo<v8::Value> &info) {
    CHECK(info.Data()->IsExternal());
    auto external = info.Data().As<v8::External>();
    JVMCallback *callback = reinterpret_cast<JVMCallback * >(external->Value());
    callback->Call(info);
}

jobject JSFunction::Wrap(JNIEnv *env, NodeRuntime *runtime, v8::Persistent<v8::Value> *value) {
    return env->NewObject(functionClass.clazz, functionClass.constructor, runtime->jcontext, (jlong) value);
}

void JSFunction::New(JNIEnv *env, jobject jthis, jstring jname) {
    v8::Persistent<v8::Value> *result = nullptr;
    const uint16_t *name = env->GetStringChars(jname, nullptr);
    const jint nameLen = env->GetStringLength(jname);
    auto callback = new JVMCallback(env, jthis, JSValue::JVMClass(), functionOnCallMethod);
    V8_SCOPE(env, jthis)
        callback->runtime = runtime;
        auto data = v8::External::New(runtime->isolate, callback);
        auto func = v8::FunctionTemplate::New(runtime->isolate, staticCallback, data)->GetFunction();
        func->SetName(V8_STRING(name, nameLen));
        result = new v8::Persistent<v8::Value>(runtime->isolate, func);
    V8_END()
    env->ReleaseStringChars(jname, name);
    JSValue::SetReference(env, jthis, (jlong) result);
}

jobject JSFunction::Call(JNIEnv *env, jobject jthis, jobject jreceiver, jobjectArray jparameters) {
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

    V8_CONTEXT(env, jthis, v8::Function)
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
    functionClass.clazz = (jclass) env->NewGlobalRef(clazz);
    functionClass.constructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/knode/js/JSContext;J)V");
    functionOnCallMethod = env->GetMethodID(clazz, "onCall",
                                            "(Lcom/linroid/knode/js/JSValue;[Lcom/linroid/knode/js/JSValue;)Lcom/linroid/knode/js/JSValue;");
    env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
    return JNI_OK;
}
