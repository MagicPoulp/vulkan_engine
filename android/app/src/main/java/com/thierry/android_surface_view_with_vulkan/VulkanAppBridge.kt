package com.getwemap.vulkan_engine

import android.content.res.AssetManager
import android.view.Surface

class VulkanAppBridge {

    init {
        System.loadLibrary("jni_vulkan")
    }

    private external fun nativeCreate(surface: Surface, assetManager: AssetManager)
    private external fun nativeDestroy()
    private external fun nativeResize(width: Int, height: Int)
    private external fun nativeDraw(elapsedTimeS: Double)

    fun create(surface: Surface, assetManager: AssetManager) {
        nativeCreate(surface, assetManager)
    }

    fun destroy() {
        nativeDestroy()
    }

    fun resize(width: Int, height: Int) {
        nativeResize(width, height)
    }

    fun draw(elapsedTimeS: Double) {
        nativeDraw(elapsedTimeS)
    }
}
