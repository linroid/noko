#include "JSPromise.h"
#include "JSValue.h"
#include "JSError.h"

jclass JSPromise::jClazz;
jmethodID JSPromise::jConstructor;
jfieldID JSPromise::jResolver;

jint JSPromise::OnLoad(JNIEnv *env) {
  jclass clazz = env->FindClass("com/linroid/noko/types/JSPromise");
  if (!clazz) {
    return JNI_ERR;
  }
  JNINativeMethod methods[] = {
      {"nativeNew",     "()V",                                    (void *) JSPromise::New},
      {"nativeReject",  "(Lcom/linroid/noko/types/JSError;)V",    (void *) JSPromise::Reject},
      {"nativeResolve", "(Lcom/linroid/noko/types/JSValue;)V",    (void *) JSPromise::Resolve},
      {"nativeThen",    "(Lcom/linroid/noko/types/JSFunction;)V", (void *) JSPromise::Then},
      {"nativeCatch",   "(Lcom/linroid/noko/types/JSFunction;)V", (void *) JSPromise::Catch},
  };
  jClazz = (jclass) env->NewGlobalRef(clazz);
  jConstructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/noko/types/JSContext;J)V");
  jResolver = env->GetFieldID(clazz, "resolverPtr", "J");
  env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
  return JNI_OK;
}

void JSPromise::New(JNIEnv *env, jobject jThis) {
  V8_SCOPE(env, jThis)
  auto context = noko->context_.Get(isolate);
  auto resolver = v8::Promise::Resolver::New(context).ToLocalChecked();
  auto promise = resolver->GetPromise();
  auto resolverResult = new v8::Persistent<v8::Value>(isolate, resolver);
  auto promiseResult = new v8::Persistent<v8::Value>(isolate, promise);
  JSValue::SetReference(env, jThis, (jlong) promiseResult);
  env->SetLongField(jThis, jResolver, reinterpret_cast<jlong>(resolverResult));
}

void JSPromise::Reject(JNIEnv *env, jobject jThis, jobject jError) {
  auto value = JSValue::Unwrap(env, jError);
  jlong resolverPtr = env->GetLongField(jThis, jResolver);
  auto resolver = reinterpret_cast<v8::Persistent<v8::Promise::Resolver> *>(resolverPtr);
  SETUP(env, jThis, v8::Promise)
  v8::TryCatch tryCatch(noko->isolate_);
  UNUSED(resolver->Get(isolate)->Reject(context, value->Get(isolate)));
  if (tryCatch.HasCaught()) {
    noko->Throw(env, tryCatch.Exception());
    return;
  }
}

void JSPromise::Resolve(JNIEnv *env, jobject jThis, jobject jValue) {
  auto value = JSValue::Unwrap(env, jValue);
  jlong resolverPtr = env->GetLongField(jThis, jResolver);
  auto resolver = reinterpret_cast<v8::Persistent<v8::Promise::Resolver> *>(resolverPtr);
  SETUP(env, jThis, v8::Promise)
  v8::TryCatch tryCatch(noko->isolate_);
  UNUSED(resolver->Get(isolate)->Resolve(context, value->Get(isolate)));
  if (tryCatch.HasCaught()) {
    noko->Throw(env, tryCatch.Exception());
    return;
  }
  // noko->isolate_->RunMicrotasks();
}

void JSPromise::Then(JNIEnv *env, jobject jThis, jobject jCallback) {
  auto callback = reinterpret_cast<v8::Persistent<v8::Function> *>(JSValue::GetReference(env, jCallback));
  SETUP(env, jThis, v8::Promise)
  v8::TryCatch tryCatch(noko->isolate_);
  auto ret = that->Then(context, callback->Get(isolate));
  if (ret.IsEmpty()) {
    noko->Throw(env, tryCatch.Exception());
    return;
  }
}

void JSPromise::Catch(JNIEnv *env, jobject jThis, jobject jCallback) {
  auto callback = reinterpret_cast<v8::Persistent<v8::Function> *>(JSValue::GetReference(env, jCallback));
  SETUP(env, jThis, v8::Promise)
  v8::TryCatch tryCatch(noko->isolate_);
  auto ret = that->Catch(context, callback->Get(isolate));
  if (ret.IsEmpty()) {
    noko->Throw(env, tryCatch.Exception());
    return;
  }
}