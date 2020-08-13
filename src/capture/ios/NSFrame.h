#pragma once

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#include "ScreenProvider.h"
#include "ImageRect.h"
#include "WindowProcessor.h"
#include "SamplesProvider.h"

@interface CapFrameProcessor: NSObject<AVCaptureVideoDataOutputSampleBufferDelegate>
@property(nonatomic, assign) FrameProcessorWrap* fpw;
@property(nonatomic, retain) AVCaptureSession *avcapturesession;
@property(nonatomic, retain) AVCaptureVideoDataOutput *output;
@property(nonatomic, retain) AVCaptureScreenInput* avinput;
@property(atomic) bool Working;
@property(atomic) bool Paused;
-(WINPROCESSOR_RETURN) Init:(FrameProcessorWrap*) parent second:(CMTime)interval;
-(void) setFrameRate:(CMTime)interval;
-(void) Stop;
-(void) Pause;
-(void) Resume;
@end
