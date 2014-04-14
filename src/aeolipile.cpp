#undef UNICODE                    // Use ANSI WinAPI functions
#undef _UNICODE                   // Use multibyte encoding on Windows
#define _MBCS                     // Use multibyte encoding on Windows
#define _WIN32_WINNT 0x500        // Enable MIIM_BITMAP
#define _CRT_SECURE_NO_WARNINGS   // Disable deprecation warning in VS2005
#define _XOPEN_SOURCE 600         // For PATH_MAX on linux
#undef WIN32_LEAN_AND_MEAN        // Let windows.h always include winsock2.h

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdarg.h>
#include <ctype.h>
#include <time.h>

#include <mongoose.h>
#include <keyPress.h>
#include <webSockCmd.h>

#include <windows.h>
#include <direct.h>  // For chdir()
#include <winsvc.h>
#include <shlobj.h>

#ifndef PATH_MAX
   #define PATH_MAX MAX_PATH
#endif

#ifndef S_ISDIR
   #define S_ISDIR(x) ((x) & _S_IFDIR)
#endif

#define DIRSEP '\\'
#define snprintf _snprintf
#define getcwd _getcwd
#define chdir _chdir
#define abs_path(rel, abs, abs_size) _fullpath((abs), (rel), (abs_size))

static int serverExiting;

static mg_server* server;
static mg_server* server_ws;

static const char* defaultDocRoot = "./www";
static const char* defaultPort = "1979";

static void __cdecl signal_handler(int sig_num) {
   signal(sig_num, signal_handler);
   serverExiting = 1;
}

static void setDocRoot(mg_server* server, const char *path_to_mongoose_exe) {
   char path[PATH_MAX], abs[PATH_MAX];
   const char *p;

   p = strrchr(path_to_mongoose_exe, DIRSEP);
   if (!p) {
      getcwd(path, sizeof(path));
   }
   else {
      snprintf(path, sizeof(path), "%.*s", (int) (p - path_to_mongoose_exe),
		  path_to_mongoose_exe);
   }

   strncat(path, "/", sizeof(path) - 1);
   strncat(path, defaultDocRoot, sizeof(path) - 1);

   // Absolutize the path, and set the option
   abs_path(path, abs, sizeof(abs));
   mg_set_option(server, "document_root", abs);
}

static int send_reply(mg_connection *conn) {
   char buffer[256];
   char firstChar;
  
   if (conn->is_websocket) {
      if (conn->content_len == 0) {
	 return MG_TRUE;
      }
      else if (conn->content_len > (sizeof(buffer)-2)) {
	 // cmd too big
	 printf("ERROR: cmd too large (%i bytes)\n", conn->content_len);
	 return MG_TRUE;
      }

      firstChar = conn->content[0];

      if ((firstChar < 0x20) || (firstChar > 0x7E)) {
	 // Weird char, probably not a normal request
	 return MG_TRUE;
      }

      // Null terminate cmd
      memcpy(buffer, conn->content, conn->content_len);
      buffer[conn->content_len] = '\0';
      buffer[conn->content_len+1] = END_OF_BUF_CHAR;

      processCmd(conn, buffer);
      return MG_TRUE;
   }
   else {
      return MG_FALSE;
   }
}

static int ev_handler(mg_connection *conn, enum mg_event ev) {
  if (ev == MG_REQUEST) {
    return send_reply(conn);
  }
  else if (ev == MG_AUTH) {
    return MG_TRUE;
  }
  return MG_FALSE;
}

static void start_mongoose(int /*argc*/, char *argv[]) {
   server = mg_create_server(NULL, NULL);
   if (!server) {
      fprintf(stderr, "Failed to start Aeolipile\n");
      exit(1);
   }

   setDocRoot(server, argv[0]);
   mg_set_option(server, "listening_port", defaultPort);

   // Change current working directory to document root. This way,
   // scripts can use relative paths.
   chdir(mg_get_option(server, "document_root"));
}

static void start_mongoose_ws() {
   server_ws = mg_create_server(NULL, ev_handler);
   if (!server_ws) {
      fprintf(stderr, "Failed to start WebSocket server\n");
      exit(1);
   }
   mg_set_option(server_ws, "listening_port", "8080");
}

int wsExit;

DWORD wsThreadMain(void* /*param*/) {
  while (!wsExit) {
    mg_poll_server(server_ws, 500);
  }
  return 0;
}

int main(int argc, char *argv[]) {

   signal(SIGTERM, signal_handler);
   signal(SIGINT, signal_handler);
   
   HANDLE wsThread;
   wsExit = 0;
   serverExiting = 0;
  
   keypress::globalInit();

   start_mongoose(argc, argv);
   start_mongoose_ws();

   printf("Aeolipile serving [%s] on port %s\n",
	  mg_get_option(server, "document_root"),
	  mg_get_option(server, "listening_port"));
  
   wsThread = CreateThread(NULL, 0, wsThreadMain, NULL, 0, NULL);

   while (!serverExiting) {
      mg_poll_server(server, 500);
   }
   printf("Exiting...");

   mg_destroy_server(&server);

   wsExit = 1;
   WaitForSingleObject(wsThread, INFINITE);  
  
   mg_destroy_server(&server_ws);
   puts(" done.");

   keypress::globalDestroy();

   return EXIT_SUCCESS;
}
