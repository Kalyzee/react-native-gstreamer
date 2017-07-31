//
//  GStreamerPlayer.h
//  GStreamerIOS
//
//  Created by Alann Sapone on 21/07/2017.
//  Copyright © 2017 Facebook. All rights reserved.
//

#ifndef RCTGSTPlayer_h
#define RCTGSTPlayer_h


#import <React/RCTView.h>
#import <UIKit/UIKit.h>
#import "GStreamerBackend.h"
#import "GStreamerBackendDelegate.h"
#import "EaglUIView.h"

@class RCTEventDispatcher;

@interface RCTGSTPlayer : UIView <GStreamerBackendDelegate> {
  
  UILabel *label;
  UIButton *play_button;
  UIButton *pause_button;
  UILabel *message_label;
  
  EaglUIView *video_view;
  UIView *video_container_view;
  NSLayoutConstraint *video_width_constraint;
  NSLayoutConstraint *video_height_constraint;
  
  RCTEventDispatcher *_eventDispatcher;
}

- (instancetype)initWithEventDispatcher:(RCTEventDispatcher *)eventDispatcher;

-(void) playButtonAction:(UIButton *)button;
-(void) pauseButtonAction:(UIButton *)button;

/* From GStreamerBackendDelegate */
-(void) gstreamerInitialized;
-(void) gstreamerSetUIMessage:(NSString *)message;

@end

#endif /* RCTGSTPlayer_h */
