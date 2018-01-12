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
#import <React/RCTViewManager.h>

@interface EaglUIView : UIView {
    guintptr handle;
}

//react-native properties
@property (nonatomic, copy) RCTBubblingEventBlock onPlayerInit;
@property (nonatomic, copy) RCTBubblingEventBlock onStateChanged;
@property (nonatomic, copy) RCTBubblingEventBlock onVolumeChanged;
@property (nonatomic, copy) RCTBubblingEventBlock onUriChanged;
@property (nonatomic, copy) RCTBubblingEventBlock onEOS;
@property (nonatomic, copy) RCTBubblingEventBlock onElementError;


// Getters
- (guintptr) getHandle;


@end

#endif /* EaglUIView_h */
