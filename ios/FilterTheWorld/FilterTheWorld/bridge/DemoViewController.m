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

#import "DemoViewController.h"

#include <MoltenVK/mvk_vulkan.h>
#include "../../vulkan_application/cube2.h"

#pragma mark -
#pragma mark DemoViewController

@implementation DemoViewController {
	CADisplayLink* displayLink;
	struct demo demo;
}

-(void) dealloc {
	demo_cleanup(&demo);
	[displayLink release];
	[super dealloc];
}

-(instancetype)initWithView:(UIView *) view {
    self = [super init];

    if (self) {
      self.view = view;
    }
    return self;
}

/** Since this is a single-view app, init Vulkan when the view is loaded. */
-(void) viewWillLayoutSubviews {
	[super viewWillLayoutSubviews];

  self.view.contentScaleFactor = UIScreen.mainScreen.nativeScale;

#if TARGET_OS_SIMULATOR
	// Avoid linear host-coherent texture loading on simulator
	const char* argv[] = { "cube", "--use_staging" };
#else
	const char* argv[] = { "cube" };
#endif
	int argc = sizeof(argv)/sizeof(char*);
	demo_main(&demo, self.view.layer, argc, argv);
	demo_draw(&demo, 0);

	displayLink = [CADisplayLink displayLinkWithTarget: self selector: @selector(renderLoop)];
  [displayLink addToRunLoop: NSRunLoop.currentRunLoop forMode: NSDefaultRunLoopMode];
}

-(void) renderLoop {
  double elapsedTimeS = displayLink.targetTimestamp - displayLink.timestamp;
	demo_draw(&demo, elapsedTimeS);
}

// Allow device rotation to resize the swapchain
-(void)viewWillTransitionToSize:(CGSize)size withTransitionCoordinator:(id)coordinator {
	[super viewWillTransitionToSize:size withTransitionCoordinator:coordinator];
	demo_resize(&demo);
}

@end

#pragma mark -
#pragma mark DemoView

@implementation DemoView

/** Returns a Metal-compatible layer. */
+(Class) layerClass { return [CAMetalLayer class]; }

@end

