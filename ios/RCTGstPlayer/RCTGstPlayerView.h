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
#import "RCTGstPlayerEventDelegate.h"

@interface RCTGstPlayerView : UIView {
    RctGstUserData *userData;
    GstState pipelineState;
}

@property (nonatomic, weak) id <RCTGstPlayerEventDelegate> delegate;

// Getters
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
