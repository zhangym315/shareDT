#import <CoreGraphics/CGEvent.h>
#import "MouseEvents.h"
#include "InputInterface.h"

@implementation MouseInputIOS

+(void) moveMouseTo:(CGPoint)coordinate {
    CGEventRef ourEvent = CGEventCreate(NULL);
    CGPoint originCoordOfMouse = CGEventGetLocation(ourEvent);
    
    for(int i = 0; i < 10; i++) {
        float x = ((coordinate.x - originCoordOfMouse.x)/10 * i) + originCoordOfMouse.x;
        float y = ((coordinate.y - originCoordOfMouse.y)/10 * i) + originCoordOfMouse.y;
        [MouseInputIOS setMouse:CGPointMake(x, y)];
        usleep(100*1000);
    }
    CFRelease(ourEvent);
}

+(void) setMouse:(CGPoint)coordinate {
    CGEventRef move = CGEventCreateMouseEvent(NULL, kCGEventMouseMoved, coordinate, kCGMouseButtonLeft);
    CGEventPost(kCGHIDEventTap, move);
    CFRelease(move);
}

+(void) leftClick:(int)count
{
    CGEventRef ourEvent = CGEventCreate(NULL);
    CGPoint originCoordOfMouse = CGEventGetLocation(ourEvent);
    [MouseInputIOS leftClickAt:originCoordOfMouse clickCount:count];
    CFRelease(ourEvent);
}

+(void) rightClick:(int)count
{
    CGEventRef ourEvent = CGEventCreate(NULL);
    CGPoint originCoordOfMouse = CGEventGetLocation(ourEvent);
    [MouseInputIOS rightClickAt:originCoordOfMouse clickCount:count];
    CFRelease(ourEvent);
}

+(void) centerClick:(int)count
{
    CGEventRef ourEvent = CGEventCreate(NULL);
    CGPoint originCoordOfMouse = CGEventGetLocation(ourEvent);
    [MouseInputIOS centerClickAt:originCoordOfMouse clickCount:count];
    CFRelease(ourEvent);
}

+(void) leftClickAt:(CGPoint)coordinate clickCount:(int)count
{
    CGEventRef click_down = CGEventCreateMouseEvent(NULL, kCGEventLeftMouseDown, coordinate, kCGMouseButtonLeft);
    CGEventRef click_up = CGEventCreateMouseEvent(NULL, kCGEventLeftMouseUp, coordinate, kCGMouseButtonLeft);
    CGEventPost(kCGHIDEventTap, click_down);
    CGEventPost(kCGHIDEventTap, click_up);
    CFRelease(click_down);
    CFRelease(click_up);
}

+(void) rightClickAt:(CGPoint)coordinate clickCount:(int)count
{
    CGEventRef click_down = CGEventCreateMouseEvent(NULL, kCGEventRightMouseDown, coordinate, kCGMouseButtonRight);
    CGEventRef click_up = CGEventCreateMouseEvent(NULL, kCGEventRightMouseUp, coordinate, kCGMouseButtonRight);
    CGEventPost(kCGHIDEventTap, click_down);
    CGEventPost(kCGHIDEventTap, click_up);
    CFRelease(click_down);
    CFRelease(click_up);
}

+(void) centerClickAt:(CGPoint)coordinate clickCount:(int)count
{
    CGEventRef click_down = CGEventCreateMouseEvent(NULL, kCGEventOtherMouseDown, coordinate, kCGMouseButtonCenter);
    CGEventRef click_up = CGEventCreateMouseEvent(NULL, kCGEventOtherMouseUp, coordinate, kCGMouseButtonCenter);
    CGEventPost(kCGHIDEventTap, click_down);
    CGEventPost(kCGHIDEventTap, click_up);
    CFRelease(click_down);
    CFRelease(click_up);
}

+(void) leftMouseDownAt:(CGPoint)coordinate
{
    CGEventRef click_down = CGEventCreateMouseEvent(NULL, kCGEventLeftMouseDown, coordinate, kCGMouseButtonLeft);
    CGEventPost(kCGHIDEventTap, click_down);
    CFRelease(click_down);
} 

+(void) rightMouseDownAt:(CGPoint)coordinate
{
    CGEventRef click_down = CGEventCreateMouseEvent(NULL, kCGEventRightMouseDown, coordinate, kCGMouseButtonRight);
    CGEventPost(kCGHIDEventTap, click_down);
    CFRelease(click_down);
}

+(void) centerMouseDownAt:(CGPoint)coordinate
{
    CGEventRef click_down = CGEventCreateMouseEvent(NULL, kCGEventOtherMouseDown, coordinate, kCGMouseButtonCenter);
    CGEventPost(kCGHIDEventTap, click_down);
    CFRelease(click_down);
}

+(void) leftMouseUpAt:(CGPoint)coordinate
{
    CGEventRef click_up = CGEventCreateMouseEvent(NULL, kCGEventLeftMouseUp, coordinate, kCGMouseButtonLeft);
    CGEventPost(kCGHIDEventTap, click_up);
    CFRelease(click_up);
}

+(void) rightMouseUpAt:(CGPoint)coordinate
{
    CGEventRef click_up = CGEventCreateMouseEvent(NULL, kCGEventRightMouseUp, coordinate, kCGMouseButtonRight);
    CGEventPost(kCGHIDEventTap, click_up);
    CFRelease(click_up);
}

+(void) centerMouseUpAt:(CGPoint)coordinate
{
    CGEventRef click_up = CGEventCreateMouseEvent(NULL, kCGEventOtherMouseUp, coordinate, kCGMouseButtonCenter);
    CGEventPost(kCGHIDEventTap, click_up);
    CFRelease(click_up);
}

+(void) scrollRightHorizontalLines:(int)numberOfLine
{
    CGEventRef scroll = CGEventCreateScrollWheelEvent(NULL, kCGScrollEventUnitLine, 2, 0, -numberOfLine);
    CGEventPost(kCGHIDEventTap, scroll);
    CFRelease(scroll);
}

+(void) scrollLeftHorizontalLines:(int)numberOfLine
{
    CGEventRef scroll = CGEventCreateScrollWheelEvent(NULL, kCGScrollEventUnitLine, 2, 0, numberOfLine);
    CGEventPost(kCGHIDEventTap, scroll);
    CFRelease(scroll);
}

+(void) scrollUpVerticalLines:(int)numberOfLine
{
    CGEventRef scroll = CGEventCreateScrollWheelEvent(NULL, kCGScrollEventUnitLine, 1, -numberOfLine);
    CGEventPost(kCGHIDEventTap, scroll);
    CFRelease(scroll);
}

+(void) scrollDownVerticalLines:(int)numberOfLine
{
    CGEventRef scroll = CGEventCreateScrollWheelEvent(NULL, kCGScrollEventUnitLine, 1, numberOfLine);
    CGEventPost(kCGHIDEventTap, scroll);
    CFRelease(scroll);
}

@end

void InputMousePlatform::mouseClickAtCordinate(Cordinate c, MouseButton b, int count)
{
    if ((b & MouseButtonMask) == LeftButton) {
        if ((b & MousePressMask) == ButtonDown) {
            [MouseInputIOS leftMouseDownAt:CGPointMake(c._x, c._y)];
        } else if ((b & MousePressMask) == ButtonUp) {
            [MouseInputIOS leftMouseUpAt:CGPointMake(c._x, c._y)];
        } else {
            [MouseInputIOS leftClickAt:CGPointMake(c._x, c._y) clickCount:count];
        }
    }

    if ((b & MouseButtonMask) == RightButton) {
        if ((b & MousePressMask) == ButtonDown) {
            [MouseInputIOS rightMouseDownAt:CGPointMake(c._x, c._y)];
        } else if ((b & MousePressMask) == ButtonUp) {
            [MouseInputIOS rightMouseUpAt:CGPointMake(c._x, c._y)];
        } else {
            [MouseInputIOS rightClickAt:CGPointMake(c._x, c._y) clickCount:count];
        }
    }

    if ((b & MouseButtonMask) == MiddleButton) {
        if ((b & MousePressMask) == ButtonDown) {
            [MouseInputIOS centerMouseDownAt:CGPointMake(c._x, c._y)];
        } else if ((b & MousePressMask) == ButtonUp) {
            [MouseInputIOS centerMouseUpAt:CGPointMake(c._x, c._y)];
        } else {
            [MouseInputIOS centerClickAt:CGPointMake(c._x, c._y) clickCount:count];
        }
    }

    if ((b & ~MouseButtonMask) == WheeleMoved) {
        if (c._y > 0) {
            [MouseInputIOS scrollUpVerticalLines:2];
        } else if (c._y < 0) {
            [MouseInputIOS scrollDownVerticalLines:2];
        }

        if (c._x > 0) {
            [MouseInputIOS scrollLeftHorizontalLines:(2)];
        } else if (c._x < 0) {
            [MouseInputIOS scrollRightHorizontalLines:(2)];
        }
    }
}