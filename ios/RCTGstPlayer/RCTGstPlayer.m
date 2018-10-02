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

- (NSArray<NSString *> *)supportedEvents
{
    return @[
             @"onPlayerInit",
             @"onPadAdded",
             @"onVolumeChanged",
             @"onStateChanged",
             @"onUriChanged",
             @"onPlayingProgress",
             @"onBufferingProgress",
             @"onEOS",
             @"onElementError",
             @"onElementLog",
             ];
}

- (void)playerInited:(RCTGstPlayerView *)sender {
    [self.bridge.eventDispatcher sendAppEventWithName:@"onPlayerInit" body:@{}];
}

- (void)padAdded:(RCTGstPlayerView *)sender withData:(NSMutableDictionary *)data
{
    [self.bridge.eventDispatcher sendAppEventWithName:@"onPadAdded" body:data];
}

- (void)volumeChanged:(RCTGstPlayerView *)sender withData:(NSMutableDictionary *)data
{
    [self.bridge.eventDispatcher sendAppEventWithName:@"onVolumeChanged" body:data];
}

- (void)stateChanged:(RCTGstPlayerView *)sender withData:(NSMutableDictionary *)data
{
    [self.bridge.eventDispatcher sendAppEventWithName:@"onStateChanged" body:data];
}

- (void)uriChanged:(RCTGstPlayerView *)sender withData:(NSMutableDictionary *)data
{
    [self.bridge.eventDispatcher sendAppEventWithName:@"onUriChanged" body:data];
}

- (void)playingProgress:(RCTGstPlayerView *)sender withData:(NSMutableDictionary *)data
{
    [self.bridge.eventDispatcher sendAppEventWithName:@"onPlayingProgress" body:data];
}

- (void)bufferingProgress:(RCTGstPlayerView *)sender withData:(NSMutableDictionary *)data
{
    [self.bridge.eventDispatcher sendAppEventWithName:@"onBufferingProgress" body:data];
}

- (void)eos:(RCTGstPlayerView *)sender
{
    [self.bridge.eventDispatcher sendAppEventWithName:@"onEOS" body:@{}];
}

- (void)elementError:(RCTGstPlayerView *)sender withData:(NSMutableDictionary *)data
{
    [self.bridge.eventDispatcher sendAppEventWithName:@"onElementError" body:data];
}

- (void)elementLog:(RCTGstPlayerView *)sender withData:(NSMutableDictionary *)data
{
    /*
    RCTEvent *event = [[RCTEvent alloc] init];

    [self.bridge.eventDispatcher sendEvent:event];
     */
}


// react-native init
- (UIView *)view
{
    RCTGstPlayerView *view = [[RCTGstPlayerView alloc] init];
    view.delegate = self;
        
    // Preparing pipeline
    rct_gst_init([view getUserData]);
    
    return view;
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
    // [view setShareInstance:[RCTConvert BOOL:json]];
}

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
