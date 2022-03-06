#include "js_promise.h"
#include "js_value.h"
#include "js_error.h"

namespace JsPromise {

jclass class_;
jmethodID init_method_id_;
jfieldID resolver_field_id_;

jobject Of(JNIEnv *env, jobject node, jlong pointer) {
  return env->NewObject(class_, init_method_id_, node, pointer);
}

void New(JNIEnv *env, jobject j_this) {
  V8_SCOPE(env, j_this)
  auto context = runtime->context_.Get(isolate);
  auto resolver = v8::Promise::Resolver::New(context).ToLocalChecked();
  auto promise = resolver->GetPromise();
  auto resolver_result = new v8::Persistent<v8::Value>(isolate, resolver);
  auto result = new v8::Persistent<v8::Value>(isolate, promise);
  JsValue::SetPointer(env, j_this, result);
  env->SetLongField(j_this, resolver_field_id_, reinterpret_cast<jlong>(resolver_result));
}

void Reject(JNIEnv *env, jobject j_this, jobject j_error) {
  auto value = JsValue::GetPointer(env, j_error);
  jlong resolver_pointer = env->GetLongField(j_this, resolver_field_id_);
  auto resolver = reinterpret_cast<v8::Persistent<v8::Promise::Resolver> *>(resolver_pointer);
  SETUP(env, j_this, v8::Promise)
  v8::TryCatch try_catch(runtime->isolate_);
  UNUSED(resolver->Get(isolate)->Reject(context, value->Get(isolate)));
  if (try_catch.HasCaught()) {
    runtime->Throw(env, try_catch.Exception());
    return;
  }
}

void Resolve(JNIEnv *env, jobject j_this, jobject j_value) {
  auto value = JsValue::GetPointer(env, j_value);
  jlong resolver_pointer = env->GetLongField(j_this, resolver_field_id_);
  auto resolver = reinterpret_cast<v8::Persistent<v8::Promise::Resolver> *>(resolver_pointer);
  SETUP(env, j_this, v8::Promise)
  v8::TryCatch try_catch(runtime->isolate_);
  UNUSED(resolver->Get(isolate)->Resolve(context, value->Get(isolate)));
  if (try_catch.HasCaught()) {
    runtime->Throw(env, try_catch.Exception());
    return;
  }
  // runtime->isolate_->RunMicrotasks();
}

void Then(JNIEnv *env, jobject j_this, jobject j_callback) {
  auto callback = reinterpret_cast<v8::Persistent<v8::Function> *>(JsValue::GetPointer(env,
                                                                                       j_callback));
  SETUP(env, j_this, v8::Promise)
  v8::TryCatch try_catch(runtime->isolate_);
  auto ret = that->Then(context, callback->Get(isolate));
  if (ret.IsEmpty()) {
    runtime->Throw(env, try_catch.Exception());
    return;
  }
}

void Catch(JNIEnv *env, jobject j_this, jobject j_callback) {
  auto callback = reinterpret_cast<v8::Persistent<v8::Function> *>(JsValue::GetPointer(env,
                                                                                       j_callback));
  SETUP(env, j_this, v8::Promise)
  v8::TryCatch try_catch(runtime->isolate_);
  auto ret = that->Catch(context, callback->Get(isolate));
  if (ret.IsEmpty()) {
    runtime->Throw(env, try_catch.Exception());
    return;
  }
}

jint OnLoad(JNIEnv *env) {
  jclass clazz = env->FindClass("com/linroid/noko/types/JsPromise");
  if (!clazz) {
    return JNI_ERR;
  }
  JNINativeMethod methods[] = {
      {"nativeNew", "()V", (void *) New},
      {"nativeReject", "(Lcom/linroid/noko/types/JsError;)V", (void *) Reject},
      {"nativeResolve", "(Ljava/lang/Object;)V", (void *) Resolve},
      {"nativeThen", "(Lcom/linroid/noko/types/JsFunction;)V", (void *) Then},
      {"nativeCatch", "(Lcom/linroid/noko/types/JsFunction;)V", (void *) Catch},
  };
  class_ = (jclass) env->NewGlobalRef(clazz);
  init_method_id_ = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/noko/Node;J)V");
  resolver_field_id_ = env->GetFieldID(clazz, "resolverPointer", "J");
  env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
  return JNI_OK;
}

}
