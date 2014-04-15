#include "keypress.h"
#include <iostream>
#include <windows.h>
#include <stdint.h>
#include <vector>

#pragma warning(disable:4127) // conditional expression is constant

namespace keypress {

const int MAX_TIMERS = 64;
const int DEFAULT_UPDOWN_TIME_MS = 300;
const int SEQ_KEYPRESS_DURATION_MS = 100;
const int SEQ_INTER_KEY_DELAY_MS = 30;

using namespace std;

enum KeyDir {
   KeyDir_Down,
   KeyDir_Up
};

struct TimerInfo {
   PTP_TIMER timer;
   int key;
   KeyDir dir;
   bool done;

   TimerInfo() {
      reinit(0, KeyDir_Down);
      this->done = true;
   }
   void init(PTP_TIMER timer) {
      this->timer = timer;
   }
   void reinit(int key, KeyDir dir) {
      this->key = key;
      this->dir = dir;
      this->done = false;
   }
   void setDone() { done = true; }
   bool isDone() { return done; }
};

TimerInfo timers[MAX_TIMERS];

TimerInfo& getFreeTimer() {
   static int currentTimerIndex = 0;

   int thisIndex = currentTimerIndex++;
   // Roll over to 0 if we've reached MAX_TIMERS
   currentTimerIndex = currentTimerIndex % MAX_TIMERS;

   TimerInfo& freeTimer = timers[thisIndex];

   if (!freeTimer.isDone()) {
      printf("Error: Trying to Re-use a timer that is not done!\n");
   }
   return freeTimer;
}
   
void sendKeyInternal(int key, KeyDir dir) {
   static int counter = 0;

   INPUT input;
   memset(&input, 0, sizeof(input));
   input.type = INPUT_KEYBOARD;
   input.ki.wVk = (WORD)key;
   if (dir == KeyDir_Up) input.ki.dwFlags = KEYEVENTF_KEYUP;

   uint32_t numEventsSent = SendInput(1, &input, sizeof(INPUT));

   printf("%i: Sent key %s: %i\n", counter++, (dir == KeyDir_Down) ? "down" : "up", key);
   if (numEventsSent != 1) printf("ERROR: Failed to send key event\n");
}

int keyNameToCode(const char* key) {
   size_t keyLen = strlen(key);
   int thisKey = -1;

   if (keyLen == 1) {
      char keyChar = *key;
      // TODO: Translate lower-case to upper case (which map to the
      //       right key codes).
      if ((keyChar >= 0x21) &&
          (keyChar <= 0x7e)) {
         thisKey = keyChar;
      }
      else {
         printf("ERROR: Key %i not supported\n", keyChar);
      }
   }
   else {
      // multi-char key sequence
      #define KEYNAME_MAPPING(nameStr, code) \
         else if (strcmp(nameStr, key) == 0) { thisKey = code; }

      if (false) {}
      #include "keynames.def"
   }
   return thisKey;
}

void keyDown(const char* key) {
   int keyCode = keyNameToCode(key);
   if (keyCode == -1) return;
   sendKeyInternal(keyCode, KeyDir_Down);
}

void keyUp(const char* key) {
   int keyCode = keyNameToCode(key);
   if (keyCode == -1) return;
   sendKeyInternal(keyCode, KeyDir_Up);
}

void scheduleFutureKeypress(int timeMs, int key, KeyDir dir) {
   TimerInfo& thisTimer = getFreeTimer();
   thisTimer.reinit(key, dir);

   // Time is measured in 100ns increments.  Negative value indicates a
   // relative time.
   int64_t timeVal = ((int64_t)timeMs) * -10000;
   // TODO: document last 2 params
   SetThreadpoolTimer(thisTimer.timer, (FILETIME*)&timeVal, 0, 0);
}

void keyDownUp(const char* key) {
   int keyCode = keyNameToCode(key);
   if (keyCode == -1) return;

   sendKeyInternal(keyCode, KeyDir_Down);
   scheduleFutureKeypress(DEFAULT_UPDOWN_TIME_MS, keyCode, KeyDir_Up);
}

void keySequence(const char* seq) {
   char thisCharBuf[64];
   const char* endChar;
   int charLen;
   int keyCode;
   int timeLine = 0;

   // seq is of the form "w;a;s;d;"

   while (*seq) {
      endChar = strchr(seq, ';');
      charLen = (int)(endChar - seq);

      memcpy(thisCharBuf, seq, charLen);
      thisCharBuf[charLen] = '\0';

      keyCode = keyNameToCode(thisCharBuf);
      if (keyCode == -1) {
         printf("Failed to translate key \"%s\" to code\n", thisCharBuf);
         return;
      }

      scheduleFutureKeypress(timeLine, keyCode, KeyDir_Down);
      timeLine += SEQ_KEYPRESS_DURATION_MS;
      scheduleFutureKeypress(timeLine, keyCode, KeyDir_Up);
      timeLine += SEQ_INTER_KEY_DELAY_MS;

      seq = endChar + 1;
   }
}

void timerCallback(PTP_CALLBACK_INSTANCE, void* context, PTP_TIMER) {
   TimerInfo* thisTimer = (TimerInfo*)context;
   sendKeyInternal(thisTimer->key, thisTimer->dir);
   thisTimer->setDone();
}

void globalInit() {
   for (int x = 0; x < MAX_TIMERS; ++x) {
      timers[x].init(CreateThreadpoolTimer(timerCallback, &timers[x], NULL));
   }
}

void globalDestroy() {
   for (int x = 0; x < MAX_TIMERS; ++x) {
      CloseThreadpoolTimer(timers[x].timer);
   }
}

} // namespace keypress
