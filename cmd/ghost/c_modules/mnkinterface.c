#include "mnkinterface.h"
#include <ApplicationServices/ApplicationServices.h>
#include <Carbon/Carbon.h>
#include <CoreFoundation/CFCGTypes.h>
#include <CoreGraphics/CGEvent.h>
#include <CoreGraphics/CGEventTypes.h>
#include <CoreGraphics/CGRemoteOperation.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/_pthread/_pthread_t.h>
#include <sys/_types/_useconds_t.h>
#include <time.h>
#include <unistd.h>

void moveMouse(int x, int y) {
  CGEventRef move = CGEventCreateMouseEvent(
      NULL, kCGEventMouseMoved, CGPointMake(x, y), kCGMouseButtonLeft);

  CGEventPost(kCGEventMouseMoved, move);
  CFRelease(move);
}

void clickMouse() {

  CGPoint cursor_position = CGEventGetLocation(CGEventCreate(NULL));

  CGEventRef click_down = CGEventCreateMouseEvent(
      NULL, kCGEventLeftMouseDown, cursor_position, kCGMouseButtonLeft);

  CGEventPost(kCGHIDEventTap, click_down);
  CFRelease(click_down);

  CGEventRef click_up = CGEventCreateMouseEvent(
      NULL, kCGEventLeftMouseUp, cursor_position, kCGMouseButtonLeft);

  CGEventPost(kCGHIDEventTap, click_up);
  CFRelease(click_up);
}

void pressKey(CGKeyCode keycode) {
  CGEventRef keyPress = CGEventCreateKeyboardEvent(NULL, keycode, true);
  CGEventPost(kCGHIDEventTap, keyPress);
  CFRelease(keyPress);
}

void longPressKey(CGKeyCode keycode, useconds_t duration, useconds_t delay) {
  CGEventRef keyDown = CGEventCreateKeyboardEvent(NULL, keycode, true);

  struct timespec ts_captured;

  if (clock_gettime(CLOCK_MONOTONIC, &ts_captured) == 0) {
    while (1) {
      struct timespec ts;

      if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
        useconds_t ts_diff_useconds = (ts.tv_nsec - ts_captured.tv_nsec) / 1000;
        if (ts_diff_useconds <= duration) {

          CGEventPost(kCGHIDEventTap, keyDown);

          usleep(delay);
        } else {
          break;
        }
      }
    }
  }

  CGEventRef keyUp = CGEventCreateKeyboardEvent(NULL, keycode, false);
  CGEventPost(kCGHIDEventTap, keyUp);
}

typedef struct {
  time_t lastInterruptTime;
  seconds_t interruptDelay;
  seconds_t activityInterval;
} Interrupt;

CGEventRef eventTapCallback(CGEventTapProxy proxy, CGEventType type,
                            CGEventRef event, void *refcon) {

  Interrupt *interrupt = (Interrupt *)refcon;
  interrupt->lastInterruptTime = time(NULL);

  return event;
}

void *interuptThreadFunction(void *params) {

  Interrupt *interrupt = (Interrupt *)params;

  while (1) {
    if (time(NULL) - interrupt->lastInterruptTime >=
        interrupt->interruptDelay) {

      printf("Interrupting\n");

      clickMouse();
      clickMouse();

      pressKey(kVK_F19);
      pressKey(kVK_F19);

      interrupt->lastInterruptTime = time(NULL);

      sleep(interrupt->activityInterval);
    } else {
      sleep(1);
    }
  }
}

void keepAlive(seconds_t *interruptDelay, seconds_t *activityInterval) {

  pthread_t interruptThread;
  Interrupt interrupt;
  interrupt.lastInterruptTime = time(NULL);
  interrupt.interruptDelay = *interruptDelay;
  interrupt.activityInterval = *activityInterval;

  CGEventMask eventMask = kCGEventMaskForAllEvents;
  CFMachPortRef eventTap = CGEventTapCreate(
      kCGSessionEventTap, kCGHeadInsertEventTap, kCGEventTapOptionDefault,
      eventMask, eventTapCallback, &interrupt);

  if (!eventTap) {
    fprintf(stderr, "Failed to create event tap\n");
    exit(1);
  }

  CFRunLoopSourceRef runLoopSource =
      CFMachPortCreateRunLoopSource(kCFAllocatorDefault, eventTap, 0);

  CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource,
                     kCFRunLoopCommonModes);

  CGEventTapEnable(eventTap, true);

  pthread_create(&interruptThread, NULL, &interuptThreadFunction, &interrupt);

  printf("Listening for events. Press Ctrl+C to exit.\n");

  CFRunLoopRun();

  pthread_join(interruptThread, NULL);
  CFRelease(runLoopSource);
  CFRelease(eventTap);
}
