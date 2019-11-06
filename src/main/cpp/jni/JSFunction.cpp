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

jobject JSFunction::Wrap(JNIEnv *env, NodeRuntime *runtime, v8::Local<v8::Function> &value) {
    auto reference = new v8::Persistent<v8::Value>(runtime->isolate, value);
    return env->NewObject(functionClass.clazz, functionClass.constructor, runtime->jcontext, (jlong) reference);
}

void JSFunction::New(JNIEnv *env, jobject jthis, jstring jname) {
    V8_SCOPE(env, jthis)
        auto name = JSString::ToV8(env, runtime->isolate, jname);
        auto data = v8::External::New(runtime->isolate, new JVMCallback(runtime, env, jthis, JSValue::JVMClass(), functionOnCallMethod));
        auto func = v8::FunctionTemplate::New(runtime->isolate, staticCallback, data)->GetFunction();
        func->SetName(name);
        auto reference = new v8::Persistent<v8::Value>(runtime->isolate, func);
        JSValue::SetReference(env, jthis, (jlong) reference);
    V8_END()
}

jobject JSFunction::Call(JNIEnv *env, jobject jthis, jobject jreceiver, jobjectArray jparameters) {
    jobject result = nullptr;
    V8_CONTEXT(env, jthis, v8::Function)
        int argc = env->GetArrayLength(jparameters);
        v8::Local<v8::Value> *argv = new v8::Local<v8::Value>[argc];
        for (int i = 0; i < argc; ++i) {
            auto j_element = env->GetObjectArrayElement(jparameters, i);
            argv[i] = JSValue::GetReference(env, runtime->isolate, j_element);
        }
        auto recv = JSValue::GetReference(env, runtime->isolate, jreceiver);
        v8::TryCatch tryCatch(runtime->isolate);
        auto ret = that->Call(context, recv, argc, argv);
        if (tryCatch.HasCaught()) {
            JSError::Throw(env, runtime, tryCatch);
            result = 0;
            return;
        }
        if (ret.IsEmpty()) {
            result = 0;
            return;
        }
        auto retValue = ret.ToLocalChecked();
        result = runtime->Wrap(env, retValue);
    V8_END()
    return result;
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
