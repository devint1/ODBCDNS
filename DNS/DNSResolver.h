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

#pragma once

#include <map>
#include <string>
#include "log4cxx\logger.h"
#include "..\ODBC\SQLConnection.h"
#include "DNSQuery.h"

using std::map;
using std::string;
using namespace log4cxx;

struct ResponseInfo {
	u_short answers = 0;
	u_short authAnswers = 0;
	u_short otherAnswers = 0;
	RCODE rcode = RCODE_NO_ERROR;
	size_t length;
	bool recursionOccurred = false;
	bool truncationOccurred = false;
};

class DNSResolver {
	LoggerPtr logger;
	SQLConnection connection;
	SOCKET sock;
	fd_set fd;
	const sockaddr_in *recursiveServer;
	map<string, uint16_t> pointerLocations;

	// Resolves "A" queries
	size_t resolveA(DNSQuery& query, char *responseData, ResponseInfo *responseInfo, DNSPacket *packet, size_t packetLen);

	// Forwards the packet to the recursive DNS server and writes the recieved packet to buffer.
	// Returns the number of additional bytes included in the output packet compared to input
	size_t recursiveResolve(__in DNSPacket *inputPacket, __out DNSPacket *outputPacket, __in size_t packetLen);
public:
	DNSResolver(const SQLEnvironment &environment, SQLWCHAR *connString, const sockaddr_in *recursiveServer);
	~DNSResolver();

	// Resolves a number of queries and provides the response packet data to a
	// buffer. Uses original packet to determine pointer locations and to do 
	// recursive queries. Returns a ResponseInfo struct that contains the
	// number of responses that were generated of each type.
	ResponseInfo resolve(__in const vector<DNSQuery> &queries, __out char *responseData, __in DNSPacket *packet, __in size_t packetLen);

	// Extracts queries from a DNS packet. Provides parsed queries as a vector
	// of DNSQuery objects (PROVIDED VECTOR MUST BE EMPTY UPON ENTERING). 
	// Return value is the number of query bytes read from the packet.
	size_t extractQueries(__in const DNSPacket *packet, __out vector<DNSQuery> *outputQueries);

	// Caches the provided response data. Best used when caching packets recieved
	// from the recursive server.
	void cacheResponse(__in char *data, DNSPacket *packet, __in int numResponses, __in size_t bufLen);

	// Cleans up the resolver, namely, empties the pointer locations.
	void clean();
};
