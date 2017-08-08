//
//  EaglUIView.m
//  GStreamerIOS
//
//  Created by Alann Sapone on 25/07/2017.
//  Copyright © 2017 Facebook. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "EaglUIView.h"

#import <QuartzCore/QuartzCore.h>

@implementation EaglUIView


+ (Class) layerClass
{
  return [CAEAGLLayer class];
}

@end
