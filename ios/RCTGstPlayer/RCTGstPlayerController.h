//
//  RCTGstPlayerController.h
//  RCTGstPlayer
//
//  Created by Alann on 20/12/2017.
//  Copyright © 2017 Kalyzee. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "gstreamer_backend.h"
#import "RctGstParentView.h"
#import "DrawableSurfaceFactory.h"

@interface RCTGstPlayerController : UIViewController {
    RctGstParentView *_view;
}
@end
