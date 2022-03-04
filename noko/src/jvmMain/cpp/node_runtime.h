#ifndef NODE_NOKO_H
#define NODE_NOKO_H

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

int init_node();

enum JSType {
  /** Cached types */
  kNone,
  kUndefined,
  kNull,
  kBoolean,

  /** Non cached types */
  kValue,
  kArray,
  kObject,
  kString,
  kNumber,
  kFunction,
  kPromise,
  kError,
};

class NodeRuntime {

 private:
  static jmethodID attach_method_id_;
  static jmethodID detach_method_id_;

  static jfieldID shared_null_field_id_;
  static jfieldID shared_undefined_field_id_;
  static jfieldID shared_true_field_id_;
  static jfieldID shared_false_field_id_;
  static jfieldID pointer_field_id_;

  static std::mutex shared_mutex_;
  static jint instance_count_;
  static jint sequence_;

  int id_ = -1;
  bool keep_alive_ = false;
  bool strict_ = false;

  jobject j_this_ = nullptr;
  jobject j_global_ = nullptr;
  jobject shared_null_ = nullptr;
  jobject shared_undefined_ = nullptr;
  jobject shared_true_ = nullptr;
  jobject shared_false_ = nullptr;

  v8::Persistent<v8::Object> global_;
  v8::Persistent<v8::Function> require_;

  bool running_ = false;
  std::thread::id thread_id_;
  std::mutex async_mutex_;
  std::vector<std::function<void()>> callbacks_;

  uv_loop_t *event_loop_ = nullptr;
  uv_async_t *keep_alive_handle_ = nullptr;
  uv_async_t *callback_handle_ = nullptr;

  v8::Persistent<v8::Object> process_;

  void Handle(uv_async_t *handle);

  bool InitLoop();

  void TryLoop();

  void RunLoop(node::Environment *env);

  void CloseLoop();

  void Attach();

  void Detach() const;

 public:
  JavaVM *vm_ = nullptr;

  v8::Isolate *isolate_ = nullptr;
  v8::Persistent<v8::Context> context_;

  NodeRuntime(JNIEnv *env, jobject j_this, bool keep_alive, bool strict);

  ~NodeRuntime();

  int Start(std::vector<std::string> &args);

  void Exit(int code);

  bool Await(const std::function<void()> &runnable);

  bool Post(const std::function<void()> &runnable);

  jobject ToJava(JNIEnv *env, v8::Local<v8::Value> value) const;

  v8::Local<v8::Value> Require(const char *path);

  void MountFile(const char *src, const char *dst, int mode);

  void Chroot(const char *path);

  void Throw(JNIEnv *env, v8::Local<v8::Value> exception);

  void CheckThread();

  bool IsRunning() const {
    return running_;
  }

  jobject Eval(const uint16_t *code, int codeLen, const uint16_t *source, int source_len, int line);

  jobject ParseJson(const uint16_t *json, int json_len);

  jobject ThrowError(const uint16_t *message, int message_len) const;

  jobject Require(const uint16_t *path, int path_len);

  static jint OnLoad(JNIEnv *env);

  static NodeRuntime *From(JNIEnv *env, jobject j_this) {
    jlong pointer = env->GetLongField(j_this, pointer_field_id_);
    return reinterpret_cast<NodeRuntime *>(pointer);
  }
};

#endif //NODE_NOKO_H
