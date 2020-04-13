//
// Created by linroid on 2019/11/8.
//

#include "JSPromise.h"
#include "JSValue.h"
#include "JSError.h"

jclass JSPromise::jClazz;
jmethodID JSPromise::jConstructor;
jfieldID JSPromise::jResolver;

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
    jClazz = (jclass) env->NewGlobalRef(clazz);
    jConstructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/knode/js/JSContext;J)V");
    jResolver = env->GetFieldID(clazz, "resolverPtr", "J");
    env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
    return JNI_OK;
}

void JSPromise::New(JNIEnv *env, jobject jThis) {
    v8::Persistent<v8::Value> *promiseResult = nullptr;
    v8::Persistent<v8::Value> *resolverResult = nullptr;
    V8_SCOPE(env, jThis)
        auto context = runtime->context_.Get(isolate);
        auto resolver = v8::Promise::Resolver::New(context).ToLocalChecked();
        auto promise = resolver->GetPromise();
        resolverResult = new v8::Persistent<v8::Value>(isolate, resolver);
        promiseResult = new v8::Persistent<v8::Value>(isolate, promise);
    V8_END()
    JSValue::SetReference(env, jThis, (jlong) promiseResult);
    env->SetLongField(jThis, jResolver, reinterpret_cast<jlong>(resolverResult));
}

void JSPromise::Reject(JNIEnv *env, jobject jThis, jobject jError) {
    auto value = JSValue::Unwrap(env, jError);
    jlong resolverPtr = env->GetLongField(jThis, jResolver);
    auto resolver = reinterpret_cast<v8::Persistent<v8::Promise::Resolver> *>(resolverPtr);
    v8::Persistent<v8::Value> *error = nullptr;
    V8_CONTEXT(env, jThis, v8::Promise)
        v8::TryCatch tryCatch(runtime->isolate_);
        UNUSED(resolver->Get(isolate)->Reject(context, value->Get(isolate)));
        if (tryCatch.HasCaught()) {
            error = new v8::Persistent<v8::Value>(isolate, tryCatch.Exception());
            return;
        }
    V8_END()
    if (error) {
        JSError::Throw(env, runtime, error);
    }
}

void JSPromise::Resolve(JNIEnv *env, jobject jThis, jobject jValue) {
    auto value = JSValue::Unwrap(env, jValue);
    jlong resolverPtr = env->GetLongField(jThis, jResolver);
    auto resolver = reinterpret_cast<v8::Persistent<v8::Promise::Resolver> *>(resolverPtr);
    v8::Persistent<v8::Value> *error = nullptr;
    V8_CONTEXT(env, jThis, v8::Promise)
        v8::TryCatch tryCatch(runtime->isolate_);
        UNUSED(resolver->Get(isolate)->Resolve(context, value->Get(isolate)));
        if (tryCatch.HasCaught()) {
            error = new v8::Persistent<v8::Value>(isolate, tryCatch.Exception());
            return;
        }
        // runtime->isolate_->RunMicrotasks();
    V8_END()
    if (error) {
        JSError::Throw(env, runtime, error);
    }
}

void JSPromise::Then(JNIEnv *env, jobject jThis, jobject jCallback) {
    auto callback = reinterpret_cast<v8::Persistent<v8::Function> *>(JSValue::GetReference(env, jCallback));
    v8::Persistent<v8::Value> *error = nullptr;
    V8_CONTEXT(env, jThis, v8::Promise)
        v8::TryCatch tryCatch(runtime->isolate_);
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

void JSPromise::Catch(JNIEnv *env, jobject jThis, jobject jCallback) {
    auto callback = reinterpret_cast<v8::Persistent<v8::Function> *>(JSValue::GetReference(env, jCallback));
    v8::Persistent<v8::Value> *error = nullptr;
    V8_CONTEXT(env, jThis, v8::Promise)
        v8::TryCatch tryCatch(runtime->isolate_);
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
