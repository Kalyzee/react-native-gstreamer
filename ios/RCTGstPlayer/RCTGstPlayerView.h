//
//  EaglUiView.h
//
//  Created by Alann on 13/12/2017.
//  Copyright © 2017 Kalyzee. All rights reserved.
//

#ifndef RCTGstPlayerView_h
#define RCTGstPlayerView_h

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <glib-object.h>
#import <React/RCTViewManager.h>
#import "gstreamer_backend.h"

@interface RCTGstPlayerView : UIView {
    RctGstUserData *userData;
    GstState pipelineState;
    gboolean is_view_ready;
    
    NSString *uri;
    guint64 refreshRate;
    gdouble volume;
}

// react-native events
@property (nonatomic, copy) RCTBubblingEventBlock onPlayerInit;
@property (nonatomic, copy) RCTBubblingEventBlock onPadAdded;
@property (nonatomic, copy) RCTBubblingEventBlock onStateChanged;
@property (nonatomic, copy) RCTBubblingEventBlock onVolumeChanged;
@property (nonatomic, copy) RCTBubblingEventBlock onUriChanged;
@property (nonatomic, copy) RCTBubblingEventBlock onPlayingProgress;
@property (nonatomic, copy) RCTBubblingEventBlock onBufferingProgress;
@property (nonatomic, copy) RCTBubblingEventBlock onEOS;
@property (nonatomic, copy) RCTBubblingEventBlock onElementError;
@property (nonatomic, copy) RCTBubblingEventBlock onElementLog;

// Getters
+ (RCTGstPlayerView *)getView;

- (guintptr)getHandle;
- (RctGstUserData *)getUserData;
- (gboolean)isReady;

// Methods
- (void) setViewReady:(gboolean) is_view_ready;
- (void)setPipelineState:(int)pipelineState;
- (void)seek:(gint64)position;

- (void)setUri:(NSString*)uri;
- (void)setRefreshRate:(guint64)refreshRate;
- (void)setVolume:(gdouble)volume;

// Setters
- (void)setShareInstance:(BOOL)_shareInstance;
- (void)setApplicationState:(NSString *)applicationState;

@end

#endif /* RCTGstPlayerView_h */
