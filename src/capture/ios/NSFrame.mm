#include <thread>
#include <chrono>
#include "NSFrame.h"
#include "SamplesProvider.h"


@implementation CapFrameProcessor

-(WINPROCESSOR_RETURN) Init:(FrameGetterSystem*) parent second:(CMTime)interval
{
    self = [super init];
    if (self) {
        self.Working = false;
        self.Paused = false;
        self.fpw = parent;
        self.avcapturesession = [[AVCaptureSession alloc] init];

        self.avinput = [[[AVCaptureScreenInput alloc] initWithDisplayID:parent->getMonitor()->getId ()] autorelease];
        [self.avcapturesession addInput:self.avinput];

        self.output = [[AVCaptureVideoDataOutput alloc] init];
        NSDictionary* videoSettings;

        videoSettings = [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithUnsignedInt:kCVPixelFormatType_32BGRA], (id)kCVPixelBufferPixelFormatTypeKey, nil];

        NSArray<NSNumber *> * availableCode = self.output.availableVideoCVPixelFormatTypes;

        [self.output setVideoSettings:videoSettings];
        [self.output setAlwaysDiscardsLateVideoFrames:true];

        //partial capture needed
        if(parent->isPartial()){
            // The origin (0,0) is the bottom-left corner of the screen.
            CGRect r ;
            r.origin.x = parent->getBounds()->getTLX();
            r.origin.y = parent->getMonitor()->getOrgHeight() - parent->getBounds()->getTLY() - parent->getBounds()->getHeight();
            r.size.height = parent->getBounds()->getHeight();
            r.size.width  = parent->getBounds()->getWidth ();
            [self.avinput setCropRect:r];
        }

        auto sc = (CGFloat)((float)1/(float)(parent->getMonitor()->getScale()));
        [self.avinput setScaleFactor:sc];
        [self.avinput setMinFrameDuration:interval];

        self.avinput.capturesCursor = false;
        self.avinput.capturesMouseClicks = false;

        [self.avcapturesession addOutput:self.output];
        [self.output setSampleBufferDelegate:self queue:dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_LOW, 0)];
        [self.avcapturesession startRunning];

        return WINPROCESSOR_RETURN_SUCCESS;
    }
    return WINPROCESSOR_RETURN_SUCCESS;
}
- (void)dealloc
{
    [self.avcapturesession stopRunning];
    while(self.avcapturesession.isRunning || self.Working){
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    [self.output release];
    [self.avinput release];
    [self.avcapturesession release];
    [super dealloc];
}
-(void) setFrameRate:(CMTime)interval{
    [self.avinput setMinFrameDuration:interval];
}
-(void) Stop{
    [self.avcapturesession stopRunning];
}
-(void) Pause{
    if(self.Paused) return;
    self.Paused = true;
    if(self.output){
        self.output.connections[0].enabled = NO;
    }
}
-(void) Resume{
    if(!self.Paused) return;
    self.Paused = false;
    if(self.output){
        self.output.connections[0].enabled = YES;
    }
}

- (void)captureOutput:(AVCaptureOutput *)captureOutput didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer fromConnection:(AVCaptureConnection *)connection {
    self.Working = true;
    if(!self.avcapturesession.isRunning){
        self.Working = false;
        return;
    }

    CVImageBufferRef imageBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);
    CVPixelBufferLockBaseAddress(imageBuffer, kCVPixelBufferLock_ReadOnly);
    auto bytesperrow = CVPixelBufferGetBytesPerRow(imageBuffer);
    auto buf = static_cast<unsigned char*>(CVPixelBufferGetBaseAddress(imageBuffer));
    size_t bufferSize = CVPixelBufferGetDataSize(imageBuffer);
    if(self.fpw->isPartial()) {
        self.fpw->writeBuf(self.fpw->getBounds(), buf, bytesperrow, bufferSize);
    }else
        self.fpw->writeBuf(self.fpw->getMonitor(), buf, bytesperrow, bufferSize);
    CVPixelBufferUnlockBaseAddress(imageBuffer, kCVPixelBufferLock_ReadOnly);
    self.Working = false;
}

@end

class FrameProcessorImpl{
private:
    CapFrameProcessor* ptr=nullptr;

public:
    FrameProcessorImpl(){
        ptr = [[CapFrameProcessor alloc] init];
    }

    ~FrameProcessorImpl(){
        if(ptr) {
            [ptr Stop];
            [ptr release];
            ptr = nullptr;
        }
    }

    void pause(){
        if(ptr) {
            [ptr Pause];
        }
    }

    void resume(){
        if(ptr) {
            [ptr Resume];
        }
    }

    void stop(){
        if(ptr) {
            [ptr Stop];
        }
    }

    void setMinFrameDuration(const std::chrono::microseconds& duration){
        if(duration.count()>1){
            auto microsecondsinsec = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::seconds(1));
            auto secs =std::chrono::duration_cast<std::chrono::seconds>(duration);
            if(secs.count()>0){//more than 1 second duration. Im not going to do the math right now for that
                [ptr setFrameRate:CMTimeMake(1, 1)];
            } else {
                auto f =duration.count();
                auto f1 =microsecondsinsec.count();
                auto interv = f1/f;
                [ptr setFrameRate:CMTimeMake(1, interv)];
            }
        } else {
            [ptr setFrameRate:CMTimeMake(1, 100)];
        }
    }

    WINPROCESSOR_RETURN init(FrameGetterSystem* parent, const std::chrono::microseconds& duration){
        return [ptr Init:parent second:CMTimeMake(1, 1000)];
    }
};

void FrameGetterSystem::init() {
    if (_fpi && _isReInited) delete _fpi;
    else if (_fpi) return;

    _fpi = new FrameProcessorImpl();
    _fpi->init (this, _duration);
    _isReady = true;
}

void FrameGetterSystem::pause() {
    _fpi->pause();
    FrameGetterControl::pause();
}

void FrameGetterSystem::resume() {
    _fpi->resume();
    FrameGetterControl::resume();
}

void FrameGetterSystem::stop() {
    _fpi->stop();
    FrameGetterControl::stop();
}

void FrameGetterThread::init() { }
