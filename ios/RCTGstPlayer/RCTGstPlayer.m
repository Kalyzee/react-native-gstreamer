//
//  RCTGstPlayer.m
//  RCTGstPlayer
//
//  Created by Alann on 20/12/2017.
//  Copyright © 2017 Kalyzee. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <React/RCTUIManager.h>
#import "RCTGstPlayer.h"

@implementation RCTGstPlayer

- (instancetype)init
{
    self = [super init];
    if (self)
        gst_ios_init(); // Init GStreamer
    return self;
}

+ (BOOL) requiresMainQueueSetup
{
    return TRUE;
}

// react-native init
- (UIView *)view
{
    return [RCTGstPlayerView getView];
}

RCT_EXPORT_MODULE();

// Shared properties
RCT_CUSTOM_VIEW_PROPERTY(uri, NSString, RCTGstPlayerView)
{
    NSString* uri = [RCTConvert NSString:json];
    if (uri.length > 0) {
        rct_gst_set_uri([view getUserData], g_strdup([uri UTF8String]));
    }
}
RCT_CUSTOM_VIEW_PROPERTY(uiRefreshRate, NSNumber, RCTGstPlayerView)
{
    guint64 refreshRate = [[RCTConvert NSNumber:json] unsignedLongLongValue];
    rct_gst_set_ui_refresh_rate([view getUserData], refreshRate);
}
RCT_CUSTOM_VIEW_PROPERTY(volume, NSNumber, RCTGstPlayerView)
{
    rct_gst_set_volume([view getUserData], [[RCTConvert NSNumber:json] doubleValue]);
}
RCT_CUSTOM_VIEW_PROPERTY(shareInstance, BOOL, RCTGstPlayerView)
{
    [view setShareInstance:[RCTConvert BOOL:json]];
}

// Shared events
RCT_EXPORT_VIEW_PROPERTY(onPlayerInit, RCTBubblingEventBlock)
RCT_EXPORT_VIEW_PROPERTY(onStateChanged, RCTBubblingEventBlock)
RCT_EXPORT_VIEW_PROPERTY(onVolumeChanged, RCTBubblingEventBlock)
RCT_EXPORT_VIEW_PROPERTY(onUriChanged, RCTBubblingEventBlock)
RCT_EXPORT_VIEW_PROPERTY(onPlayingProgress, RCTBubblingEventBlock)
RCT_EXPORT_VIEW_PROPERTY(onBufferingProgress, RCTBubblingEventBlock)
RCT_EXPORT_VIEW_PROPERTY(onEOS, RCTBubblingEventBlock)
RCT_EXPORT_VIEW_PROPERTY(onElementError, RCTBubblingEventBlock)

// Methods
RCT_EXPORT_METHOD(setState:(nonnull NSNumber *)reactTag state:(nonnull NSNumber *)state) {
    NSNumber *_state = [RCTConvert NSNumber:state];
    gint gst_state = [_state intValue];
    
    [self.bridge.uiManager addUIBlock:^(RCTUIManager *uiManager, NSDictionary<NSNumber *,UIView *> *viewRegistry) {
        RCTGstPlayerView *view = (RCTGstPlayerView *)viewRegistry[reactTag];
        if ([view isKindOfClass:[RCTGstPlayerView class]]) {
            [view setPipelineState:gst_state];
        }
    }];
}

RCT_EXPORT_METHOD(seek:(nonnull NSNumber *)reactTag position:(nonnull NSNumber *)position) {
    NSNumber *_position = [RCTConvert NSNumber:position];
    gint64 gst_position = [_position longLongValue];

    [self.bridge.uiManager addUIBlock:^(RCTUIManager *uiManager, NSDictionary<NSNumber *,UIView *> *viewRegistry) {
        RCTGstPlayerView *view = (RCTGstPlayerView *)viewRegistry[reactTag];
        if ([view isKindOfClass:[RCTGstPlayerView class]]) {
            [view seek:gst_position];
        }
    }];
}
@end
