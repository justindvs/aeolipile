#ifndef KEYPRESS_H__
#define KEYPRESS_H__

namespace keypress
{

void keyDown(const char* key);
void keyUp(const char* key);
void keyDownUp(const char* key);
void keySequence(const char* seq);

void globalInit();
void globalDestroy();

}

#endif
