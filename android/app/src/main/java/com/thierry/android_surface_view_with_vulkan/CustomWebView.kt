package com.getwemap.vulkan_engine

import android.annotation.SuppressLint
import android.content.Context
import android.graphics.Color
import android.net.Uri
import android.util.AttributeSet
import android.webkit.*
import androidx.annotation.RequiresApi
import androidx.webkit.WebViewAssetLoader

import android.webkit.WebView

import android.webkit.WebViewClient

class CustomWebView(context: Context, attrs: AttributeSet) : WebView(context, attrs) {

    val assetLoader: WebViewAssetLoader = WebViewAssetLoader.Builder()
        .addPathHandler("/assets/", WebViewAssetLoader.AssetsPathHandler(context))
        .addPathHandler("/", WebViewAssetLoader.AssetsPathHandler(context))
        .build()
    var showError: (message: String) -> Unit = {}

    init {
        setSettings()
        //val callback: (String) -> Unit = { mespage ->
        //    showError(message)
        //}
        //addJavascriptInterface(
        //    WebViewJavascriptInterface(getContext(), callback), "interface"
        //)
        loadUrl("https://appassets.androidplatform.net/assets/www/index.html")
    }

    @SuppressLint("ObsoleteSdkInt", "SetJavaScriptEnabled")
    private fun setSettings() {
        setBackgroundColor(Color.TRANSPARENT);
        //setLayerType(WebView.LAYER_TYPE_SOFTWARE, null);

        webViewClient = object : WebViewClient() {
            @RequiresApi(21)
            override fun shouldInterceptRequest(
                view: WebView,
                request: WebResourceRequest
            ): WebResourceResponse? {
                val url = request.url
                val last = url.toString().split("/").last()
                val url2 = Uri.parse("https://appassets.androidplatform.net/assets/www/" + last)
                return assetLoader.shouldInterceptRequest(url2)
            }
        }
        val webViewSettings = settings
        // Setting this off for security. Off by default for SDK versions >= 16.
        if (android.os.Build.VERSION.SDK_INT < 16) {
            @Suppress("DEPRECATION")
            webViewSettings.allowFileAccessFromFileURLs = false
        }
        // Off by default, deprecated for SDK versions >= 30.
        if (android.os.Build.VERSION.SDK_INT < 31) {
            @Suppress("DEPRECATION")
            webViewSettings.allowUniversalAccessFromFileURLs = false
        }
        // Keeping these off is less critical but still a good idea, especially if your app is not
        // using file:// or content:// URLs.
        webViewSettings.allowFileAccess = false
        webViewSettings.allowContentAccess = false

        // resource cache
        //clearCache(true)
        clearHistory()
        webViewSettings.setJavaScriptEnabled(true)
        webViewSettings.setJavaScriptCanOpenWindowsAutomatically(false)
        webViewSettings.blockNetworkLoads = false
        webViewSettings.blockNetworkImage = false

        // Assets are hosted under http(s)://appassets.androidplatform.net/assets/... .
        // If the application's assets are in the "main/assets" folder this will read the file
        // from "main/assets/www2/index.html?emmid=17555" and load it as if it were hosted on:
        // https://appassets.androidplatform.net/assets/www/index.html
    }
}

// https://letsencrypt.org/docs/certificates-for-localhost/

