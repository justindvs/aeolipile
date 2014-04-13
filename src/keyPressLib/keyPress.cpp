#include "keyPress.h"
#include <iostream>
#include <windows.h>
#include <stdint.h>
#include <vector>

using namespace std;

#pragma warning(disable:4127) // conditional expression is constant

#define MAX_TIMERS 64

enum KeyDir
{
  KeyDir_Down,
  KeyDir_Up
};

struct TimerInfo
{
  PTP_TIMER timer;
  int key;
  KeyDir dir;
  bool done;

  void init(PTP_TIMER timer)
  {
    this->timer = timer;
    this->key = 0;
    this->dir = KeyDir_Down;
    this->done = true;
  }
  void reinit(int key, KeyDir dir)
  {
    this->key = key;
    this->dir = dir;
    this->done = false;
  }
  void setDone()
  {
    done = true;
  }
  bool isDone()
  {
    return done;
  }
};

TimerInfo timers[MAX_TIMERS];
int currentTimerIndex;

void sendKeyInternal(int key, KeyDir dir)
{
  static int counter = 0;
  
  INPUT input;
  memset(&input, 0, sizeof(input));
  input.type = INPUT_KEYBOARD;
  input.ki.wVk = (WORD)key;
  if (dir == KeyDir_Up) input.ki.dwFlags = KEYEVENTF_KEYUP;

  printf("%i: Sending key %s: %i\n", counter++, (dir == KeyDir_Down) ? "down" : "up", key);
  
  uint32_t numEventsSent = SendInput(1, &input, sizeof(INPUT));
  if (numEventsSent != 1) printf("ERROR: Failed to send key event\n");
}

int keyNameToCode(const char* key)
{
  size_t keyLen = strlen(key);
  int thisKey = -1;

  if (keyLen == 1)
  {
    char keyChar = *key;
    if ((keyChar >= 0x21) &&
	(keyChar <= 0x7e))
    {
      thisKey = keyChar;
    }
    else
    {
      printf("ERROR: Key %i not supported\n", keyChar);
    }
  }
  else
  {
    // multi-char key sequence
    #define KEYNAME_MAPPING(nameStr, code) \
      else if (strcmp(nameStr, key) == 0) { thisKey = code; }

    if (false) {}
    #include "keyNames.def"
  }
  return thisKey;
}

void keyDown(const char* key)
{
  int keyCode = keyNameToCode(key);
  if (keyCode == -1) return;
  sendKeyInternal(keyCode, KeyDir_Down);
}

void keyUp(const char* key)
{
  int keyCode = keyNameToCode(key);
  if (keyCode == -1) return;
  sendKeyInternal(keyCode, KeyDir_Up);
}

void scheduleFutureKeypress(
   int timeMs,
   int key,
   KeyDir dir)
{
  int thisIndex = currentTimerIndex++;
  // Roll over to 0 if we've reached MAX_TIMERS
  currentTimerIndex = currentTimerIndex % MAX_TIMERS;
  TimerInfo& thisTimer = timers[thisIndex];

  if (!thisTimer.isDone()) printf("Warning: Re-using a timer that is not done!\n");

  thisTimer.reinit(key, dir);
  
  // Time is measured in 100ns increments.  Negative value indicates a
  // relative time.
  int64_t timeVal = ((int64_t)timeMs) * -10000;
  SetThreadpoolTimer(thisTimer.timer,
		     (FILETIME*)&timeVal,
		     0,
		     0);
}

void keyDownUp(const char* key)
{
  int keyCode = keyNameToCode(key);
  if (keyCode == -1) return;
  
  sendKeyInternal(keyCode, KeyDir_Down);
  scheduleFutureKeypress(300, keyCode, KeyDir_Up);
}

void keySequence(const char* seq)
{
  char tempCharBuf[64];
  const char* endChar;
  int charLen;
  int keyCode;
  int timeLine = 0;
  int keyPressDuration = 100;
  int nextKeyDelay = 30;

  // printf("sleeping for 5 seconds...\n");
  // Sleep(5000);
  
  // seq is of the form "w;a;s;d;"

  while (*seq)
  {
    endChar = strchr(seq, ';');
    charLen = (int)(endChar - seq);
  
    memcpy(tempCharBuf, seq, charLen);
    tempCharBuf[charLen] = '\0';

    keyCode = keyNameToCode(tempCharBuf);
    if (keyCode == -1)
    {
       printf("Failed to translate key \"%s\" to code\n", tempCharBuf);
       return;
    }
  
    scheduleFutureKeypress(timeLine, keyCode, KeyDir_Down);
    timeLine += keyPressDuration;
    scheduleFutureKeypress(timeLine, keyCode, KeyDir_Up);
    timeLine += nextKeyDelay;
  
    seq = endChar + 1;
  }
}
 
void timerCallback(
   PTP_CALLBACK_INSTANCE /*instance*/,
   void* context,
   PTP_TIMER /*timer*/ )
{
  TimerInfo* thisTimer = (TimerInfo*)context;

  sendKeyInternal(thisTimer->key, thisTimer->dir);
  thisTimer->setDone();
}

void globalInit()
{
  currentTimerIndex = 0;

  for (int x = 0; x < MAX_TIMERS; ++x)
  {
    timers[x].init(CreateThreadpoolTimer(timerCallback,
					 &timers[x],
					 NULL));
  }
}

void globalDestroy()
{
  for (int x = 0; x < MAX_TIMERS; ++x)
  {
    CloseThreadpoolTimer(timers[x].timer);
  }
}
