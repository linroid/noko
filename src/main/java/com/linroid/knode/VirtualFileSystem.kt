package com.linroid.knode

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
  private val addWhiteList: JSFunction = thiz.get("addWhiteList")

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
    if (exists) {
      return
    }
    symlink.call(thiz, file.absolutePath, target)
  }

  fun mount(file: File, target: String, mask: Int) {
    mountFunc.call(thiz, file.absolutePath, target, mask)
  }

  fun addWhiteList(file: File) {
    addWhiteList.call(thiz, file.absolutePath)
  }
}
