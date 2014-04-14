#ifndef WEBSOCKCMD_H_
#define WEBSOCKCMD_H_

#include <utils.h>

#define END_OF_BUF_CHAR 0x1E

struct mg_connection;

HM_EXTERN_C void processCmd(struct mg_connection* conn, char* cmd, size_t cmdLen);

#endif
