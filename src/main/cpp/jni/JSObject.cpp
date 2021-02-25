//
// Created by linroid on 2019-10-20.
//

#include <jni.h>
#include <node.h>
#include "JSObject.h"
#include "JSValue.h"
#include "JSString.h"
#include "JSUndefined.h"
#include "JSNull.h"
#include "../JniPropertyObserver.h"

jclass JSObject::jClazz;
jmethodID JSObject::jConstructor;

JNICALL void JSObject::Set(JNIEnv *env, jobject jThis, jstring jKey, jobject jValue) {
  const jchar *key = env->GetStringChars(jKey, nullptr);
  const jint keyLen = env->GetStringLength(jKey);

  auto value = JSValue::Unwrap(env, jValue);
  SETUP(env, jThis, v8::Object)
  // CHECK_NOT_NULL(*that);
  UNUSED(that->Set(context, V8_STRING(isolate, key, keyLen), value->Get(isolate)));
  env->ReleaseStringChars(jKey, key);
}

JNICALL jobject JSObject::Get(JNIEnv *env, jobject jThis, jstring jKey) {
  const uint16_t *key = env->GetStringChars(jKey, nullptr);
  const jint keyLen = env->GetStringLength(jKey);
  SETUP(env, jThis, v8::Object)
  auto value = that->Get(context, V8_STRING(isolate, key, keyLen)).ToLocalChecked();
  env->ReleaseStringChars(jKey, key);
  return runtime->ToJava(env, value);
}

jboolean JSObject::Has(JNIEnv *env, jobject jThis, jstring jKey) {
  const uint16_t *key = env->GetStringChars(jKey, nullptr);
  const jint keyLen = env->GetStringLength(jKey);
  SETUP(env, jThis, v8::Object)
  bool result = that->Has(context, V8_STRING(isolate, key, keyLen)).ToChecked();
  env->ReleaseStringChars(jKey, key);
  return static_cast<jboolean>(result);
}

jobjectArray JSObject::Keys(JNIEnv *env, jobject jThis) {
  SETUP(env, jThis, v8::Object)
  v8::TryCatch tryCatch(isolate);
  auto names = that->GetPropertyNames(context);
  if (names.IsEmpty()) {
    runtime->Throw(env, tryCatch.Exception());
    return nullptr;
  }
  auto array = names.ToLocalChecked();
  auto length = array->Length();
  auto result = env->NewObjectArray(length, env->FindClass("java/lang/String"), nullptr);
  for (int i = 0; i < length; i++) {
    auto element = array->Get(context, i);
    if (element.IsEmpty()) {
      runtime->Throw(env, tryCatch.Exception());
      return nullptr;
    }
    v8::String::Value unicodeString(isolate, element.ToLocalChecked());
    uint16_t *unicodeChars = *unicodeString;
    env->SetObjectArrayElement(result, i, env->NewString(unicodeChars, unicodeString.length()));
  }
  return result;
}

void JSObject::New(JNIEnv *env, jobject jThis) {
  V8_SCOPE(env, jThis)
  auto value = v8::Object::New(runtime->isolate_);
  auto result = new v8::Persistent<v8::Value>(runtime->isolate_, value);
  JSValue::SetReference(env, jThis, (jlong) result);
}

void JSObject::Delete(JNIEnv *env, jobject jThis, jstring jKey) {
  const uint16_t *key = env->GetStringChars(jKey, nullptr);
  const jint keyLen = env->GetStringLength(jKey);
  SETUP(env, jThis, v8::Object)
  UNUSED(that->Delete(context, V8_STRING(isolate, key, keyLen)));
  env->ReleaseStringChars(jKey, key);
}

static void GetterCallback(const v8::FunctionCallbackInfo<v8::Value> &info) {
  auto holder = info.Data().As<v8::Object>();
  auto context = info.GetIsolate()->GetCurrentContext();
  if (holder->Has(context, 0).ToChecked()) {
    info.GetReturnValue().Set(holder->Get(context, 0).ToLocalChecked());
  }
}

static void SetterCallback(const v8::FunctionCallbackInfo<v8::Value> &info) {
  auto holder = info.Data().As<v8::Object>();
  auto context = info.GetIsolate()->GetCurrentContext();
  auto newValue = info[0];
  UNUSED(holder->Set(context, 1, newValue));

  auto key = holder->Get(context, 0).ToLocalChecked();
  auto observer = holder->Get(context, 2).ToLocalChecked().As<v8::External>();

  auto callback = reinterpret_cast<JniPropertyObserver *>(observer->Value());
  callback->onPropertyChanged(key, newValue);
}

static void ObserverWeakCallback(const v8::WeakCallbackInfo<JniPropertyObserver> &data) {
  LOGW("ObserverWeakCallback");
  JniPropertyObserver *callback = data.GetParameter();
  delete callback;
}

void JSObject::Watch(JNIEnv *env, jobject jThis, jobjectArray jKeys, jobject jObserver) {
  SETUP(env, jThis, v8::Object)
  auto length = env->GetArrayLength(jKeys);

  LOGI("watch %d properties", length);
  jclass clazz = env->GetObjectClass(jObserver);
  jmethodID methodId = env->GetMethodID(clazz, "onPropertyChanged", "(Ljava/lang/String;Ljava/lang/Object;)V");

  for (int i = 0; i < length; ++i) {
    LOGI("bind key[%d]", i);
    jstring element = (jstring) env->GetObjectArrayElement(jKeys, i);
    const uint16_t *key = env->GetStringChars(element, nullptr);
    const jint keyLen = env->GetStringLength(element);
    auto v8Key = V8_STRING(isolate, key, keyLen);

    /**
     * Save necessary information on this object
     * [0] -> key
     * [1] -> value
     * [2] -> observer
     */
    auto holder = v8::Object::New(isolate);

    UNUSED(holder->Set(context, 0, v8Key));

    auto getter = v8::FunctionTemplate::New(isolate, GetterCallback, holder)->GetFunction(context).ToLocalChecked();
    auto setter = v8::FunctionTemplate::New(isolate, SetterCallback, holder)->GetFunction(context).ToLocalChecked();

    v8::PropertyDescriptor descriptor(getter, setter);
    descriptor.set_enumerable(true);
    descriptor.set_configurable(false);

    // Remove the entry if exists, and move the value into holder
    if (that->Has(context, v8Key).ToChecked()) {
      auto value = that->Get(context, v8Key).ToLocalChecked();
      UNUSED(holder->Set(context, 1, value));
      UNUSED(that->Delete(context, v8Key));
    }

    auto observer = new JniPropertyObserver(env, jObserver, clazz, methodId);
    auto v8Observer = v8::External::New(isolate, observer);
    auto weakRef = new v8::Persistent<v8::Value>(runtime->isolate_, v8Observer);
    weakRef->SetWeak(observer, ObserverWeakCallback, v8::WeakCallbackType::kParameter);

    UNUSED(holder->Set(context, 2, v8Observer));

    UNUSED(that->DefineProperty(context, v8Key, descriptor));

    env->ReleaseStringChars(element, key);
    LOGI("bind key[%d] end", i);
  }
  if (env->ExceptionCheck()) {
    LOGW("Exception");
  }
  LOGI("watch end");
}

jint JSObject::OnLoad(JNIEnv *env) {
  jclass clazz = env->FindClass("com/linroid/knode/js/JSObject");
  if (!clazz) {
    return JNI_ERR;
  }

  JNINativeMethod methods[] = {
    {"nativeGet",    "(Ljava/lang/String;)Lcom/linroid/knode/js/JSValue;",                    (void *) (JSObject::Get)},
    {"nativeSet",    "(Ljava/lang/String;Lcom/linroid/knode/js/JSValue;)V",                   (void *) (JSObject::Set)},
    {"nativeNew",    "()V",                                                                   (void *) (JSObject::New)},
    {"nativeHas",    "(Ljava/lang/String;)Z",                                                 (void *) (JSObject::Has)},
    {"nativeDelete", "(Ljava/lang/String;)V",                                                 (void *) (JSObject::Delete)},
    {"nativeKeys",   "()[Ljava/lang/String;",                                                 (void *) (JSObject::Keys)},
    {"nativeWatch",  "([Ljava/lang/String;Lcom/linroid/knode/observable/PropertyObserver;)V", (void *) (JSObject::Watch)},
  };
  jClazz = (jclass) env->NewGlobalRef(clazz);
  jConstructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/knode/js/JSContext;J)V");
  env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
  return JNI_OK;
}
