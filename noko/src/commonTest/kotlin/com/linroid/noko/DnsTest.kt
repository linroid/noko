package com.linroid.noko

import kotlinx.serialization.json.Json
import kotlinx.serialization.json.JsonArray
import kotlin.test.Test
import kotlin.test.assertIs
import com.linroid.noko.types.JsValue

class DnsTest : SetupNode() {

  @Test
  fun getServers() = runNodeTest {
    val result = eval("new (require('dns').promises.Resolver)().getServers()")
    assertIs<JsValue>(result)
    val json = result.toJson()!!
    val servers = Json.parseToJsonElement(json)
    assertIs<JsonArray>(servers)
  }
}
