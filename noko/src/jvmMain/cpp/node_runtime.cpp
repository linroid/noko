#include <jni.h>
#include <mutex>
#include <uv.h>
#include <v8.h>
#include "node_runtime.h"
#include "types/js_value.h"
#include "types/js_undefined.h"
#include "types/js_boolean.h"
#include "types/js_number.h"
#include "types/js_object.h"
#include "types/js_string.h"
#include "types/js_function.h"
#include "types/js_null.h"
#include "types/js_promise.h"
#include "types/js_array.h"
#include "types/js_error.h"
#include "util/env_helper.h"

int NodeRuntime::instance_count_ = 0;
std::mutex NodeRuntime::shared_mutex_;
int NodeRuntime::sequence_ = 0;

bool node_initialized = false;
std::unique_ptr<node::MultiIsolatePlatform> platform;

NodeRuntime::NodeRuntime(
    JNIEnv *env,
    jobject j_this,
    bool keep_alive,
    bool strict
) : keep_alive_(keep_alive),
    strict_(strict) {

  env->GetJavaVM(&vm_);
  j_this_ = env->NewGlobalRef(j_this);
  std::lock_guard<std::mutex> lock(shared_mutex_);
  ++instance_count_;
  ++sequence_;
  id_ = sequence_;
  if (!node_initialized) {
    init_node();
  }
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat"
  LOGD("new Node: thread_id=%d, id=%d, this=%p", std::this_thread::get_id(), id_, this);
#pragma clang diagnostic pop
}

NodeRuntime::~NodeRuntime() {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat"
  LOGE("Node(): thread_id=%d, id=%d, this=%p", std::this_thread::get_id(), id_, this);
#pragma clang diagnostic pop
  std::lock_guard<std::mutex> shared_lock(shared_mutex_);
  std::lock_guard<std::mutex> async_lock(async_mutex_);
  {
    EnvHelper env(vm_);
    env->DeleteGlobalRef(shared_undefined_);
    env->DeleteGlobalRef(shared_null_);
    env->DeleteGlobalRef(shared_true_);
    env->DeleteGlobalRef(shared_false_);
    env->SetLongField(j_this_, pointer_field_id_, 0l);
    env->DeleteGlobalRef(j_this_);
    env->DeleteGlobalRef(j_global_);
  }

  isolate_ = nullptr;
  running_ = false;
  LOGI("set running=false");

  --instance_count_;
  LOGE("Node() finished, instanceCount=%d", instance_count_);
}

void NodeRuntime::Attach() {
  RUNTIME_V8_SCOPE();
  running_ = true;
  EnvHelper env(vm_);
  auto null_value = new v8::Persistent<v8::Value>(isolate_, v8::Null(isolate_));
  auto undefined_value = new v8::Persistent<v8::Value>(isolate_, v8::Undefined(isolate_));
  auto true_value = new v8::Persistent<v8::Value>(isolate_, v8::True(isolate_));
  auto false_value = new v8::Persistent<v8::Value>(isolate_, v8::False(isolate_));
  this->shared_null_ = env->NewGlobalRef(JsNull::ToJava(*env, j_this_, (jlong) null_value));
  this->shared_undefined_ = env->NewGlobalRef(JsUndefined::ToJava(*env, j_this_, (jlong) undefined_value));
  this->shared_true_ = env->NewGlobalRef(JsBoolean::ToJava(*env, j_this_, (jlong) true_value, true));
  this->shared_false_ = env->NewGlobalRef(JsBoolean::ToJava(*env, j_this_, (jlong) false_value, false));
  this->j_global_ = env->NewGlobalRef(JsObject::ToJava(*env, j_this_, (jlong) &global_));

  env->SetObjectField(j_this_, shared_null_field_id_, shared_null_);
  env->SetObjectField(j_this_, shared_undefined_field_id_, shared_undefined_);
  env->SetObjectField(j_this_, shared_true_field_id_, shared_true_);
  env->SetObjectField(j_this_, shared_false_field_id_, shared_false_);

  env->CallVoidMethod(j_this_, attach_method_id_, j_global_);
}

void NodeRuntime::Detach() const {
  EnvHelper env(vm_);
  env->CallVoidMethod(j_this_, detach_method_id_, j_this_);
}

int NodeRuntime::Start(std::vector<std::string> &args) {
  thread_id_ = std::this_thread::get_id();

  int exit_code = 0;
  if (!InitLoop()) {
    LOGE("Failed to call InitLoop()");
    return -1;
  }

  std::shared_ptr<node::ArrayBufferAllocator> allocator = node::ArrayBufferAllocator::Create();
  isolate_ = node::NewIsolate(allocator, event_loop_, platform.get());

  if (isolate_ == nullptr) {
    LOGE("Failed to call node::NewIsolate()");
    return -2;
  }

  {
    v8::Locker locker(isolate_);
    v8::Isolate::Scope isolate_scope(isolate_);

    auto isolate_data = node::CreateIsolateData(isolate_, event_loop_, platform.get(),
                                                allocator.get());
    v8::HandleScope handle_scope(isolate_);
    v8::Local<v8::Context> context = node::NewContext(isolate_);
    if (context.IsEmpty()) {
      LOGE("Failed to call node::NewContext()");
      return -3;
    }

    v8::Context::Scope context_scope(context);
    auto flags = static_cast<node::EnvironmentFlags::Flags>(node::EnvironmentFlags::kOwnsEmbedded
        |
            node::EnvironmentFlags::kOwnsProcessState
        |
            node::EnvironmentFlags::kTrackUnmanagedFds);
    auto env = node::CreateEnvironment(isolate_data, context, args, args, flags);
    node::SetProcessExitHandler(env, [](node::Environment *environment, int code) {
      LOGW("Node.js process is exiting: code=%d", code);
      node::Stop(environment);
    });

    context_.Reset(isolate_, context);
    global_.Reset(isolate_, context->Global());

    Attach();

    isolate_->SetMicrotasksPolicy(v8::MicrotasksPolicy::kAuto);
    node::LoadEnvironment(env,
                          [&](const node::StartExecutionCallbackInfo &info) -> v8::MaybeLocal<v8::Value> {
                            require_.Reset(isolate_, info.native_require);
                            process_.Reset(isolate_, info.process_object);
                            return v8::Null(isolate_);
                          });

    RunLoop(env);
    Detach();
    // node::EmitExit() returns the current exit code.
    auto exit_code_maybe = node::EmitProcessExit(env);
    if (exit_code_maybe.IsJust()) {
      exit_code = exit_code_maybe.ToChecked();
    }
    // node::Stop() can be used to explicitly stop the event loop and keep
    // further JavaScript from running. It can be called from any thread,
    // and will act like worker.terminate() if called from another thread.
    node::Stop(env);

    node::FreeIsolateData(isolate_data);
    node::FreeEnvironment(env);

    context_.Reset();
    global_.Reset();
    running_ = false;
  }

  CloseLoop();
  return exit_code;
}

void NodeRuntime::CloseLoop() {
  LOGW("closing uv loop");
  // Unregister the Isolate with the platform and add a listener that is called
  // when the Platform is done cleaning up any state it had associated with
  // the Isolate.
  bool platform_finished = false;
  platform->AddIsolateFinishedCallback(isolate_, [](void *data) {
    *static_cast<bool *>(data) = true;
  }, &platform_finished);
  platform->UnregisterIsolate(isolate_);
  isolate_->Dispose();
  // Wait until the platform has cleaned up all relevant resources.
  while (!platform_finished) {
    uv_run(event_loop_, UV_RUN_ONCE);
  }
  int uvErrorCode = uv_loop_close(event_loop_);
  event_loop_ = nullptr;
  LOGI("close loop result: %d, callbacks.size=%lu", uvErrorCode, callbacks_.size());
  // uv_loop_delete(loop);
  // assert(err == 0);
}

void NodeRuntime::RunLoop(node::Environment *env) {
  v8::SealHandleScope seal(isolate_);

  int more;
  do {
    uv_run(event_loop_, UV_RUN_DEFAULT);

    // V8 tasks on background threads may end up scheduling new tasks in the
    // foreground, which in turn can keep the event loop going. For example,
    // WebAssembly.compile() may do so.
    platform->DrainTasks(isolate_);

    // If there are new tasks, continue.
    more = uv_loop_alive(event_loop_);
    if (more) continue;
    LOGW("no more task, try to exit");
    // node::EmitBeforeExit() is used to emit the 'beforeExit' event on
    // the `process` object.
    node::EmitProcessBeforeExit(env);

    // 'beforeExit' can also schedule new work that keeps the event loop
    // running.
    more = uv_loop_alive(event_loop_);
  } while (more);
}

void NodeRuntime::Exit(int code) {
  LOGI("Exit(code=%d)", code);
  if (keep_alive_) {
    uv_async_send(keep_alive_handle_);
  }
  Post([&] {
    v8::HandleScope handle_scope(isolate_);
    auto process = process_.Get(isolate_);
    auto context = context_.Get(isolate_);
    v8::Local<v8::Value> v8Code = v8::Number::New(isolate_, code);
    auto exit_func = process->Get(context, V8_UTF_STRING(isolate_, "exit"));
    LOGI("Calling process.exit(%d)", code);
    UNUSED(v8::Local<v8::Function>::Cast(exit_func.ToLocalChecked())
               ->Call(context, process, 1, &v8Code));
  });
}

bool NodeRuntime::InitLoop() {
  event_loop_ = uv_loop_new();
  int ret = uv_loop_init(event_loop_);
  if (ret != 0) {
    LOGE("Failed to initialize loop: %s", uv_err_name(ret));
    return false;
  }
  if (keep_alive_) {
    keep_alive_handle_ = new uv_async_t();
    ret = uv_async_init(event_loop_, keep_alive_handle_, [](uv_async_t *handle) {
      LOGD("Stop keep alive");
#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCDFAInspection"
      uv_close((uv_handle_t *) handle, [](uv_handle_t *h) {
        free(h);
      });
#pragma clang diagnostic pop
    });
    if (ret != 0) {
      LOGE("Failed to initialize keep alive handle: %s", uv_err_name(ret));
      return false;
    }
  }
  return true;
}

jobject NodeRuntime::ToJava(JNIEnv *env, v8::Local<v8::Value> value) const {
  if (value->IsNull()) {
    return this->shared_null_;
  } else if (value->IsUndefined()) {
    return this->shared_undefined_;
  } else if (value->IsBoolean()) {
    v8::Local<v8::Boolean> target = value->ToBoolean(isolate_);
    if (target->Value()) {
      return this->shared_true_;
    } else {
      return this->shared_false_;
    }
  } else {
    auto pointer = new v8::Persistent<v8::Value>(isolate_, value);
    if (value->IsNumber()) {
      return JsNumber::ToJava(env, j_this_, (jlong) pointer, value.As<v8::Number>()->Value());
    } else if (value->IsObject()) {
      if (value->IsFunction()) {
        return JsFunction::ToJava(env, j_this_, (jlong) pointer);
      } else if (value->IsPromise()) {
        return JsPromise::ToJava(env, j_this_, (jlong) pointer);
      } else if (value->IsNativeError()) {
        return JsError::ToJava(env, j_this_, (jlong) pointer);
      } else if (value->IsArray()) {
        return JsArray::ToJava(env, j_this_, (jlong) pointer);
      }
      return JsObject::ToJava(env, j_this_, (jlong) pointer);
    } else if (value->IsString()) {
      v8::String::Value unicodeString(isolate_, value);
      jstring jValue = env->NewString(*unicodeString, unicodeString.length());
      return JsString::ToJava(env, j_this_, (jlong) pointer, jValue);
    }
    return JsValue::ToJava(env, j_this_, (jlong) pointer);
  }
}

void NodeRuntime::TryLoop() {
  if (!event_loop_) {
    LOGE("TryLoop but eventLoop is null");
    return;
  }
  if (!callback_handle_) {
    callback_handle_ = new uv_async_t();
    callback_handle_->data = this;
    uv_async_init(event_loop_, callback_handle_, [](uv_async_t *handle) {
      auto *runtime = reinterpret_cast<NodeRuntime *>(handle->data);
      runtime->Handle(handle);
    });
    uv_async_send(callback_handle_);
  }
}

bool NodeRuntime::Await(const std::function<void()> &runnable) {
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

bool NodeRuntime::Post(const std::function<void()> &runnable) {
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

void NodeRuntime::Handle(uv_async_t *handle) {
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

void NodeRuntime::MountFile(const char *src, const char *dst, const int mode) {
  RUNTIME_V8_SCOPE();
  auto env = node::GetCurrentEnvironment(context);
  node::Mount(env, src, dst, mode);
}

void NodeRuntime::Chroot(const char *path) {
  RUNTIME_V8_SCOPE();
  auto env = node::GetCurrentEnvironment(context);
  node::Chroot(env, path);
}

void NodeRuntime::Throw(JNIEnv *env, v8::Local<v8::Value> exception) {
  RUNTIME_V8_SCOPE();
  auto reference = new v8::Persistent<v8::Value>(isolate_, exception);
  auto j_exception = JsError::ToException(env, JsError::ToJava(env, j_this_, (jlong) reference));
  env->Throw((jthrowable) j_exception);
}

void NodeRuntime::CheckThread() {
  if (std::this_thread::get_id() != thread_id_) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat"
    LOGE("js object can only be accessed from the Node.js thread: current=%ld, threadId=%ld",
         std::this_thread::get_id(), thread_id_);
#pragma clang diagnostic pop
    std::abort();
  }
}

jobject NodeRuntime::Eval(const uint16_t *code, int code_len, const uint16_t *source, int source_len, int line) {
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
  return ToJava(*env, result.ToLocalChecked());
}

jobject NodeRuntime::ParseJson(const uint16_t *json, int json_len) {
  RUNTIME_V8_SCOPE();

  v8::TryCatch try_catch(isolate_);
  auto result = v8::JSON::Parse(context, V8_STRING(isolate_, json, json_len));
  EnvHelper env(vm_);
  if (result.IsEmpty()) {
    Throw(*env, try_catch.Exception());
    return nullptr;
  }
  return ToJava(*env, result.ToLocalChecked());
}

jobject NodeRuntime::ThrowError(const uint16_t *message, int message_len) const {
  auto error = v8::Exception::Error(V8_STRING(isolate_, message, message_len));
  isolate_->ThrowException(error);
  EnvHelper env(vm_);
  return ToJava(*env, error);
}

jobject NodeRuntime::Require(const uint16_t *path, int path_len) {
  auto context = context_.Get(isolate_);
  auto *argv = new v8::Local<v8::Value>[1];
  argv[0] = V8_STRING(isolate_, path, path_len);
  auto global = global_.Get(isolate_);
  v8::TryCatch try_catch(isolate_);

  auto require = global->Get(context, V8_UTF_STRING(isolate_, "require"))
      .ToLocalChecked()->ToObject(context)
      .ToLocalChecked();
  assert(require->IsFunction());

  auto result = require->CallAsFunction(context, global, 1, argv);

  EnvHelper env(vm_);
  if (result.IsEmpty()) {
    Throw(*env, try_catch.Exception());
    return nullptr;
  }
  return ToJava(*env, result.ToLocalChecked());
}

int init_node() {
  // Make argv memory adjacent
  char cmd[128];
  strcpy(cmd, "node --trace-exit --trace-sigint --trace-sync-io --trace-warnings --title=Dora.js");
  int argc = 0;
  char *argv[128];
  char *p2 = strtok(cmd, " ");
  while (p2 && argc < 128 - 1) {
    argv[argc++] = p2;
    p2 = strtok(nullptr, " ");
  }
  argv[argc] = nullptr;

  std::vector<std::string> args = std::vector<std::string>(argv, argv + argc);
  std::vector<std::string> exec_args;
  std::vector<std::string> errors;
  // Parse Node.js CLI options, and print any errors that have occurred while
  // trying to parse them.
  int exit_code = node::InitializeNodeWithArgs(&args, &exec_args, &errors);
  for (const std::string &error : errors)
    LOGE("%s: %s\n", args[0].c_str(), error.c_str());
  if (exit_code == 0) {
    node_initialized = true;
  }

  // Create a v8::Platform instance. `MultiIsolatePlatform::Create()` is a way
  // to create a v8::Platform instance that Node.js can use when creating
  // Worker threads. When no `MultiIsolatePlatform` instance is present,
  // Worker threads are disabled.
  platform = node::MultiIsolatePlatform::Create(4);
  v8::V8::InitializePlatform(platform.get());
  v8::V8::Initialize();

  return exit_code;
}

// void shutdownNode() {
//     node_initialized = false;
//     v8::V8::Dispose();
//     v8::V8::ShutdownPlatform();
// }

jmethodID NodeRuntime::attach_method_id_;
jmethodID NodeRuntime::detach_method_id_;
jfieldID NodeRuntime::shared_null_field_id_;
jfieldID NodeRuntime::shared_undefined_field_id_;
jfieldID NodeRuntime::shared_true_field_id_;
jfieldID NodeRuntime::shared_false_field_id_;
jfieldID NodeRuntime::pointer_field_id_;

jint NodeRuntime::OnLoad(JNIEnv *env) {
  jclass clazz = env->FindClass("com/linroid/noko/Node");
  if (clazz == nullptr) {
    return JNI_ERR;
  }
  shared_null_field_id_ = env->GetFieldID(clazz, "sharedNull", "Lcom/linroid/noko/types/JsNull;");
  shared_undefined_field_id_ = env->GetFieldID(clazz, "sharedUndefined",
                                               "Lcom/linroid/noko/types/JsUndefined;");
  shared_true_field_id_ = env->GetFieldID(clazz, "sharedTrue", "Lcom/linroid/noko/types/JsBoolean;");
  shared_false_field_id_ = env->GetFieldID(clazz, "sharedFalse", "Lcom/linroid/noko/types/JsBoolean;");
  pointer_field_id_ = env->GetFieldID(clazz, "pointer", "J");

  attach_method_id_ = env->GetMethodID(clazz, "attach", "(Lcom/linroid/noko/types/JsObject;)V");
  detach_method_id_ = env->GetMethodID(clazz, "detach", "(Lcom/linroid/noko/types/JsObject;)V");
  return JNI_OK;
}
