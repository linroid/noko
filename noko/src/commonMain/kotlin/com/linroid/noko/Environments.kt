package com.linroid.noko

class Environments() {
  private val values = HashMap<String, String>()

  private constructor(values: Map<String, String>) : this() {
    this.values.putAll(values)
  }

  fun set(key: String, value: String) {
    values[key] = value
  }

  fun remove(key: String) {
    values.remove(key)
  }

  fun spawn(): Environments {
    return Environments(values)
  }
}