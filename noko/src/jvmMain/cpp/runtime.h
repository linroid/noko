#ifndef NODE_RUNTIME_H
#define NODE_RUNTIME_H

#include <jni.h>
#include <string>
#include <vector>
#include <thread>
#include <node.h>
#include <uv.h>
#include "util/macros.h"
#define RUNTIME_V8_SCOPE()                  \
  CheckThread();                            \
  v8::Locker locker(isolate_);              \
  v8::HandleScope handle_scope(isolate_);   \
  auto context = context_.Get(isolate_);    \
  v8::Context::Scope context_scope(context) \

class Runtime {

 private:
  static jmethodID on_attach_method_id_;
  static jmethodID on_start_method_id_;
  static jmethodID on_detach_method_id_;

  static jfieldID shared_undefined_field_id_;
  static jfieldID pointer_field_id_;

  static std::mutex shared_mutex_;
  static jint instance_count_;
  static jint sequence_;

  int id_ = -1;
  bool keep_alive_ = false;
  bool strict_ = false;

  jobject j_global_ = nullptr;
  jobject shared_undefined_ = nullptr;
  JavaVM *vm_ = nullptr;
  jobject j_this_ = nullptr;

  bool running_ = false;
  std::thread::id thread_id_;
  std::mutex async_mutex_;
  std::vector<std::function<void()>> callbacks_;

  uv_loop_t *event_loop_ = nullptr;
  uv_async_t *keep_alive_handle_ = nullptr;
  uv_async_t *callback_handle_ = nullptr;
  node::MultiIsolatePlatform *platform_;

  v8::Isolate *isolate_ = nullptr;
  v8::Persistent<v8::Context> context_;
  v8::Persistent<v8::Object> process_;
  v8::Persistent<v8::Object> global_;
  v8::Persistent<v8::Function> require_;

  void Handle(uv_async_t *handle);

  bool KeepAlive();

  void TryLoop();

  void Attach();

  void OnStart();

  void Detach() const;

 public:

  Runtime(
      JNIEnv *env,
      jobject j_this,
      node::MultiIsolatePlatform *platform,
      bool keep_alive,
      bool strict
  );

  ~Runtime();

  v8::Isolate *Isolate() { return isolate_; }

  v8::Local<v8::Context> Context() {
    return context_.Get(isolate_);
  }

  jobject JThis() { return j_this_; }

  jobject JGlobal() { return j_global_; }

  JavaVM *Jvm() { return vm_; }

  int Start(std::vector<std::string> &args);

  void Exit(int code);

  bool Await(const std::function<void()> &runnable);

  bool Post(const std::function<void()> &runnable);

  void MountFile(const char *src, const char *dst, int mode);

  void Chroot(const char *path);

  void Throw(JNIEnv *env, v8::Local<v8::Value> exception);

  void CheckThread();

  bool IsRunning() const { return running_; }

  jobject Eval(const uint16_t *code, int codeLen, const uint16_t *source, int source_len, int line);

  jobject ParseJson(const uint16_t *json, int json_len);

  jobject ThrowError(const uint16_t *message, int message_len) const;

  jobject Require(const uint16_t *path, int path_len);

  static Runtime *Get(JNIEnv *env, jobject obj);

  static Runtime *Current();

  static jint OnLoad(JNIEnv *env);
};

#endif //NODE_RUNTIME_H
