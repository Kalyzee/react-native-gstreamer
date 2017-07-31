//
//  RNTMapManager.m
//  GStreamerIOS
//
//  Created by Alann Sapone on 21/07/2017.
//  Copyright © 2017 Facebook. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <React/RCTViewManager.h>
#import <React/RCTBridgeModule.h>
#import <React/RCTLog.h>

#import "RCTGSTPlayerManager.h"
#import "ViewWrapper.h"

@implementation RCTGSTPlayerManager

RCT_EXPORT_MODULE();
RCT_CUSTOM_VIEW_PROPERTY(uri, NSString, RCTGSTPlayerController)
{
  [self->rctGstPlayer setUri:json];
}

@synthesize bridge = _bridge;

- (UIView *)view
{
  self->rctGstPlayer = [[UIStoryboard storyboardWithName:@"Storyboard" bundle:nil] instantiateViewControllerWithIdentifier:@"RCTGSTPlayerController"];

  return rctGstPlayer.view;
}
@end
