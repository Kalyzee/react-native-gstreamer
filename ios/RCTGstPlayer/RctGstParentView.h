//
//  RctGstParentView.h
//  RCTGstPlayer
//
//  Created by Alann on 21/12/2017.
//  Copyright © 2017 Kalyzee. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <React/RCTViewManager.h>

@interface RctGstParentView : UIView

//react-native properties
@property (nonatomic, copy) RCTBubblingEventBlock onPlayerInit;
@property (nonatomic, copy) RCTBubblingEventBlock onStateChanged;
@property (nonatomic, copy) RCTBubblingEventBlock onVolumeChanged;
@property (nonatomic, copy) RCTBubblingEventBlock onUriChanged;
@property (nonatomic, copy) RCTBubblingEventBlock onEOS;
@property (nonatomic, copy) RCTBubblingEventBlock onElementError;

@end
