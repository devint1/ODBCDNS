/*

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

#include <WinSock2.h>
#include <WS2tcpip.h>
#include "..\DNS\DNSResolver.h"
#include <exception>
#include <stdio.h>
#include "DNSServer.h"
#include "Errors.h"

using std::exception;

DWORD WINAPI requestThreadEntry(LPVOID serverPtr) {
	DNSServer *server = (DNSServer*)serverPtr;
	server->serviceRequests();
	return 0;
}

DNSServer::DNSServer(int port, int numThreads, char *recursiveServer,
	SQLWCHAR *connectionString) : numThreads(numThreads),
	connectionString(connectionString), logger(Logger::getLogger("DNSServer")) {
	// Set up recursive server
	this->recursiveServer.sin_family = AF_INET;
	this->recursiveServer.sin_port = htons(port);
	inet_pton(AF_INET, recursiveServer, &this->recursiveServer.sin_addr.s_addr);

	// Allocate recieve buffer
	recvBuf = new char[MAX_DNS_PACKET_LENGTH];

	// Set up threads
	threads = new HANDLE[numThreads];
	requestSemaphore = CreateSemaphore(NULL, 0, (LONG)min(LONG_MAX, requestQueue._Get_container().max_size()), NULL);
	quitEvent = CreateEvent(NULL, true, false, NULL);
	InitializeCriticalSection(&queueCS);
	for (int i = 0; i < numThreads; ++i) {
		threads[i] = CreateThread(NULL, 0, requestThreadEntry, this, 0, NULL);
	}

	// Create the socket
	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock == INVALID_SOCKET) {
		LOG4CXX_FATAL(logger, "Socket creation failed with error: " << WSAGetLastError());
		throw ERR_SOCKET_CREATE;
	}

	// Where to recieve from
	sockaddr_in addr;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);

	// Bind socket
	int retval = bind(sock, (sockaddr *)&addr, sizeof(addr));
	if (retval == SOCKET_ERROR) {
		LOG4CXX_FATAL(logger, "Bind failed with error: " << WSAGetLastError());
		throw ERR_BIND;
	}

	// Socket read event
	socketEvent = CreateEvent(NULL, false, false, NULL);
	retval = WSAEventSelect(sock, socketEvent, FD_READ);
	if (retval == SOCKET_ERROR) {
		LOG4CXX_FATAL(logger, "Event select failed with error: " << WSAGetLastError());
	}
}

DNSServer::~DNSServer() {
	// Wait for all threads to finish
	int threadsWaiting = numThreads;
	int numWait;
	while (threadsWaiting > 0) {
		if (threadsWaiting > MAXIMUM_WAIT_OBJECTS) {
			numWait = MAXIMUM_WAIT_OBJECTS;
		}
		else {
			numWait = threadsWaiting;
		}
		WaitForMultipleObjects(numWait, &threads[threadsWaiting - numThreads], true, INFINITE);
		threadsWaiting -= numWait;
	}

	// Clear em up!
	delete[] threads;
	delete[] recvBuf;

	int retval = closesocket(sock);
	if (retval == SOCKET_ERROR) {
		LOG4CXX_ERROR(logger, "Close socket failed with error: " << WSAGetLastError());
		throw ERR_SOCKET_CLOSE;
	}
}

void DNSServer::run() {
	int retval; 
	HANDLE events[2];
	events[0] = socketEvent;
	events[1] = quitEvent;
	while (isRunning) {
		try {
			// Wait for either data or quit event
			WaitForMultipleObjects(2, events, false, INFINITE);

			// If we're not running, quit
			if (!isRunning) {
				break;
			}

			// Set up data structures for recv
			memset(recvBuf, 0, MAX_DNS_PACKET_LENGTH);
			DNSRequest request;
			int dnsHeadLen = sizeof(DNSHeader);

			// Recieve data
			retval = recvfrom(sock, recvBuf, MAX_DNS_PACKET_LENGTH, 0, (sockaddr *)&request.from, &request.fromlen);
			if (retval == SOCKET_ERROR) {
				int error = WSAGetLastError();
				
				// Ignore connection reset errors; pretty much useless for UDP
				if (error != 10054) {
					LOG4CXX_ERROR(logger, "Recieve failed with error: " << error);
				}
				continue;
			}
			LOG4CXX_TRACE(logger, "Recieved request");

			// Add to request queue
			// Allocate twice the what is required in case truncation happens
			request.packet = (DNSPacket*)malloc(MAX_DNS_PACKET_LENGTH * 2);
			memcpy_s(request.packet, MAX_DNS_PACKET_LENGTH, recvBuf, retval);
			EnterCriticalSection(&queueCS);
			requestQueue.push(request);
			LeaveCriticalSection(&queueCS);
			ReleaseSemaphore(requestSemaphore, 1, NULL);
			LOG4CXX_TRACE(logger, "Added request to queue");
		}
		catch (int e) {
			LOG4CXX_ERROR(logger, e);
		}
		catch (exception e) {
			LOG4CXX_ERROR(logger, e.what());
		}
	}
	LOG4CXX_TRACE(logger, "Main thread exiting");
}

void DNSServer::shutdown() {
	LOG4CXX_INFO(logger, "Server shutting down...");
	isRunning = false;
	SetEvent(quitEvent);
}

void DNSServer::serviceRequests() {
	DNSRequest request;
	request.packet = NULL;
	exception e;
	ResponseInfo responseInfo;
	bool error = false;
	HANDLE events[2];
	events[0] = requestSemaphore;
	events[1] = quitEvent;
	vector<DNSQuery> queries;
	size_t totalBytes;
	size_t queryBytes;
	int retval;
	char *ptr = NULL;
	char addr[16];

	// Set up resolver
	DNSResolver *resolver;
	try {
		resolver = new DNSResolver(environment, connectionString, &recursiveServer);
	}
	catch (exception e) {
		LOG4CXX_FATAL(logger, e.what());
		exit(ERR_CONNECT_ODBC);
	}
	LOG4CXX_TRACE(logger, "Succesfully started worker thread");

	while (isRunning || requestQueue.size() > 0) {
		try {
			if (!error) {
				// Cache the packet if recursion happened last time
				if (responseInfo.recursionOccurred) {
					LOG4CXX_TRACE(logger, "Caching recursive response");
					resolver->cacheResponse(request.packet->data + queryBytes, request.packet,
						ntohs(request.packet->header.ancount),
						MAX_DNS_PACKET_LENGTH - sizeof(DNSHeader) - queryBytes);
				}
			}

			LOG4CXX_TRACE(logger, "Cleaning thread");
			error = false;
			queries.clear();
			resolver->clean();

			// Clear up stuff
			if (request.packet) {
				free(request.packet);
			}

			// Wait for either a packet or quit signal
			WaitForMultipleObjects(2, events, false, INFINITE);

			// Quit if we're not running any more
			if (!isRunning && requestQueue.size() <= 0) {
				break;
			}

			// Get a packet from the queue
			EnterCriticalSection(&queueCS);
			request = requestQueue.front();
			requestQueue.pop();
			LeaveCriticalSection(&queueCS);
			if (logger->isInfoEnabled()) {
				inet_ntop(AF_INET, &request.from.sin_addr, addr, 16);
				LOG4CXX_INFO(logger, "Got request from " << addr);
			}

			// Try to extract queries
			try {
				LOG4CXX_TRACE(logger, "Extracting queries");
				queryBytes = resolver->extractQueries(request.packet, &queries);
				totalBytes = queryBytes;
			}
			catch (exception ex) {
				request.packet->header.rcode = RCODE_FORMAT_ERROR;
				error = true;
				e = ex;
			}
			catch (...) {
				request.packet->header.rcode = RCODE_FORMAT_ERROR;
				error = true;
				e = exception("Unknown");
			}

			// Try to set up response
			try {
				ptr = request.packet->data + totalBytes;

				LOG4CXX_TRACE(logger, "Resolving request");
				responseInfo = resolver->resolve(queries, ptr, request.packet, sizeof(DNSHeader) + totalBytes);
				if (responseInfo.truncationOccurred) {
					LOG4CXX_WARN(logger, "Truncation occurred");
					request.packet->header.tc = 1;
				}
				totalBytes += responseInfo.length;

				if (!responseInfo.recursionOccurred) {
					LOG4CXX_TRACE(logger, "Recursion did not occur, setting response flags");
					request.packet->header.qr = 1;
					request.packet->header.ra = recursiveServer.sin_addr.s_addr != 4294967295;
					request.packet->header.ancount = htons(responseInfo.answers);
					request.packet->header.nscount = htons(responseInfo.authAnswers);
					request.packet->header.arcount = htons(responseInfo.otherAnswers);
					request.packet->header.rcode = responseInfo.rcode;
				}
			}
			catch (exception ex) {
				request.packet->header.rcode = RCODE_SERVER_FAILURE;
				error = true;
				e = ex;
			}
			catch (int ex) {
				request.packet->header.rcode = RCODE_SERVER_FAILURE;
				error = true;
				char error[6];
				_itoa_s(ex, error, 6, 10);
				e = exception(error);
			}
			catch (...) {
				request.packet->header.rcode = RCODE_SERVER_FAILURE;
				error = true;
				e = exception("Unknown");
			}

			// Send response
			retval = sendto(sock, (char*)request.packet, (int)min(sizeof(DNSHeader)
				+ totalBytes, MAX_DNS_PACKET_LENGTH), 0, (sockaddr *)&request.from,
				request.fromlen);
			if (retval == SOCKET_ERROR) {
				LOG4CXX_ERROR(logger, "Send failed with error: " << WSAGetLastError());
				error = true;
				continue;
			}
			LOG4CXX_TRACE(logger, "Sent response");

			// Rethrow error if it occurred
			if (error) {
				throw e;
			}
		}
		catch (int e) {
			error = true;
			LOG4CXX_ERROR(logger, e);
		}
		catch (exception e) {
			error = true;
			LOG4CXX_ERROR(logger, e.what());
		}
	}
	delete resolver;
	LOG4CXX_TRACE(logger, "Thread exiting");
}
