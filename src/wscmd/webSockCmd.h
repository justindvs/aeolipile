#ifndef WEBSOCKCMD_H_
#define WEBSOCKCMD_H_

struct mg_connection;

namespace wscmd
{
void processCmd(mg_connection* conn, char* cmd, size_t cmdLen);
}

#endif
