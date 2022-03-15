package com.linroid.noko.example

import androidx.compose.material.MaterialTheme
import androidx.compose.ui.window.Window
import androidx.compose.ui.window.application
import com.linroid.noko.Node

fun main() = application {
    Node.setup(4)
    Window(onCloseRequest = ::exitApplication) {
        MaterialTheme {
            Terminal()
        }
    }
}