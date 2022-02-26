#include "JsFunction.h"
#include "JsValue.h"
#include "JsString.h"
#include "JsError.h"
#include "JavaCallback.h"
#include "../EnvHelper.h"

jclass JsFunction::jClazz;
jmethodID JsFunction::jConstructor;
jmethodID JsFunction::jCall;

void staticCallback(const v8::FunctionCallbackInfo<v8::Value> &info) {
  // CHECK(info.Data()->IsExternal());
  auto external = info.Data().As<v8::External>();
  auto callback = reinterpret_cast<JavaCallback *>(external->Value());
  callback->Call(info);
}

static void WeakCallback(const v8::WeakCallbackInfo<JavaCallback> &data) {
  LOGW("ObserverWeakCallback");
  JavaCallback *callback = data.GetParameter();
  EnvHelper env(callback->node->vm_);
  JsValue::SetReference(*env, callback->that, 0);
  delete callback;
}

void JsFunction::New(JNIEnv *env, jobject jThis, jstring jName) {
  const uint16_t *name = env->GetStringChars(jName, nullptr);
  const jint nameLen = env->GetStringLength(jName);
  V8_SCOPE(env, jThis)
  auto callback = new JavaCallback(node, env, jThis, JsValue::jClazz, jCall);
  auto data = v8::External::New(isolate, callback);
  auto context = node->context_.Get(isolate);
  auto func = v8::FunctionTemplate::New(isolate, staticCallback, data)->GetFunction(context).ToLocalChecked();
  func->SetName(V8_STRING(isolate, name, nameLen));

  auto result = new v8::Persistent<v8::Value>(isolate, func);
  result->SetWeak(callback, WeakCallback, v8::WeakCallbackType::kParameter);
  env->ReleaseStringChars(jName, name);
  JsValue::SetReference(env, jThis, (jlong) result);
}

jobject JsFunction::Call(JNIEnv *env, jobject jThis, jobject jReceiver, jobjectArray jParameters) {
  auto receiver = JsValue::Unwrap(env, jReceiver);

  int argc = env->GetArrayLength(jParameters);
  v8::Persistent<v8::Value> *parameters[argc];
  for (int i = 0; i < argc; ++i) {
    auto jElement = env->GetObjectArrayElement(jParameters, i);
    parameters[i] = JsValue::Unwrap(env, jElement);
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
    node->Throw(env, tryCatch.Exception());
    return nullptr;
  }
  return node->ToJava(env, result.ToLocalChecked());
}

jint JsFunction::OnLoad(JNIEnv *env) {
  jclass clazz = env->FindClass("com/linroid/noko/types/JsFunction");
  if (!clazz) {
    return JNI_ERR;
  }

  JNINativeMethod methods[] = {
      {"nativeCall", "(Lcom/linroid/noko/types/JsValue;[Lcom/linroid/noko/types/JsValue;)Lcom/linroid/noko/types/JsValue;", (void *) JsFunction::Call},
      {"nativeNew",  "(Ljava/lang/String;)V",                                                                               (void *) JsFunction::New},
  };
  jClazz = (jclass) env->NewGlobalRef(clazz);
  jConstructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/noko/Node;J)V");
  jCall = env->GetMethodID(clazz, "onCall",
                           "(Lcom/linroid/noko/types/JsValue;[Lcom/linroid/noko/types/JsValue;)Lcom/linroid/noko/types/JsValue;");
  env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
  return JNI_OK;
}
