//
//  DrawableSurfaceFactory.h
//  RCTGstPlayer
//
//  Created by Alann on 20/12/2017.
//  Copyright © 2017 Kalyzee. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "EaglUIView.h"

@interface DrawableSurfaceFactory : NSObject

+(EaglUIView*) getView:(UIView*)viewContainer;

@end
