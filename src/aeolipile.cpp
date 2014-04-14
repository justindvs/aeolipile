// Windows complains about CRT functions being deprecated
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

// Windows stuff
#include <windows.h>
#include <direct.h> // _chdir, _getcwd

// Aeolipile stuff
#include <mongoose.h>
#include <keyPress.h>
#include <webSockCmd.h>

#define DIRSEP '\\'
#define snprintf _snprintf
#define getcwd _getcwd
#define chdir _chdir
#define abs_path(rel, abs, abs_size) _fullpath((abs), (rel), (abs_size))

int serverExiting;

mg_server* server;
mg_server* webSocketServer;

const char* docRoot = "./www";
const char* webServerPort = "1979";

void __cdecl signalHandler(int sig_num) {
   signal(sig_num, signalHandler);
   serverExiting = 1;
}

static void setDocRoot(mg_server* server, const char *path_to_mongoose_exe) {
   char path[MAX_PATH], abs[MAX_PATH];
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
   strncat(path, docRoot, sizeof(path) - 1);

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

      processCmd(conn, buffer, conn->content_len);
      return MG_TRUE;
   }
   else {
      return MG_FALSE;
   }
}

static int webSocketEventHandler(mg_connection *conn, enum mg_event ev) {
  if (ev == MG_REQUEST) {
    return send_reply(conn);
  }
  else if (ev == MG_AUTH) {
    return MG_TRUE;
  }
  return MG_FALSE;
}

static void start_mongoose(const char* serverBin) {
   server = mg_create_server(NULL, NULL);
   if (!server) {
      fprintf(stderr, "Failed to start Aeolipile\n");
      exit(1);
   }

   setDocRoot(server, serverBin);
   mg_set_option(server, "listening_port", webServerPort);

   chdir(mg_get_option(server, "document_root"));
}

static void start_mongoose_ws() {
   webSocketServer = mg_create_server(NULL, webSocketEventHandler);
   if (!webSocketServer) {
      fprintf(stderr, "Failed to start WebSocket server\n");
      exit(1);
   }
   // TODO: Allow use of a different web socket port
   mg_set_option(webSocketServer, "listening_port", "8080");
}

DWORD websockThreadMain(void* /*param*/) {
  while (!serverExiting) {
    mg_poll_server(webSocketServer, 500);
  }
  return 0;
}

void printUsageAndExit() {
   puts("USAGE: aeolipile [options]");
   puts("  options:");
   puts("    --help|-h         Print this message");
   puts("    --port|-p PORT    Set the web server port (defaults to 1979)");
   exit(1);
}

void processArgs(char* argv[]) {
   // TODO: Use a proper argument parsing library (like getopt)
   char** currentArg = &argv[1];

   while (*currentArg) {
      if ((strcmp("--help", *currentArg) == 0) ||
	  (strcmp("-h", *currentArg) == 0)) {
	 printUsageAndExit();
      }
      else if ((strcmp("--port", *currentArg) == 0) ||
	       (strcmp("-p", *currentArg) == 0)) {
	 // Make sure an argument was supplied
	 ++currentArg;
	 if (!*currentArg) printUsageAndExit();
	 webServerPort = *currentArg;
      }
      ++currentArg;
   }
}

int main(int /*argc*/, char *argv[]) {
   processArgs(argv);
   
   // Setup signal handlers so the server can be cleanly killed with a ctrl-c.
   // TODO: Do we need to handle both of these signals?
   signal(SIGTERM, signalHandler);
   signal(SIGINT, signalHandler);
   
   HANDLE websockThread;
   serverExiting = 0;
  
   keypress::globalInit();

   start_mongoose(argv[0]);
   start_mongoose_ws();

   printf("Aeolipile serving [%s] on port %s\n",
	  mg_get_option(server, "document_root"),
	  mg_get_option(server, "listening_port"));
  
   websockThread = CreateThread(NULL, 0, websockThreadMain, NULL, 0, NULL);

   while (!serverExiting) {
      mg_poll_server(server, 500);
   }

   printf("Exiting...");

   mg_destroy_server(&server);
   WaitForSingleObject(websockThread, INFINITE);  
   mg_destroy_server(&webSocketServer);

   puts(" done.");

   keypress::globalDestroy();
   return 0;
}
