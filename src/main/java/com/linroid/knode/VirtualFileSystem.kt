package com.linroid.knode

import android.util.Log
import com.linroid.knode.js.JSFunction
import com.linroid.knode.js.JSObject
import java.io.File

/**
 * @author linroid
 * @since 2019-09-28
 */
class VirtualFileSystem(override val thiz: JSObject) : JSRef {

  private val mountFunc: JSFunction = thiz.get("mount")
  private val symlink: JSFunction = thiz.get("symlink")

  var cwd: String
    get() = thiz.get("cwd")
    set(value) {
      thiz.set("cwd", value)
    }

  // var require: JSFunction
  //   get() = thiz.get("require")
  //   set(value) {
  //     thiz.set("require", value)
  //   }


  fun symlink(file: File, target: String) {
    val exists = File(target).exists()
    Log.d(TAG, "symlink: ${file.absolutePath} -> $target, exists=$exists")
    if (exists) {
      return
    }
    symlink.call(thiz, file.absolutePath, target)
  }

  fun mount(file: File, target: String, mask: Int) {
    Log.d(TAG, "mount: ${file.absolutePath} -> $target, mask=$mask")
    mountFunc.call(thiz, file.absolutePath, target, mask)
  }

  companion object {
    const val TAG = "VirtualFileSystem"
    const val ACCESS_NONE = 0
    const val ACCESS_READ = 1
    const val ACCESS_WRITE = 2
  }
}
