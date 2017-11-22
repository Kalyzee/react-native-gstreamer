//
//  RNTMapManager.m
//  GStreamerIOS
//
//  Created by Alann Sapone on 21/07/2017.
//  Copyright © 2017 Facebook. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <React/RCTBridgeModule.h>
#import <React/RCTLog.h>

#import "GstPlayerManager.h"

@implementation GstPlayerManager

RCT_EXPORT_MODULE();
RCT_CUSTOM_VIEW_PROPERTY(uri, NSString, GstPlayerController)
{
    [self->rctGstPlayer setUri:[RCTConvert NSString:json]];
}

RCT_CUSTOM_VIEW_PROPERTY(play, BOOL, GstPlayerController)
{
    [self->rctGstPlayer setPlay:[RCTConvert BOOL:json]];
}

RCT_EXPORT_VIEW_PROPERTY(onAudioLevelChange, RCTBubblingEventBlock)


@synthesize bridge = _bridge;

- (UIView *)view
{
    NSBundle *_bundle = [NSBundle bundleWithURL:[[NSBundle mainBundle] URLForResource:@"Resources" withExtension:@"bundle"]];
    self->rctGstPlayer = [[UIStoryboard storyboardWithName:@"Storyboard" bundle:_bundle] instantiateViewControllerWithIdentifier:@"GstPlayerController"];
    return rctGstPlayer.view;
}
@end
