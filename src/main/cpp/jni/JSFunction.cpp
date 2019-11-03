//
// Created by linroid on 2019/10/31.
//

#include "JSFunction.h"
#include "macros.h"
#include "JSValue.h"
#include "JSContext.h"
#include "JSString.h"
#include "JavaCallback.h"

JNIClass functionClass;

jmethodID functionOnCallMethod;

void staticCallback(const v8::FunctionCallbackInfo<v8::Value> &info) {
    CHECK(info.Data()->IsExternal());
    auto external = info.Data().As<v8::External>();
    JavaCallback *callback = reinterpret_cast<JavaCallback * >(external->Value());
    callback->Call(info);
}

jobject JSFunction::Wrap(JNIEnv *env, NodeRuntime *runtime, v8::Local<v8::Function> &value) {
    auto reference = new v8::Persistent<v8::Value>(runtime->isolate, value);
    auto func = value.As<v8::Function>();
    auto name = func->GetName()->ToString();
    return env->NewObject(functionClass.clazz, functionClass.constructor, runtime->jcontext, reference, JSString::ToJVM(env, name));
}

void JSFunction::New(JNIEnv *env, jobject jthis, jstring jname) {
    auto runtime = JSContext::GetRuntime(env, jthis);
    auto name = JSString::From(env, runtime->isolate, jname);
    auto data = v8::External::New(runtime->isolate, new JavaCallback(runtime, env, jthis, functionClass.clazz, functionOnCallMethod));
    auto func = v8::FunctionTemplate::New(runtime->isolate, staticCallback, data)->GetFunction();
    func->SetName(name);
    auto reference = new v8::Persistent<v8::Value>(runtime->isolate, func);
    JSValue::SetReference(env, jthis, (jlong) reference);
}

jobject JSFunction::Call(JNIEnv *env, jobject jthis, jobject j_recv, jobjectArray j_parameters) {
    V8_ENV(env, jthis, v8::Function)
    int argc = env->GetArrayLength(j_parameters);
    v8::Local<v8::Value> *argv = new v8::Local<v8::Value>[argc];
    for (int i = 0; i < argc; ++i) {
        auto j_element = env->GetObjectArrayElement(j_parameters, i);
        argv[i] = JSValue::GetReference(env, runtime->isolate, j_element);
    }
    auto recv = JSValue::GetReference(env, runtime->isolate, j_recv);
    v8::TryCatch tryCatch(runtime->isolate);
    auto ret = that->Call(context, recv, argc, argv);
    if (tryCatch.HasCaught()) {
        runtime->ThrowJSError(env, tryCatch);
        return 0;
    }
    if (ret.IsEmpty()) {
        return 0;
    };
    auto retValue = ret.ToLocalChecked();
    return runtime->Wrap(env, retValue);
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
