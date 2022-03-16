#include "js_function.h"
#include "js_value.h"
#include "js_error.h"
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

  v8::Local<v8::Function> Value(JNIEnv *env, jobject obj) {
    auto callback = new JavaCallback(env, obj);
    auto runtime = Runtime::Current();
    return callback->Get(runtime->Isolate());
  }

  JNICALL jobject Call(
      JNIEnv *env,
      jobject j_this,
      jobject j_receiver,
      jobjectArray j_parameters
  ) {
    SETUP(env, j_this, v8::Function);
    int argc = env->GetArrayLength(j_parameters);
    auto *argv = new v8::Local<v8::Value>[argc];
    v8::TryCatch try_catch(isolate);
    for (int i = 0; i < argc; ++i) {
      auto element = env->GetObjectArrayElement(j_parameters, i);
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
        {"nativeCall", "(Lcom/linroid/noko/types/JsValue;[Ljava/lang/Object;)Ljava/lang/Object;",
         (void *) Call},
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

  JavaCallback::JavaCallback(
      JNIEnv *env,
      jobject obj
  ) : obj_(env->NewGlobalRef(obj)) {
    auto runtime = Runtime::Current();
    auto isolate = runtime->Isolate();

    auto j_name = (jstring) env->GetObjectField(obj, name_field_id_);
    const uint16_t *name_chars = env->GetStringChars(j_name, nullptr);
    const jint name_len = env->GetStringLength(j_name);

    v8::Locker locker(isolate);
    v8::HandleScope handle_scope(isolate);

    auto name = V8_STRING(isolate, name_chars, name_len);
    env->ReleaseStringChars(j_name, name_chars);

    auto data = v8::External::New(isolate, this);
    auto context = runtime->Context();
    auto func = v8::FunctionTemplate::New(isolate, StaticCallback, data)
        ->GetFunction(context).ToLocalChecked();
    func->SetName(name);

    value_.Reset(isolate, func);
    value_.SetWeak(this, WeakCallback, v8::WeakCallbackType::kParameter);

    JsValue::SetPointer(env, obj, reinterpret_cast<v8::Persistent<v8::Value> *>(&value_));
    node::AddEnvironmentCleanupHook(isolate, CleanupHook, this);
  }

  void JavaCallback::Call(const v8::FunctionCallbackInfo<v8::Value> &info) const {
    auto runtime = Runtime::Current();
    EnvHelper env(runtime->Jvm());
    auto parameters = env->NewObjectArray(info.Length(), object_class_, nullptr);
    for (int i = 0; i < info.Length(); ++i) {
      v8::Local<v8::Value> element = info[i];
      auto obj = JsValue::Of(*env, element);
      env->SetObjectArrayElement(parameters, i, obj);
    }
    auto caller = (v8::Local<v8::Value>) info.This();
    auto j_caller = JsValue::Of(*env, caller);
    auto j_result = env->CallObjectMethod(obj_, call_method_id_, j_caller, parameters);
    env->DeleteLocalRef(j_caller);
    env->DeleteLocalRef(parameters);

    if (env->ExceptionCheck()) {
      // TODO: Throw exception into v8
      info.GetReturnValue().Set(v8::Undefined(info.GetIsolate()));
      env->Throw(env->ExceptionOccurred());
      if (env.HasAttached()) {
        runtime->Jvm()->DetachCurrentThread();
      }
      return;
    }

    if (j_result != nullptr) {
      auto result = JsValue::Value(*env, j_result);
      info.GetReturnValue().Set(result);
    }
  }

  v8::Local<v8::Function> JavaCallback::Get(v8::Isolate *isolate) {
    return value_.Get(isolate);
  }

  void JavaCallback::CleanupHook(void *arg) {
    auto *self = static_cast<JavaCallback *>(arg);
    delete self;
  }

  void JavaCallback::StaticCallback(const v8::FunctionCallbackInfo<v8::Value> &info) {
    // CHECK(info.Data()->IsExternal());
    auto external = info.Data().As<v8::External>();
    auto callback = reinterpret_cast<JavaCallback *>(external->Value());
    callback->Call(info);
  }

  void JavaCallback::WeakCallback(const v8::WeakCallbackInfo<JavaCallback> &data) {
    JavaCallback *callback = data.GetParameter();
    auto runtime = Runtime::Current();
    EnvHelper env(runtime->Jvm());
    JsValue::SetPointer(*env, callback->obj_, nullptr);
    delete callback;
  }

  JavaCallback::~JavaCallback() {
    auto runtime = Runtime::Current();
    node::RemoveEnvironmentCleanupHook(runtime->Isolate(), CleanupHook, this);
    EnvHelper env(runtime->Jvm());
    env->DeleteGlobalRef(obj_);
  }

}