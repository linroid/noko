//
// Created by linroid on 2019/10/31.
//

#include "JSFunction.h"
#include "macros.h"
#include "JSValue.h"
#include "JSContext.h"
#include "JSString.h"

JNIClass functionClass;

jmethodID functionOnCallMethod;

jobject JSFunction::New(JNIEnv *env, NodeRuntime *runtime, v8::Local<v8::Value> &value) {
    auto reference = new v8::Persistent<v8::Value>(runtime->isolate, value);
    auto func = value.As<v8::Function>();
    auto name = func->GetConstructorName();
    return env->NewObject(functionClass.clazz, functionClass.constructor, runtime->javaContext, reference, JSString::Value(env, name));
}

jint JSFunction::OnLoad(JNIEnv *env) {
    jclass clazz = env->FindClass("com/linroid/knode/js/JSFunction");
    if (!clazz) {
        return JNI_ERR;
    }

    JNINativeMethod methods[] = {
            {"nativeCall", "(Lcom/linroid/knode/js/JSValue;[Lcom/linroid/knode/js/JSValue;)Lcom/linroid/knode/js/JSValue;", (void *) (Call)},
            {"nativeInit", "()V",                                                                                           (void *) JSFunction::Init},
    };
    functionClass.clazz = (jclass) env->NewGlobalRef(clazz);
    functionClass.constructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/knode/js/JSContext;Ljava/lang/String;J)V");
    functionOnCallMethod = env->GetMethodID(clazz, "onCall",
                                            "(Lcom/linroid/knode/js/JSValue;[Lcom/linroid/knode/js/JSValue;)Lcom/linroid/knode/js/JSValue;");
    env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
    return JNI_OK;
}

jobject JSFunction::Call(JNIEnv *env, jobject thiz, jobject j_recv, jobjectArray j_parameters) {
    V8_ENV(env, thiz, v8::Function)
    int argc = env->GetArrayLength(j_parameters);
    v8::Local<v8::Value> *argv = new v8::Local<v8::Value>[argc];
    for (int i = 0; i < argc; ++i) {
        auto j_element = env->GetObjectArrayElement(j_parameters, i);
        argv[i] = JSValue::GetV8Value(env, runtime->isolate, j_element);
    }
    auto recv = JSValue::GetV8Value(env, runtime->isolate, j_recv);
    v8::TryCatch tryCatch(runtime->isolate);
    auto ret = that->Call(context, recv, argc, argv);
    if (tryCatch.HasCaught()) {
        runtime->throwJSError(env, tryCatch);
        return 0;
    }
    if (ret.IsEmpty()) {
        return 0;
    };
    auto retValue = ret.ToLocalChecked();
    return runtime->Wrap(env, retValue);
}

class JniCallback {
private:
    NodeRuntime *runtime;
    JavaVM *vm;
    jobject that;
    jmethodID methodId;
public:
    JniCallback(NodeRuntime *runtime, JNIEnv *env, jobject that, jmethodID methodId)
            : runtime(runtime), that(env->NewGlobalRef(that)), methodId(methodId) {
        env->GetJavaVM(&vm);
    }

    void call(const v8::FunctionCallbackInfo<v8::Value> &info) {
        JNIEnv *env;
        auto stat = vm->GetEnv((void **) (&env), JNI_VERSION_1_6);
        if (stat == JNI_EDETACHED) {
            vm->AttachCurrentThread(&env, nullptr);
        }

        auto parameters = env->NewObjectArray(info.kArgsLength, JSValue::Class().clazz, nullptr);
        for (int i = 0; i < info.kArgsLength; ++i) {
            v8::Local<v8::Value> element = info[i];
            env->SetObjectArrayElement(parameters, i, runtime->Wrap(env, element));
        }
        auto caller = (v8::Local<v8::Value>) info.This();
        auto j_ret = env->CallObjectMethod(that, methodId, runtime->Wrap(env, caller), parameters);
        if (j_ret != 0) {
            auto ret = JSValue::GetV8Value(env, runtime->isolate, j_ret);
            info.GetReturnValue().Set(ret);
        }
        if (stat == JNI_EDETACHED) {
            vm->DetachCurrentThread();
        }
    }

    ~JniCallback() {
        JNIEnv *env;
        auto stat = vm->GetEnv((void **) (&env), JNI_VERSION_1_6);
        if (stat == JNI_EDETACHED) {
            vm->AttachCurrentThread(&env, nullptr);
        }
        env->DeleteGlobalRef(that);
        if (stat == JNI_EDETACHED) {
            vm->DetachCurrentThread();
        }
    }
};


void onCall(const v8::FunctionCallbackInfo<v8::Value> &info) {
    CHECK(info.Data()->IsExternal());
    auto external = info.Data().As<v8::External>();
    JniCallback *callback = reinterpret_cast<JniCallback * >(external->Value());
    callback->call(info);
}

void JSFunction::Init(JNIEnv *env, jobject thiz) {
    auto runtime = JSContext::Runtime(env, thiz);
    auto data = v8::External::New(runtime->isolate, new JniCallback(runtime, env, thiz, functionOnCallMethod));
    auto func = v8::FunctionTemplate::New(runtime->isolate, onCall, data)->GetFunction();
    auto reference = new v8::Persistent<v8::Value>(runtime->isolate, func);
    JSValue::SetReference(env, thiz, (jlong) reference);
}
