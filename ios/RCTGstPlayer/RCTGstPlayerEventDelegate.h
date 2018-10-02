//
//  RCTGstPlayerEventDelegate.h
//  RCTGstPlayer
//
//  Created by Alann Sapone on 01/10/2018.
//  Copyright © 2018 Kalyzee. All rights reserved.
//

#import "./RCTGstPlayerView.h"

@class RCTGstPlayerView;
@protocol RCTGstPlayerEventDelegate <NSObject>
- (void) playerInited:(RCTGstPlayerView *) sender;
- (void) padAdded:(RCTGstPlayerView *) sender withData:(NSMutableDictionary *)data;
- (void) volumeChanged:(RCTGstPlayerView *) sender withData:(NSMutableDictionary *)data;
- (void) stateChanged:(RCTGstPlayerView *) sender withData:(NSMutableDictionary *)data;
- (void) uriChanged:(RCTGstPlayerView *)sender withData:(NSMutableDictionary *)data;
- (void) playingProgress:(RCTGstPlayerView *)sender withData:(NSMutableDictionary *)data;
- (void) bufferingProgress:(RCTGstPlayerView *)sender withData:(NSMutableDictionary *)data;
- (void) eos:(RCTGstPlayerView *)sender;
- (void) elementError:(RCTGstPlayerView *)sender withData:(NSMutableDictionary *)data;
- (void) elementLog:(RCTGstPlayerView *)sender withData:(NSMutableDictionary *)data;

@end
