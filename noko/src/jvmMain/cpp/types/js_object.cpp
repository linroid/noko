#include <jni.h>
#include <node.h>
#include "../observable/properties_observer.h"
#include "../util/env_helper.h"
#include "../util/jni_helper.h"
#include "js_object.h"
#include "js_value.h"
#include "js_undefined.h"
#include "boolean.h"
#include "integer.h"
#include "string.h" // NOLINT(modernize-deprecated-headers)
#include "double.h"
#include "long.h"
namespace JsObject {

jclass class_;
jclass string_class_;
jmethodID init_method_id_;

jobject Of(JNIEnv *env, jobject node, jlong pointer) {
  return env->NewObject(class_, init_method_id_, node, pointer);
}

JNICALL void Set(JNIEnv *env, jobject j_this, jstring j_key, jobject j_value) {
  const jchar *key_chars = env->GetStringChars(j_key, nullptr);
  const jint key_len = env->GetStringLength(j_key);

  SETUP(env, j_this, v8::Object);

  auto key = V8_STRING(isolate, key_chars, key_len);
  env->ReleaseStringChars(j_key, key_chars);
  auto value = JsValue::Value(env, j_value);
  that->Set(context, key, value).Check();
}

JNICALL jobject Get(JNIEnv *env, jobject j_this, jstring j_key) {
  const uint16_t *key = env->GetStringChars(j_key, nullptr);
  const jint key_len = env->GetStringLength(j_key);
  SETUP(env, j_this, v8::Object);
  auto value = that->Get(context, V8_STRING(isolate, key, key_len)).ToLocalChecked();
  env->ReleaseStringChars(j_key, key);
  return JsValue::Of(env, value);
}

jboolean Has(JNIEnv *env, jobject j_this, jstring j_key) {
  const uint16_t *key = env->GetStringChars(j_key, nullptr);
  const jint key_len = env->GetStringLength(j_key);
  SETUP(env, j_this, v8::Object);
  bool result = that->Has(context, V8_STRING(isolate, key, key_len)).ToChecked();
  env->ReleaseStringChars(j_key, key);
  return static_cast<jboolean>(result);
}

jobjectArray Keys(JNIEnv *env, jobject j_this) {
  SETUP(env, j_this, v8::Object);
  v8::TryCatch try_catch(isolate);
  auto names = that->GetPropertyNames(context);
  if (names.IsEmpty()) {
    runtime->Throw(env, try_catch.Exception());
    return nullptr;
  }
  auto array = names.ToLocalChecked();
  auto length = array->Length();
  auto result = env->NewObjectArray((jsize) length, string_class_, nullptr);
  for (int i = 0; i < length; i++) {
    auto element = array->Get(context, i);
    if (element.IsEmpty()) {
      runtime->Throw(env, try_catch.Exception());
      return nullptr;
    }
    v8::String::Value unicode_string(isolate, element.ToLocalChecked());
    uint16_t *unicode_chars = *unicode_string;
    env->SetObjectArrayElement(result, i, env->NewString(unicode_chars, unicode_string.length()));
  }
  return result;
}

jlong New(JNIEnv *env, jclass clazz) {
  UNUSED(clazz);
  V8_SCOPE(env);
  auto value = v8::Object::New(isolate);
  return (jlong) new v8::Persistent<v8::Value>(isolate, value);
}

void Delete(JNIEnv *env, jobject j_this, jstring j_key) {
  const uint16_t *key = env->GetStringChars(j_key, nullptr);
  const jint key_len = env->GetStringLength(j_key);
  SETUP(env, j_this, v8::Object);
  that->Delete(context, V8_STRING(isolate, key, key_len)).Check();
  env->ReleaseStringChars(j_key, key);
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
  auto new_value = info[0];
  holder->Set(context, 1, new_value).Check();

  auto key = holder->Get(context, 0).ToLocalChecked().As<v8::String>();

  auto v8_observer = holder->Get(context, 2).ToLocalChecked().As<v8::External>();

  v8::String::Value unicode_string(isolate, key);

  auto observer = reinterpret_cast<PropertiesObserver *>(v8_observer->Value());
  EnvHelper env(observer->runtime_->Jvm());
  jstring j_key = env->NewString(*unicode_string, unicode_string.length());
  jobject j_value = JsValue::Of(*env, new_value);
  observer->onPropertyChanged(*env, j_key, j_value);
}

static void ObserverWeakCallback(const v8::WeakCallbackInfo<PropertiesObserver> &data) {
  PropertiesObserver *callback = data.GetParameter();
  LOGW("ObserverWeakCallback: callback=%p", callback);
  delete callback;
}

void Watch(JNIEnv *env, jobject j_this, jobjectArray j_keys, jobject j_observer) {
  SETUP(env, j_this, v8::Object);
  auto length = env->GetArrayLength(j_keys);

  jmethodID method_id = env->GetMethodID(env->GetObjectClass(j_observer), "onPropertyChanged",
                                         "(Ljava/lang/String;Ljava/lang/Object;)V");

  for (int i = 0; i < length; ++i) {
    auto j_key = (jstring) env->GetObjectArrayElement(j_keys, i);
    const uint16_t *key = env->GetStringChars(j_key, nullptr);
    const jint key_len = env->GetStringLength(j_key);
    auto v8_key = V8_STRING(isolate, key, key_len);
    env->ReleaseStringChars(j_key, key);

    /**
     * Create a project and replace the property value,
     * hold necessary information in this object
     * [0] -> key, the property key
     * [1] -> value, the value of key
     * [2] -> observer, an observer to notify once the value change
     */
    auto holder = v8::Object::New(isolate);

    holder->Set(context, 0, v8_key).Check();

    auto getter = v8::FunctionTemplate::New(isolate, GetterCallback, holder)->GetFunction(context).ToLocalChecked();
    auto setter = v8::FunctionTemplate::New(isolate, SetterCallback, holder)->GetFunction(context).ToLocalChecked();

    v8::PropertyDescriptor descriptor(getter, setter);
    descriptor.set_enumerable(true);
    descriptor.set_configurable(false);

    // Remove the entry if exists, and move the value into holder
    if (that->Has(context, v8_key).ToChecked()) {
      auto value = that->Get(context, v8_key).ToLocalChecked();
      holder->Set(context, 1, value).Check();
      that->Delete(context, v8_key).Check();
    } else {
      holder->Set(context, 1, v8::Undefined(isolate)).Check();
    }

    auto observer = new PropertiesObserver(runtime, env, j_observer, method_id);
    auto v8_observer = v8::External::New(isolate, observer);
    auto weak_reference = new v8::Persistent<v8::Value>(isolate, v8_observer);
    weak_reference->SetWeak(observer, ObserverWeakCallback, v8::WeakCallbackType::kParameter);

    holder->Set(context, 2, v8_observer).Check();

    // Now replace the origin value as our getter and setter
    that->DefineProperty(context, v8_key, descriptor).Check();
  }
}

jboolean DefineProperty(
    JNIEnv *env,
    jobject j_this,
    jstring j_key,
    jboolean writeable,
    jboolean enumerable,
    jboolean configurable,
    jobject value,
    jobject getter,
    jobject setter
) {
  const uint16_t *key = env->GetStringChars(j_key, nullptr);
  const jint key_len = env->GetStringLength(j_key);
  SETUP(env, j_this, v8::Object);
  auto v8_key = V8_STRING(isolate, key, key_len);
  env->ReleaseStringChars(j_key, key);

  v8::TryCatch try_catch(isolate);
  if (getter != nullptr || setter != nullptr) {
    v8::PropertyDescriptor descriptor(JsValue::Value(env, getter), JsValue::Value(env, setter));
    descriptor.set_enumerable(enumerable);
    descriptor.set_configurable(configurable);
    auto result = that->DefineProperty(context, v8_key, descriptor);
    if (try_catch.HasCaught()) {
      runtime->Throw(env, try_catch.Exception());
      return false;
    }
    return result.ToChecked();
  } else {
    v8::PropertyDescriptor descriptor(JsValue::Value(env, value), writeable);
    descriptor.set_enumerable(enumerable);
    descriptor.set_configurable(configurable);
    auto result = that->DefineProperty(context, v8_key, descriptor);
    if (try_catch.HasCaught()) {
      runtime->Throw(env, try_catch.Exception());
      return false;
    }
    return result.ToChecked();
  }
}

jint OnLoad(JNIEnv *env) {
  jclass clazz = env->FindClass("com/linroid/noko/types/JsObject");
  if (!clazz) {
    return JNI_ERR;
  }

  JNINativeMethod methods[] = {
      {"nativeGet", "(Ljava/lang/String;)Ljava/lang/Object;", (void *) (Get)},
      {"nativeSet", "(Ljava/lang/String;Ljava/lang/Object;)V", (void *) (Set)},
      {"nativeInit", "()J", (void *) (New)},
      {"nativeHas", "(Ljava/lang/String;)Z", (void *) (Has)},
      {"nativeDelete", "(Ljava/lang/String;)V", (void *) (Delete)},
      {"nativeKeys", "()[Ljava/lang/String;", (void *) (Keys)},
      {"nativeWatch", "([Ljava/lang/String;Lcom/linroid/noko/observable/PropertiesObserver;)V", (void *) (Watch)},
      {"nativeDefineProperty", "(Ljava/lang/String;ZZZLjava/lang/Object;Lcom/linroid/noko/types/JsFunction;Lcom/linroid/noko/types/JsFunction;)Z",
       (void *) (DefineProperty)},
  };
  class_ = (jclass) env->NewGlobalRef(clazz);
  init_method_id_ = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/noko/Node;J)V");

  jclass string_class = env->FindClass("java/lang/String");
  string_class_ = (jclass) env->NewGlobalRef(string_class);
  env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
  return JNI_OK;
}

}