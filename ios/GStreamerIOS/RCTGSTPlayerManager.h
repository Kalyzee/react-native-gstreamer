//
//  RNTMapManager.h
//  GStreamerIOS
//
//  Created by Alann Sapone on 21/07/2017.
//  Copyright © 2017 Facebook. All rights reserved.
//

#ifndef RCTGSTPlayerManager_h
#define RCTGSTPlayerManager_h

#import <React/RCTViewManager.h>
#import "RCTGSTPlayerController.h"


@interface RCTGSTPlayerManager : RCTViewManager {
  RCTGSTPlayerController *rctGstPlayer;
}
@end

#endif /* RCTGSTPlayerManager_h */
