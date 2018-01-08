//
//  EaglUiView.h
//
//  Created by Alann on 13/12/2017.
//  Copyright © 2017 Kalyzee. All rights reserved.
//

#ifndef EaglUIView_h
#define EaglUIView_h

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>
#import <glib-object.h>

@interface EaglUIView : UIView {
    guintptr handle;
}

// Getters
- (guintptr) getHandle;


@end

#endif /* EaglUIView_h */
