import UIKit
import SwiftUI
import AVFoundation

class LayerUIViewController: UIViewController {
  
  var layer: CALayer?
  var angle: Double = 0
  
  init(layer: CALayer) {
    self.layer = layer
    super.init(nibName: nil, bundle: nil)
  }
  
  required init?(coder: NSCoder) {
    fatalError("init(coder:) has not been implemented")
  }
  
  override func viewDidLoad() {
    super.viewDidLoad()
    view.contentMode = .redraw
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
  }

  // to manage the CALayer rotation
  // viewDidLayoutSubviews changes the bounds whilst viewWillTransition performs a rotation
  // https://stackoverflow.com/questions/1282302/autorotation-of-calayer-on-iphone-rotation
  override func viewDidLayoutSubviews() {
    layer?.frame = view.bounds
  }

  // we rotate the CALayer to match the screen rotation, fully tested
  override func viewWillTransition(to size: CGSize, with coordinator: UIViewControllerTransitionCoordinator) {
      super.viewWillTransition(to: size, with: coordinator)
      switch UIDevice.current.orientation {
      case .landscapeLeft:
        let transform2 = -90.0 / 180.0 * .pi
        layer?.transform = CATransform3DMakeRotation(transform2, 0.0, 0.0, 1.0)
        angle = transform2

      case .landscapeRight:
        let transform2 = 90.0 / 180.0 * .pi
        layer?.transform = CATransform3DMakeRotation(transform2, 0.0, 0.0, 1.0)
        angle = transform2

      case .portrait:
        let transform2 = 0.0
        layer?.transform = CATransform3DMakeRotation(transform2, 0.0, 0.0, 1.0)
        angle = 0
        
      case .portraitUpsideDown:
        let transform2 = 180.0 / 180.0 * .pi
        layer?.transform = CATransform3DMakeRotation(transform2, 0.0, 0.0, 1.0)
        angle = transform2
        
      default:
        break
      }
  }
}

// SwiftUI does not use layers therefore we need a UIView
final class LayerView: UIViewControllerRepresentable {
  var layer: CALayer?
  
  init(layer: CALayer) {
    self.layer = layer
  }

  func makeUIViewController(context: Context) -> LayerUIViewController {
    return LayerUIViewController(layer: layer!)
  }
  
  func updateUIViewController(_ uiViewController: LayerUIViewController, context: Context) {
  }

  typealias UIViewControllerType = LayerUIViewController

}
