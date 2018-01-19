//
//  RCTGstPlayerController.h
//  RCTGstPlayer
//
//  Created by Alann on 20/12/2017.
//  Copyright © 2017 Kalyzee. All rights reserved.
//

#ifndef RCTGstPlayerController_h
#define RCTGstPlayerController_h

#import <UIKit/UIKit.h>
#import "gstreamer_backend.h"
#import "EaglUIView.h"
#import "DrawableSurfaceDelegate.h"

@interface RCTGstPlayerController : UIViewController <DrawableSurfaceDelegate>
- (void)setUri:(gchar *)uri;
- (void)setAudioLevelRefreshRate:(gint)audioLevelRefreshRate;
- (void)setDebugging:(gboolean)is_debugging;
- (void)setPipelineState:(int)pipelineState;
- (void)kill;

- (void)surfaceDestroyed;
@end

#endif /* RCTGstPlayerController_h */
