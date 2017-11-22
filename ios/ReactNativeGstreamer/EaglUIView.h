//
//  EaglUIView.h
//  GStreamerIOS
//
//  Created by Alann Sapone on 25/07/2017.
//  Copyright © 2017 Facebook. All rights reserved.
//

#ifndef EaglUIView_h
#define EaglUIView_h

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#import <React/RCTViewManager.h>



@interface EaglUIView : UIView
@property (nonatomic, copy) RCTBubblingEventBlock onAudioLevelChange;
@end



#endif /* EaglUIView_h */
