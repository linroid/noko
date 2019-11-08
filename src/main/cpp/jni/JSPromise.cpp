//
// Created by linroid on 2019/11/8.
//

#include "JSPromise.h"
#include "macros.h"
#include "JSValue.h"
#include "JSError.h"

jclass JSPromise::jclazz;
jmethodID JSPromise::jconstructor;
jfieldID JSPromise::jresolver;

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
    jresolver = env->GetFieldID(clazz, "resolverPtr", "J");
    env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
    return JNI_OK;
}

void JSPromise::New(JNIEnv *env, jobject jthis) {
    v8::Persistent<v8::Value> *promiseResult = nullptr;
    v8::Persistent<v8::Value> *resolverResult = nullptr;
    V8_SCOPE(env, jthis)
        auto context = runtime->context.Get(isolate);
        auto resolver = v8::Promise::Resolver::New(context).ToLocalChecked();
        auto promise = resolver->GetPromise();
        resolverResult = new v8::Persistent<v8::Value>(isolate, resolver);
        promiseResult = new v8::Persistent<v8::Value>(isolate, promise);
    V8_END()
    JSValue::SetReference(env, jthis, (jlong) promiseResult);
    env->SetLongField(jthis, jresolver, reinterpret_cast<jlong>(resolverResult));
}

void JSPromise::Reject(JNIEnv *env, jobject jthis, jobject jerror) {
    auto value = JSValue::Unwrap(env, jerror);
    jlong resolverPtr = env->GetLongField(jthis, jresolver);
    auto resolver = reinterpret_cast<v8::Persistent<v8::Promise::Resolver> *>(resolverPtr);
    v8::Persistent<v8::Value> *error = nullptr;
    V8_CONTEXT(env, jthis, v8::Promise)
        v8::TryCatch tryCatch(runtime->isolate);
        resolver->Get(isolate)->Reject(context, value->Get(isolate));
        if (tryCatch.HasCaught()) {
            error = new v8::Persistent<v8::Value>(isolate, tryCatch.Exception());
            return;
        }
    V8_END()
    if (error) {
        JSError::Throw(env, runtime, error);
    }
}

void JSPromise::Resolve(JNIEnv *env, jobject jthis, jobject jvalue) {
    auto value = JSValue::Unwrap(env, jvalue);
    jlong resolverPtr = env->GetLongField(jthis, jresolver);
    auto resolver = reinterpret_cast<v8::Persistent<v8::Promise::Resolver> *>(resolverPtr);
    v8::Persistent<v8::Value> *error = nullptr;
    V8_CONTEXT(env, jthis, v8::Promise)
        v8::TryCatch tryCatch(runtime->isolate);
        resolver->Get(isolate)->Resolve(context, value->Get(isolate));
        if (tryCatch.HasCaught()) {
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
    LOGV("Then");
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
    LOGV("Catch");
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
