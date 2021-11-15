#ifndef SHAREDT_KEYBOARDEVENTS_H
#define SHAREDT_KEYBOARDEVENTS_H
#import <CoreGraphics/CGEvent.h>
#import <Foundation/Foundation.h>
#import <CoreFoundation/CoreFoundation.h>

@interface KeyboardEvents : NSObject {
}

+(void) pressKeyboardEvent:(CGKeyCode)keyCode isDown:(bool)down;
@end

#endif //SHAREDT_KEYBOARDEVENTS_H
