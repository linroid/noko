-keep class com.linroid.knode.js.BindJS
#-keepclasseswithmembers class com.linroid.dora.** {
#    @ocom.linroid.knode.js.BindJS *;
#}
#-keepnames class com.linroid.dora.** {
#    @com.linroid.knode.js.BindJS *;
#}
-keepclassmembers class ** {
    @com.linroid.knode.js.BindJS *;
}
-keep class com.linroid.knode.** { *; }
-keepnames class com.linroid.knode.** { *; }
