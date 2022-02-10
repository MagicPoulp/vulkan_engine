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
#import <QuartzCore/CAMetalLayer.h>

#include <MoltenVK/mvk_vulkan.h>
#include "../../vulkan_application/Program.h"


#pragma mark -
#pragma mark DemoViewController

@implementation DemoViewController {
	CVDisplayLinkRef	_displayLink;
  struct Program* program;
}

-(void) dealloc {
  Program__destroy(program);
	CVDisplayLinkRelease(_displayLink);
	[super dealloc];
}

// Since this is a single-view app, init Vulkan when the view is loaded.
-(void) viewDidLoad {
	[super viewDidLoad];

	self.view.wantsLayer = YES;		// Back the view with a layer created by the makeBackingLayer method.

  // the debugger step-in cannot be used in demo-main, BUT it works in a function just after
  NSString * texture1 = [[NSBundle mainBundle] pathForResource:  @"home8" ofType: @"png"];
  // https://www.qi-u.com/?qa=924696/c-how-to-fopen-on-the-iphone
  NSArray *split = [texture1 componentsSeparatedByString:@"/"];
  NSMutableArray *split2 = [split mutableCopy];
  [split2 removeObjectAtIndex:[split count]-1];
  NSString *joined = [split2 componentsJoinedByString:@"/"];
  const char *assetsPath = [joined cStringUsingEncoding:1];
  program = Program__create(assetsPath);
  program->vulkanDSL->caMetalLayer = self.view.layer;
#if TARGET_OS_SIMULATOR
  program->vulkanDSL->iosSim = true;
#endif
  program->vulkanDSL->validate = true;
  vulkanDSL_main(program->vulkanDSL);

	CVDisplayLinkCreateWithActiveCGDisplays(&_displayLink);
	CVDisplayLinkSetOutputCallback(_displayLink, &DisplayLinkCallback, program);
	CVDisplayLinkStart(_displayLink);
}


#pragma mark Display loop callback function

/** Rendering loop callback function for use with a CVDisplayLink. */
static CVReturn DisplayLinkCallback(CVDisplayLinkRef displayLink,
									const CVTimeStamp* now,
									const CVTimeStamp* outputTime,
									CVOptionFlags flagsIn,
									CVOptionFlags* flagsOut,
									void* target) {
  double fps = (outputTime->rateScalar * (double)(outputTime->videoTimeScale) / (double)(outputTime->videoRefreshPeriod));
  double elapsedTimeS = 1/fps;
  struct Program *program = (struct Program *)target;
  //demo_draw(program->vulkanDSL, elapsedTimeS);
	return kCVReturnSuccess;
}

@end


#pragma mark -
#pragma mark DemoView

@implementation DemoView

/** Indicates that the view wants to draw using the backing layer instead of using drawRect:.  */
-(BOOL) wantsUpdateLayer { return YES; }

/** Returns a Metal-compatible layer. */
+(Class) layerClass { return [CAMetalLayer class]; }

/** If the wantsLayer property is set to YES, this method will be invoked to return a layer instance. */
-(CALayer*) makeBackingLayer {
	CALayer* layer = [self.class.layerClass layer];
	CGSize viewScale = [self convertSizeToBacking: CGSizeMake(1.0, 1.0)];
	layer.contentsScale = MIN(viewScale.width, viewScale.height);
	return layer;
}

@end
