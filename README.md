# noko
[WIP]Kotlin MPP binding to Node.js

### Example
```kotlin
val node = Node(keepAlive = true).also {
        it.start("-i")
      }
```
### Supported Platforms

|          | arm64  | x64  |
|   ---    | --- | ---|
|  Windows |  ✔️|  ✔️ |
|  Linux   |  ✔️ | ✔️  |
|  macOS   | ✔️  | ✔️  |
|  Android |  ✔️ |  ✔️ |
|  iOS     |  N/A | N/A  |
|  JS      |  TBD  | TBD  |
