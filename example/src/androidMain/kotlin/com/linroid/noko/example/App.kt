package com.linroid.noko.example

import android.app.Application
import com.linroid.noko.Node

class App : Application() {
  override fun onCreate() {
    super.onCreate()
    Node.setup(4, this)
  }
}