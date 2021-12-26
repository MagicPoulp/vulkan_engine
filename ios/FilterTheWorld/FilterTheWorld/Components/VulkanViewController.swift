
import UIKit
import SwiftUI

struct VulkanViewController: UIViewControllerRepresentable {

    func makeUIViewController(context: Context) -> DemoViewController {
      let screenSize: CGRect = UIScreen.main.bounds
      let frame = CGRect(x: 0, y: 0, width: screenSize.width, height: screenSize.height)
      let view = DemoView(frame: frame)
      view.backgroundColor = UIColor.white.withAlphaComponent(0)
      let vc = DemoViewController(view: view)
      return vc!
    }

    func updateUIViewController(_ uiViewController: DemoViewController, context: Context) {
        
    }
}
