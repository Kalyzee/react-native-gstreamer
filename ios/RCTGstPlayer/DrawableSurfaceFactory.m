//
//  DrawableSurfaceFactory.m
//  RCTGstPlayer
//
//  Created by Alann on 20/12/2017.
//  Copyright © 2017 Kalyzee. All rights reserved.
//

#import "DrawableSurfaceFactory.h"

@implementation DrawableSurfaceFactory

+ (EaglUIView*) getView:(UIView *)viewContainer
{
    EaglUIView* view = [[EaglUIView alloc] initWithFrame:viewContainer.frame];
    
    [view setAutoresizingMask:UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight];
    [view setBackgroundColor:[UIColor blackColor]];
    return view;
}

@end
