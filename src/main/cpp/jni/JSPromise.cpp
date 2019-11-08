//
// Created by linroid on 2019/11/8.
//

#include "JSPromise.h"
#include "macros.h"
#include "JSValue.h"
#include "JSError.h"

jclass JSPromise::jclazz;
jmethodID JSPromise::jconstructor;

jint JSPromise::OnLoad(JNIEnv *env) {
    jclass clazz = env->FindClass("com/linroid/knode/js/JSPromise");
    if (!clazz) {
        return JNI_ERR;
    }
    JNINativeMethod methods[] = {
            {"nativeNew",     "()V",                                  (void *) JSPromise::New},
            {"nativeReject",  "(Lcom/linroid/knode/js/JSError;)V",    (void *) JSPromise::Reject},
            {"nativeResolve", "(Lcom/linroid/knode/js/JSValue;)V",    (void *) JSPromise::Resolve},
            {"nativeThen",    "(Lcom/linroid/knode/js/JSFunction;)V", (void *) JSPromise::Then},
            {"nativeCatch",   "(Lcom/linroid/knode/js/JSFunction;)V", (void *) JSPromise::Catch},
    };
    jclazz = (jclass) env->NewGlobalRef(clazz);
    jconstructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/knode/js/JSContext;J)V");
    env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
    return JNI_OK;
}

void JSPromise::New(JNIEnv *env, jobject jthis) {
    v8::Persistent<v8::Value> *result = nullptr;
    V8_SCOPE(env, jthis)
        auto value = v8::Promise::New(runtime->isolate);
        result = new v8::Persistent<v8::Value>(runtime->isolate, value);
    V8_END()
    JSValue::SetReference(env, jthis, (jlong) result);
}

void JSPromise::Reject(JNIEnv *env, jobject jthis, jobject jerror) {
    auto parameter = JSValue::Unwrap(env, jerror);
    v8::Persistent<v8::Value> *error = nullptr;

    V8_CONTEXT(env, jthis, v8::Promise)
        auto reject = that->Get(V8_UTF_STRING(isolate, "reject")).As<v8::Function>();
        v8::Local<v8::Value> *argv = new v8::Local<v8::Value>[1];
        argv[0] = parameter->Get(isolate);
        v8::TryCatch tryCatch(runtime->isolate);
        auto ret = reject->Call(context, that, 1, argv);
        if (ret.IsEmpty()) {
            error = new v8::Persistent<v8::Value>(isolate, tryCatch.Exception());
            return;
        }
    V8_END()
    if (error) {
        JSError::Throw(env, runtime, error);
    }
}

void JSPromise::Resolve(JNIEnv *env, jobject jthis, jobject jvalue) {
    auto parameter = JSValue::Unwrap(env, jvalue);
    v8::Persistent<v8::Value> *error = nullptr;
    V8_CONTEXT(env, jthis, v8::Promise)
        auto reject = that->Get(V8_UTF_STRING(isolate, "resolve")).As<v8::Function>();
        v8::Local<v8::Value> *argv = new v8::Local<v8::Value>[1];
        argv[0] = parameter->Get(isolate);
        v8::TryCatch tryCatch(runtime->isolate);
        auto ret = reject->Call(context, that, 1, argv);
        if (ret.IsEmpty()) {
            error = new v8::Persistent<v8::Value>(isolate, tryCatch.Exception());
            return;
        }
    V8_END()
    if (error) {
        JSError::Throw(env, runtime, error);
    }
}

void JSPromise::Then(JNIEnv *env, jobject jthis, jobject jcallback) {
    auto callback = reinterpret_cast<v8::Persistent<v8::Function> *>(JSValue::GetReference(env, jcallback));
    v8::Persistent<v8::Value> *error = nullptr;
    V8_CONTEXT(env, jthis, v8::Promise)
        v8::TryCatch tryCatch(runtime->isolate);
        auto ret = that->Then(context, callback->Get(isolate));
        if (ret.IsEmpty()) {
            error = new v8::Persistent<v8::Value>(isolate, tryCatch.Exception());
            return;
        }
    V8_END()
    if (error) {
        JSError::Throw(env, runtime, error);
    }
}

void JSPromise::Catch(JNIEnv *env, jobject jthis, jobject jcallback) {
    auto callback = reinterpret_cast<v8::Persistent<v8::Function> *>(JSValue::GetReference(env, jcallback));
    v8::Persistent<v8::Value> *error = nullptr;
    V8_CONTEXT(env, jthis, v8::Promise)
        v8::TryCatch tryCatch(runtime->isolate);
        auto ret = that->Catch(context, callback->Get(isolate));
        if (ret.IsEmpty()) {
            error = new v8::Persistent<v8::Value>(isolate, tryCatch.Exception());
            return;
        }
    V8_END()
    if (error) {
        JSError::Throw(env, runtime, error);
    }
}
