#ifndef KEYPRESS_H__
#define KEYPRESS_H__

namespace keypress
{

void globalInit();
void globalDestroy();

void keyDownUp(const char* key);
void keySequence(const char* seq);
void keyDown(const char* key);
void keyUp(const char* key);

}

#endif
