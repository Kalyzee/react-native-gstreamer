//
//  RCTGstPlayer.m
//  RCTGstPlayer
//
//  Created by Alann on 20/12/2017.
//  Copyright © 2017 Kalyzee. All rights reserved.
//

#import <Foundation/Foundation.h>

#import "RCTGstPlayer.h"

@implementation RCTGstPlayer

RCT_EXPORT_MODULE();

// Shared properties
RCT_CUSTOM_VIEW_PROPERTY(uri, NSString, RCTGstPlayerController)
{
    NSString *uri = [RCTConvert NSString:json];
    NSLog(@"RCTGstPlayer : URI : %s - LENGTH : %d", uri, uri.length);
    if (uri.length > 0)
        rct_gst_set_uri((gchar *)[uri UTF8String]);
}
RCT_CUSTOM_VIEW_PROPERTY(audioLevelRefreshRate, NSNumber, RCTGstPlayerController)
{
    gint* audioLevelRefreshRate = [[RCTConvert NSNumber:json] integerValue];
    rct_gst_set_audio_level_refresh_rate(audioLevelRefreshRate);
}
RCT_CUSTOM_VIEW_PROPERTY(isDebugging, BOOL, RCTGstPlayerController)
{
    gboolean isDebugging = [RCTConvert BOOL:json];
    rct_gst_set_debugging(isDebugging);
}

// Shared events
RCT_EXPORT_VIEW_PROPERTY(onPlayerInit, RCTBubblingEventBlock)
RCT_EXPORT_VIEW_PROPERTY(onStateChanged, RCTBubblingEventBlock)
RCT_EXPORT_VIEW_PROPERTY(onVolumeChanged, RCTBubblingEventBlock)
RCT_EXPORT_VIEW_PROPERTY(onUriChanged, RCTBubblingEventBlock)
RCT_EXPORT_VIEW_PROPERTY(onEOS, RCTBubblingEventBlock)
RCT_EXPORT_VIEW_PROPERTY(onElementError, RCTBubblingEventBlock)

// Methods
RCT_EXPORT_METHOD(setState:(nonnull NSNumber *)reactTag state:(nonnull NSNumber *)state) {
    NSNumber *_state = [RCTConvert NSNumber:state];
    rct_gst_set_pipeline_state([_state intValue]);
}

RCT_EXPORT_METHOD(recreateView:(nonnull NSNumber *)reactTag){
    [self->playerController recreateView];
}

// react-native init
- (UIView *)view
{
    // Init GStreamer
    gst_ios_init();
    
    // Init controller
    self->playerController = [[RCTGstPlayerController alloc] init];
    
    // Return view
    return [self->playerController view];
}

@end
