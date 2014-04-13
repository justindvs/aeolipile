#ifndef KEYPRESS_H__
#define KEYPRESS_H__

#include <lua.h>
#include <utils.h>

HM_EXTERN_C void keyDown(const char* key);
HM_EXTERN_C void keyUp(const char* key);
HM_EXTERN_C void keyDownUp(const char* key);
HM_EXTERN_C void keySequence(const char* seq);

HM_EXTERN_C void globalInit();
HM_EXTERN_C void globalDestroy();

#endif
