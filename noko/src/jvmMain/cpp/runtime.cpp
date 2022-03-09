#include <jni.h>
#include <mutex>
#include <uv.h>
#include <v8.h>
#include "runtime.h"
#include "types/js_value.h"
#include "types/js_undefined.h"
#include "types/boolean.h"
#include "types/double.h"
#include "types/js_object.h"
#include "types/string.h"  // NOLINT(modernize-deprecated-headers)
#include "types/js_function.h"
#include "types/js_promise.h"
#include "types/js_array.h"
#include "types/js_error.h"
#include "util/env_helper.h"
#include "types/integer.h"
#include "types/long.h"

int Runtime::instance_count_ = 0;
std::mutex Runtime::shared_mutex_;
int Runtime::sequence_ = 0;

thread_local Runtime *current_runtime_;

Runtime::Runtime(
    JNIEnv *env,
    jobject j_this,
    node::MultiIsolatePlatform *platform,
    bool keep_alive,
    bool strict
) : platform_(platform),
    keep_alive_(keep_alive),
    strict_(strict),
    j_this_(env->NewGlobalRef(j_this)) {

  env->GetJavaVM(&vm_);
  std::lock_guard<std::mutex> lock(shared_mutex_);
  ++instance_count_;
  ++sequence_;
  id_ = sequence_;
}

Runtime::~Runtime() {
  std::lock_guard<std::mutex> shared_lock(shared_mutex_);
  std::lock_guard<std::mutex> async_lock(async_mutex_);

  {
    EnvHelper env(vm_);
    env->DeleteGlobalRef(shared_undefined_);
    env->SetLongField(j_this_, pointer_field_id_, 0l);
    env->DeleteGlobalRef(j_this_);
    env->DeleteGlobalRef(j_global_);
  }

  isolate_ = nullptr;
  running_ = false;
  --instance_count_;
}

Runtime *Runtime::Current() {
  return current_runtime_;
}

int Runtime::Start(std::vector<std::string> &args) {
  thread_id_ = std::this_thread::get_id();

  int exit_code = 0;

  std::vector<std::string> errors;
  const std::vector<std::string> exec_args;

  auto flags = static_cast<node::EnvironmentFlags::Flags>(
      node::EnvironmentFlags::kOwnsEmbedded
          | node::EnvironmentFlags::kOwnsProcessState
          | node::EnvironmentFlags::kTrackUnmanagedFds);

  std::unique_ptr<node::CommonEnvironmentSetup> setup
      = node::CommonEnvironmentSetup::Create(platform_, &errors, args, exec_args, flags);

  isolate_ = setup->isolate();
  event_loop_ = setup->event_loop();
  auto env = setup->env();

  if (isolate_ == nullptr) {
    LOGE("Isolate was failed to create");
    return 2;
  }
  if (keep_alive_ && !KeepAlive()) {
    LOGE("Unable to setup keep alive handle");
    return 3;
  }

  v8::Locker locker(isolate_);
  v8::Isolate::Scope isolate_scope(isolate_);
  v8::HandleScope handle_scope(isolate_);
  auto context = setup->context();
  v8::Context::Scope context_scope(context);
  node::SetProcessExitHandler(env, [](node::Environment *environment, int code) {
    LOGW("Node.js exited with code %d", code);
    node::Stop(environment);
  });

  context_.Reset(isolate_, setup->context());
  global_.Reset(isolate_, context->Global());

  Attach();

  isolate_->SetMicrotasksPolicy(v8::MicrotasksPolicy::kAuto);
  auto load_env_result = node::LoadEnvironment(env, [&](const node::StartExecutionCallbackInfo &info) -> v8::MaybeLocal<v8::Value> {
    require_.Reset(isolate_, info.native_require);
    process_.Reset(isolate_, info.process_object);
    OnStart();
    return v8::Null(isolate_);
  });

  if (load_env_result.IsEmpty()) {
    LOGE("Failed to call node::LoadEnvironment");
    return 1;
  }
  exit_code = node::SpinEventLoop(env).FromMaybe(1);

  Detach();

  context_.Reset();
  global_.Reset();
  // node::Stop() can be used to explicitly stop the event loop and keep
  // further JavaScript from running. It can be called from any thread,
  // and will act like worker.terminate() if called from another thread.
  node::Stop(env);
  running_ = false;

  return exit_code;
}

bool Runtime::KeepAlive() {
  keep_alive_handle_ = new uv_async_t();
  auto error = uv_async_init(event_loop_, keep_alive_handle_, [](uv_async_t *handle) {
#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCDFAInspection"
    uv_close((uv_handle_t *) handle, [](uv_handle_t *h) {
      free(h);
    });
#pragma clang diagnostic pop
  });
  if (error != 0) {
    LOGE("Failed to initialize keep alive handle: %s", uv_err_name(error));
    return false;
  }
  return true;
}

void Runtime::Attach() {
  RUNTIME_V8_SCOPE();
  running_ = true;
  current_runtime_ = this;
  EnvHelper env(vm_);
  auto undefined_value = new v8::Persistent<v8::Value>(isolate_, v8::Undefined(isolate_));
  this->shared_undefined_ = env->NewGlobalRef(JsUndefined::Of(*env, j_this_, (jlong) undefined_value));
  this->j_global_ = env->NewGlobalRef(JsObject::Of(*env, j_this_, (jlong) &global_));

  env->SetObjectField(j_this_, shared_undefined_field_id_, shared_undefined_);
  env->CallVoidMethod(j_this_, on_attach_method_id_, j_global_);
}

void Runtime::OnStart() {
  RUNTIME_V8_SCOPE();
  EnvHelper env(vm_);
  env->CallVoidMethod(j_this_, on_start_method_id_, j_global_);
}

void Runtime::Detach() const {
  current_runtime_ = nullptr;
  EnvHelper env(vm_);
  env->CallVoidMethod(j_this_, on_detach_method_id_, j_this_);
}

void Runtime::Exit(int code) {
  if (code != 0) {
    LOGW("exit(%d)", code);
  }
  if (keep_alive_) {
    uv_async_send(keep_alive_handle_);
  }
  Post([&, code] {
    v8::HandleScope handle_scope(isolate_);
    auto process = process_.Get(isolate_);
    auto context = context_.Get(isolate_);
    v8::Local<v8::Value> v8Code = v8::Number::New(isolate_, code);
    auto exit_func = process->Get(context, V8_UTF_STRING(isolate_, "exit"));
    if (code != 0) {
      LOGW("Calling process.exit(%d)", code);
    }
    UNUSED(v8::Local<v8::Function>::Cast(exit_func.ToLocalChecked())
               ->Call(context, process, 1, &v8Code));
  });
}

bool Runtime::Await(const std::function<void()> &runnable) {
  assert(isolate_ != nullptr);
  if (std::this_thread::get_id() == thread_id_) {
    runnable();
    return true;
  } else if (strict_) {
    EnvHelper env(vm_);
    env->ThrowNew(env->FindClass("java/lang/IllegalStateException"), "Illegal thread access");
    return true;
  } else {
    std::condition_variable cv;
    bool signaled = false;
    auto callback = [&]() {
      runnable();
      {
        std::lock_guard<std::mutex> lock(async_mutex_);
        signaled = true;
      }
      cv.notify_one();
    };

    std::unique_lock<std::mutex> lock(async_mutex_);
    if (!running_) {
      LOGE("Instance has been destroyed, ignore await");
      return false;
    }
    callbacks_.emplace_back(callback);
    this->TryLoop();

    cv.wait(lock, [&] { return signaled; });
    lock.unlock();
    return true;
  }
}

bool Runtime::Post(const std::function<void()> &runnable) {
  if (!running_) {
    return false;
  }
  std::lock_guard<std::mutex> lock(async_mutex_);
  if (!running_) {
    return false;
  }
  if (std::this_thread::get_id() == thread_id_) {
    runnable();
    return true;
  } else {
    callbacks_.push_back(runnable);
    this->TryLoop();
    return true;
  }
}

void Runtime::TryLoop() {
  if (!event_loop_) {
    LOGE("TryLoop() but event_loop_ is null");
    return;
  }
  if (!callback_handle_) {
    callback_handle_ = new uv_async_t();
    callback_handle_->data = this;
    uv_async_init(event_loop_, callback_handle_, [](uv_async_t *handle) {
      auto *runtime = reinterpret_cast<Runtime *>(handle->data);
      runtime->Handle(handle);
    });
    uv_async_send(callback_handle_);
  }
}

void Runtime::Handle(uv_async_t *handle) {
  async_mutex_.lock();
  while (!callbacks_.empty()) {
    auto callback = callbacks_.front();
    async_mutex_.unlock();
    callback();
    async_mutex_.lock();
    callbacks_.erase(callbacks_.begin());
  }
#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCDFAInspection"
  uv_close((uv_handle_t *) handle, [](uv_handle_t *h) {
    delete (uv_async_t *) h;
  });
#pragma clang diagnostic pop
  callback_handle_ = nullptr;
  async_mutex_.unlock();
}

void Runtime::MountFile(const char *src, const char *dst, const int mode) {
  RUNTIME_V8_SCOPE();
  auto env = node::GetCurrentEnvironment(context);
  node::Mount(env, src, dst, mode);
}

void Runtime::Chroot(const char *path) {
  RUNTIME_V8_SCOPE();
  auto env = node::GetCurrentEnvironment(context);
  node::Chroot(env, path);
}

void Runtime::Throw(JNIEnv *env, v8::Local<v8::Value> exception) {
  RUNTIME_V8_SCOPE();
  auto reference = new v8::Persistent<v8::Value>(isolate_, exception);
  auto j_exception = JsError::ToException(env, JsError::Of(env, j_this_, (jlong) reference));
  env->Throw((jthrowable) j_exception);
}

void Runtime::CheckThread() {
  if (std::this_thread::get_id() != thread_id_) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat"
    LOGE("Only the original thread can access Node.js: current=%ld, threadId=%ld",
         std::this_thread::get_id(), thread_id_);
#pragma clang diagnostic pop
    std::abort();
  }
}

jobject Runtime::Eval(const uint16_t *code, int code_len, const uint16_t *source, int source_len, int line) {
  RUNTIME_V8_SCOPE();

  v8::TryCatch try_catch(isolate_);
  v8::ScriptOrigin script_origin(V8_STRING(isolate_, source, source_len),
                                 v8::Integer::New(isolate_, line),
                                 v8::Integer::New(isolate_, 0));
  auto script = v8::Script::Compile(context, V8_STRING(isolate_, code, code_len), &script_origin);
  EnvHelper env(vm_);
  if (script.IsEmpty()) {
    LOGE("Compile script with an exception");
    Throw(*env, try_catch.Exception());
    return nullptr;
  }
  auto result = script.ToLocalChecked()->Run(context);
  if (result.IsEmpty()) {
    LOGE("Run script with an exception");
    Throw(*env, try_catch.Exception());
    return nullptr;
  }
  return JsValue::Of(*env, result.ToLocalChecked());
}

jobject Runtime::ParseJson(const uint16_t *json, int json_len) {
  RUNTIME_V8_SCOPE();

  v8::TryCatch try_catch(isolate_);
  auto result = v8::JSON::Parse(context, V8_STRING(isolate_, json, json_len));
  EnvHelper env(vm_);
  if (result.IsEmpty()) {
    Throw(*env, try_catch.Exception());
    return nullptr;
  }
  return JsValue::Of(*env, result.ToLocalChecked());
}

jobject Runtime::ThrowError(const uint16_t *message, int message_len) const {
  auto error = v8::Exception::Error(V8_STRING(isolate_, message, message_len));
  isolate_->ThrowException(error);
  EnvHelper env(vm_);
  return JsValue::Of(*env, error);
}

jobject Runtime::Require(const uint16_t *path, int path_len) {
  auto context = context_.Get(isolate_);
  auto *argv = new v8::Local<v8::Value>[1];
  argv[0] = V8_STRING(isolate_, path, path_len);
  auto global = global_.Get(isolate_);
  v8::TryCatch try_catch(isolate_);

  auto maybe = global->Get(context, V8_UTF_STRING(isolate_, "require"))
      .ToLocalChecked()->ToObject(context);
  v8::Local<v8::Function> require;
  if (maybe.IsEmpty()) {
    require = require_.Get(isolate_);
  } else {
    require = maybe.ToLocalChecked().As<v8::Function>();
  }

  auto result = require->CallAsFunction(context, global, 1, argv);

  EnvHelper env(vm_);
  if (result.IsEmpty()) {
    Throw(*env, try_catch.Exception());
    return nullptr;
  }
  return JsValue::Of(*env, result.ToLocalChecked());
}

jmethodID Runtime::on_attach_method_id_;
jmethodID Runtime::on_start_method_id_;
jmethodID Runtime::on_detach_method_id_;
jfieldID Runtime::shared_undefined_field_id_;
jfieldID Runtime::pointer_field_id_;

jint Runtime::OnLoad(JNIEnv *env) {
  jclass clazz = env->FindClass("com/linroid/noko/Node");
  if (clazz == nullptr) {
    return JNI_ERR;
  }

  shared_undefined_field_id_ = env->GetFieldID(clazz, "sharedUndefined",
                                               "Lcom/linroid/noko/types/JsUndefined;");
  pointer_field_id_ = env->GetFieldID(clazz, "pointer", "J");

  on_attach_method_id_ = env->GetMethodID(clazz, "onAttach", "(Lcom/linroid/noko/types/JsObject;)V");
  on_start_method_id_ = env->GetMethodID(clazz, "onStart", "(Lcom/linroid/noko/types/JsObject;)V");
  on_detach_method_id_ = env->GetMethodID(clazz, "onDetach", "(Lcom/linroid/noko/types/JsObject;)V");

  return JNI_OK;
}

Runtime *Runtime::Get(JNIEnv *env, jobject obj) {
  jlong pointer = env->GetLongField(obj, pointer_field_id_);
  return reinterpret_cast<Runtime *>(pointer);
}
