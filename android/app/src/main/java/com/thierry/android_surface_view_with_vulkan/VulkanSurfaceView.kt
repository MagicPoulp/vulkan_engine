package com.getwemap.vulkan_engine

import android.content.Context
import android.util.AttributeSet
import android.view.SurfaceHolder
import android.view.SurfaceView

class VulkanSurfaceView: SurfaceView, SurfaceHolder.Callback2 {

    private var vulkanApp = VulkanAppBridge()
    var isSyncRendererCreated: Boolean = false
    var isSyncRendererStarted: Boolean = false
    val syncedRenderer: SyncedRenderer = SyncedRenderer { elapsedTimeNanos: Long ->
        if (!isSyncRendererStarted) {
            return@SyncedRenderer
        }
        vulkanApp.draw(elapsedTimeNanos.toDouble()/1000000000.0)
    }

    constructor(context: Context): super(context) {
    }

    constructor(context: Context, attrs: AttributeSet): super(context, attrs) {
    }

    constructor(context: Context, attrs: AttributeSet, defStyle: Int): super(context, attrs, defStyle) {
    }

    constructor(context: Context, attrs: AttributeSet, defStyle: Int, defStyleRes: Int): super(context, attrs, defStyle, defStyleRes) {
    }

    fun syncedRendererStart() {
        // The Chroeographer next frame was moved to the C code
        // we see a performance improvement, du to garbage collection and the bridge calls
        // the code is commented to show the difference
        //if (!isSyncRendererCreated) {
        //    return
        //}
        //if (!isSyncRendererStarted) {
        //    isSyncRendererStarted = true
        //    syncedRenderer.start()
        //}
    }

    fun syncedRendererStop() {
        if (!isSyncRendererCreated) {
            return
        }
        if (isSyncRendererStarted) {
            isSyncRendererStarted = false
            syncedRenderer.stop()
        }
    }

    override fun onDetachedFromWindow() {
        super.onDetachedFromWindow()
        syncedRendererStop()
    }

    override fun onAttachedToWindow() {
        super.onAttachedToWindow()
        syncedRendererStart()
    }

    init {
        alpha = 1F
        holder.addCallback(this)
    }

    // ...
    // Implementation code similar to one in GLSurfaceView is skipped.
    // See: https://android.googlesource.com/platform/frameworks/base/+/master/opengl/java/android/opengl/GLSurfaceView.java
    // ...

    override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {
        vulkanApp.resize(width, height)
    }

    override fun surfaceDestroyed(holder: SurfaceHolder) {
        isSyncRendererCreated = false
        vulkanApp.destroy()
    }

    override fun surfaceCreated(holder: SurfaceHolder) {
        holder.let { h ->
            vulkanApp.create(h.surface, resources.assets)
            isSyncRendererCreated = true
            syncedRendererStart();
        }
    }

    override fun surfaceRedrawNeeded(holder: SurfaceHolder) {
        vulkanApp.draw(0.0)
    }
}
