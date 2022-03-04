package com.linroid.noko

import kotlinx.serialization.json.Json
import kotlinx.serialization.json.JsonArray
import kotlin.test.Test
import kotlin.test.assertIs

class DnsTest : WithNode() {

  @Test
  fun getServers(): Unit = blockingInNode {
    val result = node.eval("new (require('dns').promises.Resolver)().getServers()")
    val json = result.toJson()!!
    val servers = Json.parseToJsonElement(json)
    assertIs<JsonArray>(servers)
  }
}
