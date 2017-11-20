# NetworksProg4

Patrick Falvey, Matt Flanagan, Michael Farren

Files:
======
chatclient				Executable for the client program
chatclient.cpp				C++ code that runs the client
Makefile (client directory)		Makefile to compile the chatclient.cpp code
README.md				This document listing the files included in our assignment
chatserver				Executable for the server program
chatserver.cpp				C++ code that runs the server
Makefile (server directory)		Makefile to compile the chatserver.cpp code
passwords.txt				Text file of returning users and their passwords


Example Commands:
=================

1) Must Start With Server (from student00)

$ ./chatserver 41039

2) Run clients to connect to the server from different machines

$ ./chatclient student00.cse.nd.edu 41039 patrick
$ ./chatclient student00.cse.nd.edu 41039 mikey
$ ./chatclient student00.cse.nd.edu 41039 matt

