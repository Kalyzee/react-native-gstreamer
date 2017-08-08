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
#import "GstPlayerController.h"


@interface GstPlayerManager : RCTViewManager {
  GstPlayerController *rctGstPlayer;
}
@end

#endif /* RCTGSTPlayerManager_h */
