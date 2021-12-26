
import AVFoundation

class FrameManager: NSObject, ObservableObject {
  static let shared = FrameManager()

  @Published var current: CVPixelBuffer?

  let videoOutputQueue = DispatchQueue(
    label: "com.raywenderlich.VideoOutputQ",
    qos: .userInitiated,
    attributes: [],
    autoreleaseFrequency: .workItem)

  private override init() {
    super.init()

    CameraManager.shared.set(self, queue: videoOutputQueue)
  }
}

extension FrameManager: AVCaptureVideoDataOutputSampleBufferDelegate {
}

/*
 // here we can process the stream of images, using an extension to make the image
 // the image extension in the Extensions folder was removed
extension FrameManager: AVCaptureVideoDataOutputSampleBufferDelegate {
  func captureOutput(
    _ output: AVCaptureOutput,
    didOutput sampleBuffer: CMSampleBuffer,
    from connection: AVCaptureConnection
  ) {
    if let buffer = sampleBuffer.imageBuffer {
      DispatchQueue.main.async {
        //self.current = buffer
      }
    }
  }
}
*/
