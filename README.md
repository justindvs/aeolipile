# Pyroscaphe: Adding a Touch Screen to PC Gaming

Pyroscaphe is a combination of a web server + some javascript tools to enable the creation of touch screen UIs for PC games.

For a better introduction to Pyroscaphe, see my blog post here: http://jmdavisblog.blogspot.com/2014/04/pyroscaphe.html

## How to Build

In order to build Pyroscaphe, you'll need several tools installed on your machine
* Visual C++ (the free "Express" version works fine)
* A few Cygwin tools (these must be in your PATH)
   * GNU Make
   * cat
   * rm
   * echo

The build process also assumes that you have the Visual Studio tools in your PATH.  The easiest way to do this is to run the build from a "Visual Studio Command Prompt" (can usually be found @ Start->All Programs->Microsoft Visual Studio XXXX->Visual Studio Tools).

From the command prompt, just run "make" to generate a default config.mak file.  This file contains paths that may differ from machine to machine.  Edit this file to reflect the paths on your machine and then run "make" again.  Assuming things build successfully (which should take < 1 minute), just run "make startserver" to launch the server (or just run pyroscaphe.exe).

## How to Run

To run Pyroscaphe, just launch the executable (pyroscaphe.exe).  By default, it hosts the server on port 1979, but you can run it on another port by passing it the appropriate command-line argument (ex. "pyroscaphe --port 1983").

Now, on your mobile device, open up the browser and type this address into the URL bar "http://SERVER_IP:1979".  Obviously, change "SERVER_IP" to match your server's ip address (and change the port if you're not running on the default port).

After doing this, you should be on the main landing page with links to the 2 demos and the latency test page.

When you use one of the demos, you should see the keypresses coming through on your server.  Pyroscaphe currently logs the keypress events to stdout.  You should also be able to open up something like notepad on your server and see the keys being pressed.

Note: If you want to run the games from the demo, you'll have to configure your controller to send keypresses with an application like Joy2Key.

## How to Create a New Game UI

The easiest way to create a new game UI is to copy an existing one into a new directory and start modifying it.  For example, copy www/games/duke to www/games/othergame and start tweaking away.