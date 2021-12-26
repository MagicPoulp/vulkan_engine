
import SwiftUI

struct FrameView: View {
  var frame: CALayer?

  var body: some View {
    if let x = frame {
      LayerView(layer: x)
    } else {
      EmptyView()
    }
  }
}

struct CameraView_Previews: PreviewProvider {
  static var previews: some View {
    EmptyView()
  }
}
