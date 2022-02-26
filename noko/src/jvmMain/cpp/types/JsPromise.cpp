#include "JsPromise.h"
#include "JsValue.h"
#include "JsError.h"

jclass JsPromise::jClazz;
jmethodID JsPromise::jConstructor;
jfieldID JsPromise::jResolver;

jint JsPromise::OnLoad(JNIEnv *env) {
  jclass clazz = env->FindClass("com/linroid/noko/types/JsPromise");
  if (!clazz) {
    return JNI_ERR;
  }
  JNINativeMethod methods[] = {
      {"nativeNew",     "()V",                                    (void *) JsPromise::New},
      {"nativeReject",  "(Lcom/linroid/noko/types/JsError;)V",    (void *) JsPromise::Reject},
      {"nativeResolve", "(Lcom/linroid/noko/types/JsValue;)V",    (void *) JsPromise::Resolve},
      {"nativeThen",    "(Lcom/linroid/noko/types/JsFunction;)V", (void *) JsPromise::Then},
      {"nativeCatch",   "(Lcom/linroid/noko/types/JsFunction;)V", (void *) JsPromise::Catch},
  };
  jClazz = (jclass) env->NewGlobalRef(clazz);
  jConstructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/noko/Node;J)V");
  jResolver = env->GetFieldID(clazz, "resolverPointer", "J");
  env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
  return JNI_OK;
}

void JsPromise::New(JNIEnv *env, jobject jThis) {
  V8_SCOPE(env, jThis)
  auto context = node->context_.Get(isolate);
  auto resolver = v8::Promise::Resolver::New(context).ToLocalChecked();
  auto promise = resolver->GetPromise();
  auto resolverResult = new v8::Persistent<v8::Value>(isolate, resolver);
  auto promiseResult = new v8::Persistent<v8::Value>(isolate, promise);
  JsValue::SetPointer(env, jThis, (jlong) promiseResult);
  env->SetLongField(jThis, jResolver, reinterpret_cast<jlong>(resolverResult));
}

void JsPromise::Reject(JNIEnv *env, jobject jThis, jobject jError) {
  auto value = JsValue::Unwrap(env, jError);
  jlong resolverPtr = env->GetLongField(jThis, jResolver);
  auto resolver = reinterpret_cast<v8::Persistent<v8::Promise::Resolver> *>(resolverPtr);
  SETUP(env, jThis, v8::Promise)
  v8::TryCatch tryCatch(node->isolate_);
  UNUSED(resolver->Get(isolate)->Reject(context, value->Get(isolate)));
  if (tryCatch.HasCaught()) {
    node->Throw(env, tryCatch.Exception());
    return;
  }
}

void JsPromise::Resolve(JNIEnv *env, jobject jThis, jobject jValue) {
  auto value = JsValue::Unwrap(env, jValue);
  jlong resolverPtr = env->GetLongField(jThis, jResolver);
  auto resolver = reinterpret_cast<v8::Persistent<v8::Promise::Resolver> *>(resolverPtr);
  SETUP(env, jThis, v8::Promise)
  v8::TryCatch tryCatch(node->isolate_);
  UNUSED(resolver->Get(isolate)->Resolve(context, value->Get(isolate)));
  if (tryCatch.HasCaught()) {
    node->Throw(env, tryCatch.Exception());
    return;
  }
  // node->isolate_->RunMicrotasks();
}

void JsPromise::Then(JNIEnv *env, jobject jThis, jobject jCallback) {
  auto callback = reinterpret_cast<v8::Persistent<v8::Function> *>(JsValue::GetPointer(env,
                                                                                       jCallback));
  SETUP(env, jThis, v8::Promise)
  v8::TryCatch tryCatch(node->isolate_);
  auto ret = that->Then(context, callback->Get(isolate));
  if (ret.IsEmpty()) {
    node->Throw(env, tryCatch.Exception());
    return;
  }
}

void JsPromise::Catch(JNIEnv *env, jobject jThis, jobject jCallback) {
  auto callback = reinterpret_cast<v8::Persistent<v8::Function> *>(JsValue::GetPointer(env,
                                                                                       jCallback));
  SETUP(env, jThis, v8::Promise)
  v8::TryCatch tryCatch(node->isolate_);
  auto ret = that->Catch(context, callback->Get(isolate));
  if (ret.IsEmpty()) {
    node->Throw(env, tryCatch.Exception());
    return;
  }
}
