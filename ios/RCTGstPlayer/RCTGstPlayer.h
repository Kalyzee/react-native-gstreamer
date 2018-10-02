//
//  RCTGstPlayer.h
//  RCTGstPlayer
//
//  Created by Alann on 20/12/2017.
//  Copyright © 2017 Kalyzee. All rights reserved.
//

#ifndef RCTGstPlayer_h
#define RCTGstPlayer_h

#import <React/RCTViewManager.h>
#import <Foundation/Foundation.h>
#import "gst_ios_init.h"
#import "RCTGstPlayerView.h"
#import "RCTGstPlayerEventDelegate.h"

@interface RCTGstPlayer : RCTViewManager <RCTGstPlayerEventDelegate>
@end

#endif /* RCTGstPlayer_h */
