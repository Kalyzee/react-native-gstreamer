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
#import "RCTGstPlayerController.h"
#import "gst_ios_init.h"

@interface RCTGstPlayer : RCTViewManager
{
    RCTGstPlayerController* playerController;
}

- (RCTGstPlayerController *)getController;

@end

#endif /* RCTGstPlayer_h */
