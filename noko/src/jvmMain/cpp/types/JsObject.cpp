#include <jni.h>
#include <node.h>
#include "JsObject.h"
#include "JsValue.h"
#include "JsString.h"
#include "JsUndefined.h"
#include "JsNull.h"
#include "../observable/PropertiesObserver.h"
#include "../EnvHelper.h"

jclass JsObject::jClazz;
jmethodID JsObject::jConstructor;

JNICALL void JsObject::Set(JNIEnv *env, jobject jThis, jstring jKey, jobject jValue) {
  const jchar *key = env->GetStringChars(jKey, nullptr);
  const jint keyLen = env->GetStringLength(jKey);

  auto value = JsValue::Unwrap(env, jValue);
  SETUP(env, jThis, v8::Object)
  // CHECK_NOT_NULL(*that);
  UNUSED(that->Set(context, V8_STRING(isolate, key, keyLen), value->Get(isolate)));
  env->ReleaseStringChars(jKey, key);
}

JNICALL jobject JsObject::Get(JNIEnv *env, jobject jThis, jstring jKey) {
  const uint16_t *key = env->GetStringChars(jKey, nullptr);
  const jint keyLen = env->GetStringLength(jKey);
  SETUP(env, jThis, v8::Object)
  auto value = that->Get(context, V8_STRING(isolate, key, keyLen)).ToLocalChecked();
  env->ReleaseStringChars(jKey, key);
  return node->ToJava(env, value);
}

jboolean JsObject::Has(JNIEnv *env, jobject jThis, jstring jKey) {
  const uint16_t *key = env->GetStringChars(jKey, nullptr);
  const jint keyLen = env->GetStringLength(jKey);
  SETUP(env, jThis, v8::Object)
  bool result = that->Has(context, V8_STRING(isolate, key, keyLen)).ToChecked();
  env->ReleaseStringChars(jKey, key);
  return static_cast<jboolean>(result);
}

jobjectArray JsObject::Keys(JNIEnv *env, jobject jThis) {
  SETUP(env, jThis, v8::Object)
  v8::TryCatch tryCatch(isolate);
  auto names = that->GetPropertyNames(context);
  if (names.IsEmpty()) {
    node->Throw(env, tryCatch.Exception());
    return nullptr;
  }
  auto array = names.ToLocalChecked();
  auto length = array->Length();
  auto result = env->NewObjectArray(length, env->FindClass("java/lang/String"), nullptr);
  for (int i = 0; i < length; i++) {
    auto element = array->Get(context, i);
    if (element.IsEmpty()) {
      node->Throw(env, tryCatch.Exception());
      return nullptr;
    }
    v8::String::Value unicodeString(isolate, element.ToLocalChecked());
    uint16_t *unicodeChars = *unicodeString;
    env->SetObjectArrayElement(result, i, env->NewString(unicodeChars, unicodeString.length()));
  }
  return result;
}

void JsObject::New(JNIEnv *env, jobject jThis) {
  V8_SCOPE(env, jThis)
  auto value = v8::Object::New(node->isolate_);
  auto result = new v8::Persistent<v8::Value>(node->isolate_, value);
  JsValue::SetPointer(env, jThis, (jlong) result);
}

void JsObject::Delete(JNIEnv *env, jobject jThis, jstring jKey) {
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
    info.GetReturnValue().Set(holder->Get(context, 1).ToLocalChecked());
  }
}

static void SetterCallback(const v8::FunctionCallbackInfo<v8::Value> &info) {
  auto holder = info.Data().As<v8::Object>();
  auto isolate = info.GetIsolate();
  auto context = isolate->GetCurrentContext();
  auto newValue = info[0];
  UNUSED(holder->Set(context, 1, newValue));

  auto key = holder->Get(context, 0).ToLocalChecked().As<v8::String>();

  auto v8Observer = holder->Get(context, 2).ToLocalChecked().As<v8::External>();

  v8::String::Value unicodeString(isolate, key);

  auto observer = reinterpret_cast<PropertiesObserver *>(v8Observer->Value());
  EnvHelper env(observer->node->vm_);
  jstring jKey = env->NewString(*unicodeString, unicodeString.length());
  jobject jValue = observer->node->ToJava(*env, newValue);
  observer->onPropertyChanged(*env, jKey, jValue);
}

static void ObserverWeakCallback(const v8::WeakCallbackInfo<PropertiesObserver> &data) {
  PropertiesObserver *callback = data.GetParameter();
  LOGW("ObserverWeakCallback: callback=%p", callback);
  delete callback;
}

void JsObject::Watch(JNIEnv *env, jobject jThis, jobjectArray jKeys, jobject jObserver) {
  SETUP(env, jThis, v8::Object)
  auto length = env->GetArrayLength(jKeys);

  jmethodID methodId = env->GetMethodID(env->GetObjectClass(jObserver), "onPropertyChanged",
                                        "(Ljava/lang/String;Lcom/linroid/noko/types/JsValue;)V");

  for (int i = 0; i < length; ++i) {
    auto jKey = (jstring) env->GetObjectArrayElement(jKeys, i);
    const uint16_t *key = env->GetStringChars(jKey, nullptr);
    const jint keyLen = env->GetStringLength(jKey);
    auto v8Key = V8_STRING(isolate, key, keyLen);
    env->ReleaseStringChars(jKey, key);

    /**
     * Create a project and replace the property value,
     * hold necessary information in this object
     * [0] -> key, the property key
     * [1] -> value, the value of key
     * [2] -> observer, an observer to notify once the value change
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
    } else {
      UNUSED(holder->Set(context, 1, v8::Undefined(isolate)));
    }

    auto observer = new PropertiesObserver(node, env, jObserver, methodId);
    auto v8Observer = v8::External::New(isolate, observer);
    auto weakRef = new v8::Persistent<v8::Value>(isolate, v8Observer);
    weakRef->SetWeak(observer, ObserverWeakCallback, v8::WeakCallbackType::kParameter);

    UNUSED(holder->Set(context, 2, v8Observer));

    // Now replace the origin value as our getter and setter
    UNUSED(that->DefineProperty(context, v8Key, descriptor));
  }
}

jint JsObject::OnLoad(JNIEnv *env) {
  jclass clazz = env->FindClass("com/linroid/noko/types/JsObject");
  if (!clazz) {
    return JNI_ERR;
  }

  JNINativeMethod methods[] = {
      {"nativeGet",    "(Ljava/lang/String;)Lcom/linroid/noko/types/JsValue;",                   (void *) (JsObject::Get)},
      {"nativeSet",    "(Ljava/lang/String;Lcom/linroid/noko/types/JsValue;)V",                  (void *) (JsObject::Set)},
      {"nativeNew",    "()V",                                                                    (void *) (JsObject::New)},
      {"nativeHas",    "(Ljava/lang/String;)Z",                                                  (void *) (JsObject::Has)},
      {"nativeDelete", "(Ljava/lang/String;)V",                                                  (void *) (JsObject::Delete)},
      {"nativeKeys",   "()[Ljava/lang/String;",                                                  (void *) (JsObject::Keys)},
      {"nativeWatch",  "([Ljava/lang/String;Lcom/linroid/noko/observable/PropertiesObserver;)V", (void *) (JsObject::Watch)},
  };
  jClazz = (jclass) env->NewGlobalRef(clazz);
  jConstructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/noko/Node;J)V");
  env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
  return JNI_OK;
}
