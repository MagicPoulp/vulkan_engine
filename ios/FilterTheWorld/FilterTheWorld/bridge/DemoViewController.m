/*
 * DemoViewController.m
 *
 * Copyright (c) 2015-2021 The Brenwill Workshop Ltd. (http://www.brenwill.com)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <MoltenVK/mvk_vulkan.h>

#import "DemoViewController.h"
// To make the build simple, it is enough to include here all the .c files
#include "../../vulkan_application/Program.c"
#include "../../vulkan_application/VulkanDSL.c"
#include "../../vulkan_application/assets_management/AssetsFetcher.c"

#pragma mark -
#pragma mark DemoViewController

@implementation DemoViewController {
	CADisplayLink* displayLink;
  struct Program* program;
  BOOL initDone;
}

-(void) dealloc {
  Program__destroy(program);
	[displayLink release];
	[super dealloc];
}

-(instancetype)initWithView:(UIView *) view {
    self = [super init];
    initDone = NO;
    if (self) {
      self.view = view;
    }
    return self;
}

/** Since this is a single-view app, init Vulkan when the view is loaded. */
-(void) viewWillLayoutSubviews {
	[super viewWillLayoutSubviews];
  if (initDone) {
    return;
  }
  initDone = YES;

  self.view.contentScaleFactor = UIScreen.mainScreen.nativeScale;

	// the debugger step-in cannot be used in demo-main, BUT it works in a function just after
  NSString * texture1 = [[NSBundle mainBundle] pathForResource:  @"home8" ofType: @"png"];
  // https://www.qi-u.com/?qa=924696/c-how-to-fopen-on-the-iphone
  NSArray *split = [texture1 componentsSeparatedByString:@"/"];
  NSMutableArray *split2 = [split mutableCopy];
  [split2 removeObjectAtIndex:[split count]-1];
  NSString *joined = [split componentsJoinedByString:@"/"];
  const char *texturesPath = [joined cStringUsingEncoding:1];
  program = Program__create();
  program->vulkanDSL->caMetalLayer = self.view.layer;
  vulkanDSL_main(program->vulkanDSL, texturesPath);

	displayLink = [CADisplayLink displayLinkWithTarget: self selector: @selector(renderLoop)];
  [displayLink addToRunLoop: NSRunLoop.currentRunLoop forMode: NSDefaultRunLoopMode];
}

-(void) renderLoop {
  double elapsedTimeS = displayLink.targetTimestamp - displayLink.timestamp;
	demo_draw(program->vulkanDSL, elapsedTimeS);
}

// Allow device rotation to resize the swapchain
-(void)viewWillTransitionToSize:(CGSize)size withTransitionCoordinator:(id)coordinator {
	[super viewWillTransitionToSize:size withTransitionCoordinator:coordinator];
	demo_resize(program->vulkanDSL);
}

@end

#pragma mark -
#pragma mark DemoView

@implementation DemoView

/** Returns a Metal-compatible layer. */
+(Class) layerClass { return [CAMetalLayer class]; }

@end

