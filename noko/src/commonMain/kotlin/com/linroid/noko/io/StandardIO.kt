package com.linroid.noko.io

import com.linroid.noko.types.JsFunction
import com.linroid.noko.types.JsObject
import com.linroid.noko.types.JsValue
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.channels.Channel
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.launch
import com.linroid.noko.Node
import com.linroid.noko.types.PropertyDescriptor
import kotlinx.coroutines.cancel
import kotlinx.coroutines.flow.consumeAsFlow
import okio.Closeable

class StandardIO(private val node: Node) : Closeable {

  private val outputChannel = Channel<String>()
  private val errorChannel = Channel<String>()
  private val scope = CoroutineScope(Dispatchers.IO)
  private val inputChannel = Channel<String>()

  internal fun bind(global: JsObject) {
    val process = global.get<JsObject>("process")!!
    val stdout: JsObject = process.get("stdout")!!
    stdout.set("write", object : JsFunction(node, "write") {
      override fun onCall(receiver: JsValue, parameters: Array<out Any?>): Any? {
        val content = parameters[0].toString()
        scope.launch {
          outputChannel.send(content)
        }
        return null
      }
    })
    val stderr: JsObject = process.get("stderr")!!
    stderr.set("write", object : JsFunction(node, "write") {
      override fun onCall(receiver: JsValue, parameters: Array<out Any?>): Any? {
        val content = parameters[0].toString()
        scope.launch {
          errorChannel.send(content)
        }
        return null
      }
    })
    val stream = node.require("stream") as JsFunction
    val stdin = stream.get<JsFunction>("Readable")?.call(stream)
    // node.eval(
    //   """
    //   const Stream = require("stream");
    //   const readable = new Stream.Readable();
    //   readable._read = () => {
    //   };
    //   readable
    // """.trimIndent()
    // )

    // if (output.supportsColor) {
    //   setupCode.append(
    //     """
    //     process.stderr.isTTY = true;
    //     process.stderr.isRaw = true;
    //     process.stdout.isTTY = true;
    //     process.stdout.isRaw = true;
    //     """.trimIndent()
    //   )
    // }
    // if (output.supportsColor) {
    //   setEnv("COLORTERM", "truecolor")
    // }
    check(stdin is JsObject)
    stdin.set("_read", JsFunction(node, "_read"))
    process.defineProperty("stdin", PropertyDescriptor(value = stdin))
    val push: JsFunction = stdin.get("push")!!
    scope.launch {
      inputChannel.consumeAsFlow().collect {
        push.call(stdin, it)
      }
    }
  }

  fun output(): Flow<String> {
    return outputChannel.consumeAsFlow()
  }

  fun error(): Flow<String> {
    return errorChannel.consumeAsFlow()
  }

  // fun write(bytes: ByteArray) {
  fun write(content: String) {
    scope.launch {
      inputChannel.send(content)
    }
  }

  override fun close() {
    scope.cancel()
  }
}