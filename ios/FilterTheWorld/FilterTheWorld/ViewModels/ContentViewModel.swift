
import SwiftUI

class ContentViewModel: ObservableObject {
  @Published var error: Error?
  @Published var frame: CALayer?

  private let cameraManager = CameraManager.shared
  private let frameManager = FrameManager.shared

  init() {
    setupSubscriptions()
  }

  func setupSubscriptions() {
    cameraManager.$previewLayer
      .receive(on: RunLoop.main)
      .map { $0 }
      .assign(to: &$frame)

    // swiftlint:disable:next array_init
    cameraManager.$error
      .receive(on: RunLoop.main)
      .map { $0 }
      .assign(to: &$error)
  }
}
