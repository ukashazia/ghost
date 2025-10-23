#ifndef MNKINTERFACE_H
#define MNKINTERFACE_H

#include <Carbon/Carbon.h>
#include <CoreGraphics/CoreGraphics.h>
#include <sys/_types/_useconds_t.h>

typedef unsigned long seconds_t;

void moveMouse(int x, int y);
void clickMouse();
void pressKey(CGKeyCode keycode);
void longPressKey(CGKeyCode keycode, useconds_t duration, useconds_t delay);
void keepAlive(seconds_t *interruptDelay, seconds_t *activityInterval);

#endif
