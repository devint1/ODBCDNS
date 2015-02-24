/*

Copyright (c) 2015 Devin Tuchsen. All rights reserved.

This file is part of ODBCDNS.

ODBCDNS is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

ODBCDNS is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with ODBCDNS.  If not, see <http://www.gnu.org/licenses/>.

*/

#pragma once

#include <queue>
#include "log4cxx\logger.h"
#include "..\DNS\DNSMessage.h"
#include "..\ODBC\SQLEnvironment.h"

using std::queue;
using namespace log4cxx;

class SQLEnvironment;

struct DNSRequest {
	DNSPacket *packet;
	sockaddr_in from;
	int fromlen = (int)sizeof(sockaddr_in);
};

class DNSServer {
	LoggerPtr logger;
	int numThreads;
	HANDLE *threads;
	SOCKET sock;
	queue<DNSRequest> requestQueue;
	HANDLE requestSemaphore;
	HANDLE quitEvent;
	HANDLE socketEvent;
	CRITICAL_SECTION queueCS;
	SQLEnvironment environment;
	sockaddr_in recursiveServer;
	SQLWCHAR *connectionString;
	char *recvBuf;
	bool isRunning = true;
public:
	DNSServer(int port, int threads, char *recursiveServer, SQLWCHAR *connectionString);
	~DNSServer();

	// Starts the server
	void run();

	// Shuts down the server cleanly
	void shutdown();

	// Worker thread function to service requests
	void serviceRequests();
};
