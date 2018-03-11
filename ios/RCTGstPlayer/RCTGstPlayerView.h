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
}

// react-native events
@property (nonatomic, copy) RCTBubblingEventBlock onPlayerInit;
@property (nonatomic, copy) RCTBubblingEventBlock onStateChanged;
@property (nonatomic, copy) RCTBubblingEventBlock onVolumeChanged;
@property (nonatomic, copy) RCTBubblingEventBlock onUriChanged;
@property (nonatomic, copy) RCTBubblingEventBlock onPlayingProgress;
@property (nonatomic, copy) RCTBubblingEventBlock onBufferingProgress;
@property (nonatomic, copy) RCTBubblingEventBlock onEOS;
@property (nonatomic, copy) RCTBubblingEventBlock onElementError;

// Getters
+ (RCTGstPlayerView *)getView;

- (guintptr)getHandle;
- (RctGstUserData *)getUserData;
- (gboolean)isReady;

// Methods
- (void)setPipelineState:(int)pipelineState;
- (void)seek:(gint64)position;

// Setters
- (void)setShareInstance:(BOOL)_shareInstance;

@end

#endif /* RCTGstPlayerView_h */
