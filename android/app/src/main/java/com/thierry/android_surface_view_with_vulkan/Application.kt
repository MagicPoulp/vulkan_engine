package com.getwemap.vulkan_engine

import android.app.Application
import androidx.camera.camera2.Camera2Config
import androidx.camera.core.CameraXConfig

public class MyCameraXApplication : Application(),  CameraXConfig.Provider {
    override fun getCameraXConfig(): CameraXConfig {
        return Camera2Config.defaultConfig()
    }
}
