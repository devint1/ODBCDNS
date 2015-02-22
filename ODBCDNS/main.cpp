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

#include <stdio.h>
#include <WS2tcpip.h>
#include "log4cxx\logger.h"
#include "log4cxx\basicconfigurator.h"
#include "log4cxx\propertyconfigurator.h"
#include "DNSServer.h"
#include "Errors.h"

using namespace log4cxx;

LoggerPtr logger(Logger::getLogger("main"));

// Arguments
SQLWCHAR *connectionString;
char *logPropertiesPath = NULL;
char recursiveServer[16];
int port = 53;
int numThreads = 64;

void printUsage(char *appName) {
	printf("usage:\n\t%s odbcconnectionstring [-option ...]\n\n", appName);
	printf("options:\n\trecursiveserver=SERVER\t- server to use for recursive queries\n");
	printf("\tport=PORT\t\t- port to listen to\n");
	printf("\tthreads=THREADS\t\t- number of threads and ODBC connections to use\n");
	printf("\tlogproperties=FILE\t- path to log4cxx properties file\n");
	exit(0);
}

void parseArgs(int argc, char **argv) {
	if (argc < 2) {
		printUsage(argv[0]);
	}

	// Connection string
	size_t len = strlen(argv[1]) + 1;
	size_t convertedChars = 0;
	connectionString = new SQLWCHAR[len];
	mbstowcs_s(&convertedChars, connectionString, len, argv[1], _TRUNCATE);

	for (int i = 2; i < argc; ++i) {
		char *arg = argv[i];
		char *dash = strchr(arg, '-');
		char *equals = strchr(arg, '=');
		if (!dash || !equals) {
			printf("ERROR: Invalid command line arguments\n");
			printUsage(argv[0]);
		}

		// Helpers for strncmp
		char *option = arg + 1;
		__int64 numChars = equals - arg - 1;

		// Recursive server
		if (!strncmp("recursiveserver", option, numChars)) {
			errno_t err = strcpy_s(recursiveServer, equals + 1);
			if (err) {
				printf("ERROR: Invalid server address: %s", equals + 1);
			}
		}

		// Port number
		else if (!strncmp("port", option, numChars)) {
			port = atoi(equals + 1);
			if (port <= 0) {
				printf("ERROR: Invalid port number: %d\n", port);
				printUsage(argv[0]);
			}
		}

		// NumThrreads
		else if (!strncmp("threads", option, numChars)) {
			numThreads = atoi(equals + 1);
			if (port <= 0) {
				printf("ERROR: Invalid number of threads: %d\n", numThreads);
				printUsage(argv[0]);
			}
		}

		// Log4cxx properties
		else if (!strncmp("logproperties", option, numChars)) {
			len = strlen(equals + 1) + 1;
			logPropertiesPath = new char[len];
			strcpy_s(logPropertiesPath, len, equals + 1);
		}

		// Something we don't do
		else {
			printf("ERROR: Invalid option: %.*s\n\n", numChars, option);
			printUsage(argv[0]);
		}
	}
}

DNSServer *server;

BOOL WINAPI consoleCtrlEntry(DWORD ctrlType) {
	server->shutdown();
	return true;
}

int main(int argc, char **argv) {
	try {
		parseArgs(argc, argv);

		if (logPropertiesPath) {
			PropertyConfigurator::configure(logPropertiesPath);
		}
		else {
			BasicConfigurator::configure();
		}

		LOG4CXX_INFO(logger, "Program started.");

		// Start up Winsock
		WSADATA wsaData;
		int retval = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (retval != 0) {
			printf("WSAStartup failed with error: %d\n", retval);
			return ERR_WSASTARTUP;
		}

		// Start up server
		server = new DNSServer(port, numThreads, recursiveServer, connectionString);
		SetConsoleCtrlHandler(consoleCtrlEntry, true);
		server->run();

		delete[] connectionString;
		delete[] logPropertiesPath;
		delete server;
		WSACleanup();
		return 0;
	}
	catch (int ex) {
		return ex;
	}
	catch (...) {
		return -1;
	}
}
