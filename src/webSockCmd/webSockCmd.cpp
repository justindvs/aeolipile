#include "webSockCmd.h"
#include <stdio.h>
#include "keyPress.h"
#include <string.h>
#include <stdint.h>
#include <mongoose.h>

char* skipSpaces(char* cmd)
{
  while(*cmd &&
	((*cmd == ' ') ||
	 (*cmd == '\t')))
  {
    ++cmd;
  }
  return cmd;
}

char* firstToken(char* cmd)
{
  char* endOfToken = strpbrk(cmd, " \t");
  if (endOfToken == NULL) return cmd;

  // NULL terminate token
  *endOfToken = '\0';
  return cmd;
}

char* nextToken(char* cmd)
{
  char* endOfThisToken = strchr(cmd, '\0');
  cmd = endOfThisToken + 1;
  
  if (*cmd == END_OF_BUF_CHAR)
  {
    // End of cmd
    return NULL;
  }

  cmd = skipSpaces(cmd);
  return cmd;
}

void processCmd(mg_connection* conn, char* cmd)
{
  // keydown J
  // keyup J
  char* cmdName;
  char* keyName;

  cmdName = firstToken(cmd);

  if (strcmp("ping", cmdName) == 0)
  {
    char response[] = "OK";
    mg_websocket_write(conn, 1, response, sizeof(response)-1);
  }
  else if (strcmp("keydown", cmdName) == 0)
  {
    keyName = nextToken(cmd);
    keyDown(keyName);    
  }
  else if (strcmp("keyup", cmdName) == 0)
  {
    keyName = nextToken(cmd);
    keyUp(keyName);
  }
  else if (strcmp("keydownup", cmdName) == 0)
  {
    keyName = nextToken(cmd);
    keyDownUp(keyName);
  }
  else if (strcmp("keyseq", cmdName) == 0)
  {
    keyName = nextToken(cmd);
    keySequence(keyName);
  }
  else
  {
    printf("Unhandled cmd \"%s\"\n", cmdName);
  }
  
  //printf("Received cmd: \"%s\"\n", cmd);
}
