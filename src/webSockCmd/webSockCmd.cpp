#include "webSockCmd.h"
#include <stdio.h>
#include "keyPress.h"
#include <string.h>
#include <stdint.h>
#include <mongoose.h>

namespace wscmd
{
const char END_OF_BUF_CHAR = 0x1E;

static char* skipSpaces(char* cmd) {
   while(*cmd && ((*cmd == ' ') || (*cmd == '\t'))) ++cmd;
   return cmd;
}

static char* firstToken(char* cmd) {
   char* endOfToken = strpbrk(cmd, " \t");
   if (endOfToken == NULL) return cmd;

   // NULL terminate token
   *endOfToken = '\0';
   return cmd;
}

static char* nextToken(char* cmd) {
  char* endOfThisToken = strchr(cmd, '\0');
  cmd = endOfThisToken + 1;
  
  if (*cmd == END_OF_BUF_CHAR) {
    // End of cmd
    return NULL;
  }
  return skipSpaces(cmd);
}

void processCmd(mg_connection* conn, char* cmd, size_t cmdLen)
{
   // Null-terminate the command and add END_OF_BUF_CHAR to the end of
   // the cmd buffer.  This makes it easier to parse the command
   // token-by-token.
   cmd[cmdLen] = '\0';
   cmd[cmdLen+1] = END_OF_BUF_CHAR;

   char* cmdName = firstToken(cmd);

   if (strcmp("ping", cmdName) == 0) {
      char response[] = "OK";
      mg_websocket_write(conn, 1, response, sizeof(response)-1);
   }
   else if (strcmp("keydownup", cmdName) == 0) {
      char* keyName = nextToken(cmd);
      keypress::keyDownUp(keyName);
   }
   else if (strcmp("keyseq", cmdName) == 0) {
      char* keyName = nextToken(cmd);
      keypress::keySequence(keyName);
   }
   else if (strcmp("keydown", cmdName) == 0) {
      char* keyName = nextToken(cmd);
      keypress::keyDown(keyName);    
   }
   else if (strcmp("keyup", cmdName) == 0) {
      char* keyName = nextToken(cmd);
      keypress::keyUp(keyName);
   }
   else {
      printf("Unhandled cmd \"%s\"\n", cmdName);
   }
}

} // namespace wscmd
