package com.thierry.android_surface_view_with_vulkan

import android.view.Choreographer

// https://stackoverflow.com/questions/44036646/how-to-sync-to-android-frame-rate
class SyncedRenderer(val doFrame: (frameTimeNanos: Long) -> Unit) {

    private var callback: (Long) -> Unit = {}

    fun start() {
        callback = {
            doFrame(it)
            Choreographer.getInstance().postFrameCallback(callback)
        }
        Choreographer.getInstance().postFrameCallback(callback)
    }

    fun stop() {
        Choreographer.getInstance().removeFrameCallback(callback)
        callback = {}
    }
}
