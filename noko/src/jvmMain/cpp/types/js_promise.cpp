#include "js_promise.h"
#include "js_value.h"
#include "js_error.h"

namespace JsPromise {

jclass class_;
jmethodID init_method_id_;

jobject Of(JNIEnv *env, jobject node, jlong pointer) {
  return env->NewObject(class_, init_method_id_, node, pointer);
}

JNICALL jlong New(JNIEnv *env, jclass clazz) {
  V8_SCOPE(env);
  UNUSED(clazz);
  auto context = runtime->context_.Get(isolate);
  auto resolver = v8::Promise::Resolver::New(context).ToLocalChecked();
  return (jlong) new v8::Persistent<v8::Value>(isolate, resolver);
}

void Reject(JNIEnv *env, jobject j_this, jobject j_error) {
  SETUP(env, j_this, v8::Promise::Resolver);

  auto value = JsValue::GetPointer(env, j_error);
  v8::TryCatch try_catch(runtime->isolate_);
  that->Reject(context, value->Get(isolate)).Check();
  if (try_catch.HasCaught()) {
    runtime->Throw(env, try_catch.Exception());
    return;
  }
}

void Resolve(JNIEnv *env, jobject j_this, jobject j_value) {
  SETUP(env, j_this, v8::Promise::Resolver);
  auto value = JsValue::Value(isolate, env, j_value);
  v8::TryCatch try_catch(runtime->isolate_);
  that->Resolve(context, value).Check();
  if (try_catch.HasCaught()) {
    runtime->Throw(env, try_catch.Exception());
    return;
  }
  // runtime->isolate_->RunMicrotasks();
}

void Then(JNIEnv *env, jobject j_this, jobject j_callback) {
  SETUP(env, j_this, v8::Promise);
  auto callback = JsValue::Value(isolate, env, j_callback).As<v8::Function>();
  v8::TryCatch try_catch(runtime->isolate_);
  auto ret = that->Then(context, callback);
  if (ret.IsEmpty()) {
    runtime->Throw(env, try_catch.Exception());
    return;
  }
}

void Catch(JNIEnv *env, jobject j_this, jobject j_callback) {
  SETUP(env, j_this, v8::Promise);
  auto callback = JsValue::Value(isolate, env, j_callback).As<v8::Function>();
  v8::TryCatch try_catch(runtime->isolate_);
  auto ret = that->Catch(context, callback);
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
      {"nativeNew", "()J", (void *) New},
      {"nativeReject", "(Lcom/linroid/noko/types/JsError;)V", (void *) Reject},
      {"nativeResolve", "(Ljava/lang/Object;)V", (void *) Resolve},
      {"nativeThen", "(Lcom/linroid/noko/types/JsFunction;)V", (void *) Then},
      {"nativeCatch", "(Lcom/linroid/noko/types/JsFunction;)V", (void *) Catch},
  };
  class_ = (jclass) env->NewGlobalRef(clazz);
  init_method_id_ = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/noko/Node;J)V");
  env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
  return JNI_OK;
}

}
