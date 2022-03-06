#include "jni_helper.h"
#include "log.h"

namespace JniHelper {

std::string GetClassName(JNIEnv *env, jobject obj) {
  LOGI("GetClassName");
  auto clazz = env->GetObjectClass(obj);
  jmethodID get_name_id = env->GetMethodID(clazz, "getName", "()Ljava/lang/String;");
  auto name = (jstring) env->CallObjectMethod(clazz, get_name_id);
  return GetString(env, name);
}

std::string GetString(JNIEnv *env, jstring str) {
  jclass string_class = env->GetObjectClass(str);
  jmethodID get_bytes_method_id = env->GetMethodID(string_class, "getBytes", "(Ljava/lang/String;)[B");
  auto bytes_array = (jbyteArray) env->CallObjectMethod(str, get_bytes_method_id, env->NewStringUTF("UTF-8"));
  auto length = (size_t) env->GetArrayLength(bytes_array);
  jbyte *bytes = env->GetByteArrayElements(bytes_array, nullptr);

  std::string result = std::string((char *) bytes, length);

  env->ReleaseByteArrayElements(bytes_array, bytes, JNI_ABORT);
  env->DeleteLocalRef(bytes_array);
  env->DeleteLocalRef(string_class);

  return result;
}

std::string GetExceptionMessage(JNIEnv *env, jthrowable exception) {
  LOGI("GetExceptionMessage");
  jclass clazz = env->GetObjectClass(exception);
  jmethodID get_message_id = env->GetMethodID(clazz, "getMessage", "()Ljava/lang/String;");
  auto j_message = (jstring) env->CallObjectMethod(exception, get_message_id);
  auto message = GetString(env, j_message);
  env->ReleaseStringUTFChars(j_message, nullptr);
  return message;
}

std::string GetStackTrace(JNIEnv *env, jthrowable exception) {
  jclass writer_class = env->FindClass("java/io/StringWriter");
  jmethodID string_writer_init_method_id = env->GetMethodID(writer_class, "<init>", "()V");
  jmethodID string_writer_to_string_method_id = env->GetMethodID(writer_class,
                                                                 "toString", "()Ljava/lang/String;");
  jclass print_writer_class = env->FindClass("java/io/PrintWriter");
  jmethodID writer_init_method_id = env->GetMethodID(print_writer_class, "<init>", "(Ljava/io/Writer;)V");
  jobject string_writer = env->NewObject(writer_class, string_writer_init_method_id);
  jobject print_writer = env->NewObject(print_writer_class,
                                        writer_init_method_id,
                                        string_writer);
  jclass exception_class = env->GetObjectClass(exception); // can't fail
  jmethodID print_stack_trace_method_id = env->GetMethodID(exception_class,
                                                           "printStackTrace",
                                                           "(Ljava/io/PrintWriter;)V"
  );
  env->DeleteLocalRef(exception_class);

  env->CallVoidMethod(exception, print_stack_trace_method_id, print_writer);
  auto j_message = (jstring) env->CallObjectMethod(
      string_writer, string_writer_to_string_method_id);
  auto message = GetString(env, j_message);

  env->DeleteLocalRef(j_message);
  env->DeleteLocalRef(print_writer);
  env->DeleteLocalRef(string_writer);
  env->DeleteLocalRef(print_writer_class);
  env->DeleteLocalRef(writer_class);
  return message;
}

}