#include "js_function.h"
#include "js_value.h"
#include "js_error.h"
#include "../util/java_callback.h"
#include "../util/env_helper.h"

namespace JsFunction {

jclass class_;
jclass object_class_;
jmethodID init_method_id_;
jmethodID call_method_id_;
jfieldID name_field_id_;

jobject Of(JNIEnv *env, jobject node, jlong pointer) {
  return env->NewObject(class_, init_method_id_, node, pointer);
}

bool Is(JNIEnv *env, jobject obj) {
  return env->IsInstanceOf(obj, class_);
}

void StaticCallback(const v8::FunctionCallbackInfo<v8::Value> &info) {
  // CHECK(info.Data()->IsExternal());
  auto external = info.Data().As<v8::External>();
  auto callback = reinterpret_cast<JavaCallback *>(external->Value());
  callback->Call(info);
}

static void WeakCallback(const v8::WeakCallbackInfo<JavaCallback> &data) {
  LOGI("WeakCallback");
  JavaCallback *callback = data.GetParameter();
  EnvHelper env(callback->runtime_->Jvm());
  JsValue::SetPointer(*env, callback->that_, nullptr);
  delete callback;
}

v8::Local<v8::Function> Init(JNIEnv *env, jobject j_this) {
  auto j_name = (jstring) env->GetObjectField(j_this, name_field_id_);
  const uint16_t *name = env->GetStringChars(j_name, nullptr);
  const jint name_len = env->GetStringLength(j_name);

  auto runtime = Runtime::Current();
  if (runtime == nullptr) {
    LOGE("GetRuntime runtime nullptr %s(%d)-<%s>", __FILE__, __LINE__, __FUNCTION__);
    env->FatalError("GetRuntime returns nullptr");
    abort();
  }
  auto isolate = runtime->Isolate();
  v8::Locker locker(isolate);
  v8::EscapableHandleScope handle_scope(isolate);

  auto callback = new JavaCallback(runtime, env, j_this, object_class_, call_method_id_);
  auto data = v8::External::New(isolate, callback);
  auto context = runtime->Context();
  auto func = v8::FunctionTemplate::New(isolate, StaticCallback, data)->GetFunction(context).ToLocalChecked();
  func->SetName(V8_STRING(isolate, name, name_len));

  auto result = new v8::Persistent<v8::Value>(isolate, func);
  result->SetWeak(callback, WeakCallback, v8::WeakCallbackType::kParameter);
  env->ReleaseStringChars(j_name, name);
  JsValue::SetPointer(env, j_this, result);
  return handle_scope.Escape(func);
}

JNICALL jobject Call(
    JNIEnv *env,
    jobject j_this,
    jobject j_receiver,
    jobjectArray j_parameters
) {
  SETUP(env, j_this, v8::Function);
  int argc = env->GetArrayLength(j_parameters);
  v8::Persistent<v8::Value> *parameters[argc];
  auto *argv = new v8::Local<v8::Value>[argc];
  v8::TryCatch try_catch(isolate);
  for (int i = 0; i < argc; ++i) {
    auto element = env->GetObjectArrayElement(j_parameters, i);
    parameters[i] = JsValue::GetPointer(env, element);
    argv[i] = JsValue::Value(env, element);
    env->DeleteLocalRef(element);
  }
  auto receiver = JsValue::GetPointer(env, j_receiver)->Get(isolate);
  auto result = that->Call(context, receiver, argc, argv);
  if (result.IsEmpty() || try_catch.HasCaught()) {
    runtime->Throw(env, try_catch.Exception());
    return nullptr;
  }
  return JsValue::Of(env, result.ToLocalChecked());
}

jint OnLoad(JNIEnv *env) {
  jclass clazz = env->FindClass("com/linroid/noko/types/JsFunction");
  if (!clazz) {
    return JNI_ERR;
  }

  JNINativeMethod methods[] = {
      {"nativeCall", "(Lcom/linroid/noko/types/JsValue;[Ljava/lang/Object;)Ljava/lang/Object;", (void *) Call},
  };
  class_ = (jclass) env->NewGlobalRef(clazz);
  init_method_id_ = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/noko/Node;J)V");
  call_method_id_ = env->GetMethodID(clazz, "onCall",
                                     "(Lcom/linroid/noko/types/JsValue;[Ljava/lang/Object;)Ljava/lang/Object;");

  name_field_id_ = env->GetFieldID(clazz, "name", "Ljava/lang/String;");
  object_class_ = (jclass) env->NewGlobalRef(env->FindClass("java/lang/Object"));
  env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
  return JNI_OK;
}

}