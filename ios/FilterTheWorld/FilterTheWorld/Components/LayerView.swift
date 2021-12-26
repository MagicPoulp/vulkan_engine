import UIKit
import SwiftUI
import AVFoundation

// SwiftUI does not use layers therefore we need a UIView
struct LayerView: UIViewRepresentable {
  var layer: CALayer?
  
  func makeUIView(context: Context) -> UIView {
    let view = UIView()
    if let x = layer {
      x.frame = UIScreen.main.bounds
      if let x2 = x as? AVCaptureVideoPreviewLayer {
        // the image is cropped to fill all the screen
        // it seems that the original captured size saves space at the
        // top of the screen for the navigation bar
        x2.videoGravity = .resizeAspectFill
      }
      view.layer.addSublayer(x)
    }
    return view
  }

  func updateUIView(_ uiView: UIView, context: Context) {
  }
}
