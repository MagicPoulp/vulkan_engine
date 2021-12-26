
import SwiftUI

struct ContentView: View {
  @StateObject private var model = ContentViewModel()

  var body: some View {
    ZStack {
      Color.black.edgesIgnoringSafeArea(.all)
      
      ErrorView(error: model.error)

      FrameView(frame: model.frame).edgesIgnoringSafeArea(.all)

      VulkanView().edgesIgnoringSafeArea(.all)
    }
  }
}

struct ContentView_Previews: PreviewProvider {
  static var previews: some View {
    ContentView()
  }
}
