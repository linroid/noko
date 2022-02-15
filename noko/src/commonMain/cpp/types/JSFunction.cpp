#include "JSFunction.h"
#include "JSValue.h"
#include "JSContext.h"
#include "JSString.h"
#include "JSError.h"
#include "JavaCallback.h"
#include "../EnvHelper.h"

jclass JSFunction::jClazz;
jmethodID JSFunction::jConstructor;
jmethodID JSFunction::jCall;

void staticCallback(const v8::FunctionCallbackInfo<v8::Value> &info) {
  // CHECK(info.Data()->IsExternal());
  auto external = info.Data().As<v8::External>();
  auto callback = reinterpret_cast<JavaCallback *>(external->Value());
  callback->Call(info);
}

static void WeakCallback(const v8::WeakCallbackInfo<JavaCallback> &data) {
  LOGW("ObserverWeakCallback");
  JavaCallback *callback = data.GetParameter();
  EnvHelper env(callback->runtime->vm_);
  JSValue::SetReference(*env, callback->that, 0);
  delete callback;
}

void JSFunction::New(JNIEnv *env, jobject jThis, jstring jName) {
  const uint16_t *name = env->GetStringChars(jName, nullptr);
  const jint nameLen = env->GetStringLength(jName);
  V8_SCOPE(env, jThis)
  auto callback = new JavaCallback(runtime, env, jThis, JSValue::jClazz, jCall);
  auto data = v8::External::New(isolate, callback);
  auto context = runtime->context_.Get(isolate);
  auto func = v8::FunctionTemplate::New(isolate, staticCallback, data)->GetFunction(context).ToLocalChecked();
  func->SetName(V8_STRING(isolate, name, nameLen));

  auto result = new v8::Persistent<v8::Value>(isolate, func);
  result->SetWeak(callback, WeakCallback, v8::WeakCallbackType::kParameter);
  env->ReleaseStringChars(jName, name);
  JSValue::SetReference(env, jThis, (jlong) result);
}

jobject JSFunction::Call(JNIEnv *env, jobject jThis, jobject jReceiver, jobjectArray jParameters) {
  auto receiver = JSValue::Unwrap(env, jReceiver);

  int argc = env->GetArrayLength(jParameters);
  v8::Persistent<v8::Value> *parameters[argc];
  for (int i = 0; i < argc; ++i) {
    auto jElement = env->GetObjectArrayElement(jParameters, i);
    parameters[i] = JSValue::Unwrap(env, jElement);
    env->DeleteLocalRef(jElement);
  }

  SETUP(env, jThis, v8::Function)
  auto *argv = new v8::Local<v8::Value>[argc];
  for (int i = 0; i < argc; ++i) {
    argv[i] = parameters[i]->Get(isolate);
  }
  v8::TryCatch tryCatch(isolate);
  auto result = that->Call(context, receiver->Get(isolate), argc, argv);
  if (result.IsEmpty()) {
    runtime->Throw(env, tryCatch.Exception());
    return nullptr;
  }
  return runtime->ToJava(env, result.ToLocalChecked());
}

jint JSFunction::OnLoad(JNIEnv *env) {
  jclass clazz = env->FindClass("com/linroid/noko/type/JSFunction");
  if (!clazz) {
    return JNI_ERR;
  }

  JNINativeMethod methods[] = {
    {"nativeCall", "(Lcom/linroid/noko/type/JSValue;[Lcom/linroid/noko/type/JSValue;)Lcom/linroid/noko/type/JSValue;", (void *) JSFunction::Call},
    {"nativeNew",  "(Ljava/lang/String;)V",                                                                         (void *) JSFunction::New},
  };
  jClazz = (jclass) env->NewGlobalRef(clazz);
  jConstructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/noko/type/JSContext;J)V");
  jCall = env->GetMethodID(clazz, "onCall", "(Lcom/linroid/noko/type/JSValue;[Lcom/linroid/noko/type/JSValue;)Lcom/linroid/noko/type/JSValue;");
  env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
  return JNI_OK;
}
