#ifndef SHAREDT_MOUSEEVENTS_H
#define SHAREDT_MOUSEEVENTS_H

#import <CoreFoundation/CoreFoundation.h>
#import <Foundation/Foundation.h>

@interface MouseInputIOS : NSObject

+(void) moveMouseTo:(CGPoint)coordinate;
+(void) setMouse:(CGPoint)coordinate;

+(void) leftClickAt:(CGPoint)coordinate clickCount:(int)count;
+(void) rightClickAt:(CGPoint)coordinate clickCount:(int)count;
+(void) centerClickAt:(CGPoint)coordinate clickCount:(int)count;


+(void) leftMouseDownAt:(CGPoint)coordinate;
+(void) rightMouseDownAt:(CGPoint)coordinate;
+(void) centerMouseDownAt:(CGPoint)coordinate;

+(void) leftMouseUpAt:(CGPoint)coordinate;
+(void) rightMouseUpAt:(CGPoint)coordinate;
+(void) centerMouseUpAt:(CGPoint)coordinate;

+(void) scrollLeftHorizontalLines:(int)numberOfLine;
+(void) scrollDownVerticalLines:(int)numberOfLine;

@end



#endif //SHAREDT_MOUSEEVENTS_H
