#include "js_function.h"
#include "js_value.h"
#include "string.h"
#include "js_error.h"
#include "../util/java_callback.h"
#include "../util/env_helper.h"

namespace JsFunction {

jclass class_;
jclass object_class_;
jmethodID init_method_id_;
jmethodID call_method_id_;

jobject Of(JNIEnv *env, jobject node, jlong pointer) {
  return env->NewObject(class_, init_method_id_, node, pointer);
}

void StaticCallback(const v8::FunctionCallbackInfo<v8::Value> &info) {
  // CHECK(info.Data()->IsExternal());
  auto external = info.Data().As<v8::External>();
  auto callback = reinterpret_cast<JavaCallback *>(external->Value());
  callback->Call(info);
}

static void WeakCallback(const v8::WeakCallbackInfo<JavaCallback> &data) {
  JavaCallback *callback = data.GetParameter();
  EnvHelper env(callback->runtime_->vm_);
  JsValue::SetPointer(*env, callback->that_, nullptr);
  delete callback;
}

void New(JNIEnv *env, jobject j_this, jstring jName) {
  const uint16_t *name = env->GetStringChars(jName, nullptr);
  const jint name_len = env->GetStringLength(jName);
  V8_SCOPE(env, j_this)
  auto callback = new JavaCallback(runtime, env, j_this, object_class_, call_method_id_);
  auto data = v8::External::New(isolate, callback);
  auto context = runtime->context_.Get(isolate);
  auto func = v8::FunctionTemplate::New(isolate, StaticCallback, data)->GetFunction(context).ToLocalChecked();
  func->SetName(V8_STRING(isolate, name, name_len));

  auto result = new v8::Persistent<v8::Value>(isolate, func);
  result->SetWeak(callback, WeakCallback, v8::WeakCallbackType::kParameter);
  env->ReleaseStringChars(jName, name);
  JsValue::SetPointer(env, j_this, result);
}

jobject Call(JNIEnv *env, jobject j_this, jobject j_receiver, jobjectArray j_parameters) {
  auto receiver = JsValue::GetPointer(env, j_receiver);

  int argc = env->GetArrayLength(j_parameters);
  v8::Persistent<v8::Value> *parameters[argc];
  for (int i = 0; i < argc; ++i) {
    auto element = env->GetObjectArrayElement(j_parameters, i);
    parameters[i] = JsValue::GetPointer(env, element);
    env->DeleteLocalRef(element);
  }

  SETUP(env, j_this, v8::Function)
  auto *argv = new v8::Local<v8::Value>[argc];
  for (int i = 0; i < argc; ++i) {
    argv[i] = parameters[i]->Get(isolate);
  }
  v8::TryCatch try_catch(isolate);
  auto result = that->Call(context, receiver->Get(isolate), argc, argv);
  if (result.IsEmpty()) {
    runtime->Throw(env, try_catch.Exception());
    return nullptr;
  }
  return runtime->ToJava(env, result.ToLocalChecked());
}

jint OnLoad(JNIEnv *env) {
  jclass clazz = env->FindClass("com/linroid/noko/types/JsFunction");
  if (!clazz) {
    return JNI_ERR;
  }

  JNINativeMethod methods[] = {
      {"nativeCall", "(Lcom/linroid/noko/types/JsValue;[Ljava/lang/Object;)Ljava/lang/Object;", (void *) Call},
      {"nativeNew", "(Ljava/lang/String;)V", (void *) New},
  };
  class_ = (jclass) env->NewGlobalRef(clazz);
  init_method_id_ = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/noko/Node;J)V");
  call_method_id_ = env->GetMethodID(clazz, "onCall",
                                     "(Lcom/linroid/noko/types/JsValue;[Ljava/lang/Object;)Ljava/lang/Object;");

  object_class_ = (jclass) env->NewGlobalRef(env->FindClass("java/lang/Object"));
  env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
  return JNI_OK;
}

}