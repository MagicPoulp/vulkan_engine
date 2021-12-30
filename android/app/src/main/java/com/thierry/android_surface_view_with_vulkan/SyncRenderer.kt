package com.thierry.android_surface_view_with_vulkan

import android.view.Choreographer

// https://stackoverflow.com/questions/44036646/how-to-sync-to-android-frame-rate
class SyncedRenderer(val doFrame: (frameTimeNanos: Long) -> Unit) {

    private var callback: (Long) -> Unit = {}
    private var frameTimeNanosPrevious = 0L

    fun start() {
        callback = {
            if (frameTimeNanosPrevious == -1L) {
                frameTimeNanosPrevious = it
            }
            doFrame(it - frameTimeNanosPrevious)
            frameTimeNanosPrevious = it
            Choreographer.getInstance().postFrameCallback(callback)
        }
        Choreographer.getInstance().postFrameCallback(callback)
    }

    fun stop() {
        Choreographer.getInstance().removeFrameCallback(callback)
        callback = {}
    }
}
