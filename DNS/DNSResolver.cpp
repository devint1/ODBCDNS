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

#include <exception>
#include <Winsock2.h>
#include "DNSRdataA.h"
#include "DNSRDataCNAME.h"
#include "DNSResolver.h"
#include "..\ODBC\SQLStatement.h"

using std::exception;

bool validateDescrArrays(OPCODE opcode, RCODE rcode, TYPE type, CLASS rclass) {
	return opcode >= 0 && opcode < 3 &&
		rcode >= 0 && rcode < 6 &&
		type >= 0 && type < 256 &&
		rclass >= 0 && rclass < 256;
}

DNSResolver::DNSResolver(const SQLEnvironment &environment, SQLWCHAR *connString, const sockaddr_in *recursiveServer)
	: connection(environment), recursiveServer(recursiveServer), logger(Logger::getLogger("DNSResolver")) {
	connection.connect(connString);
	this->sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	FD_ZERO(&fd);
	FD_SET(this->sock, &fd);
}

DNSResolver::~DNSResolver() {

}

size_t DNSResolver::resolveA(DNSQuery &query, char *responseData, ResponseInfo *responseInfo, DNSPacket *packet, size_t packetLen) {
	LOG4CXX_TRACE(logger, "Resolving A record");
	char *ptr = responseData;
	DNSRData *rdata = NULL;
	size_t readBytes;
	int remLen = MAX_DNS_PACKET_LENGTH - sizeof(DNSHeader);
	int numARecords = 0;

	// Vars for SQL
	SQLStatement statement(connection);
	SQLWCHAR *sqlQuery = L"WITH RESULTS(NAME, VALUE, EXPIRES, TYPE, CLASS) AS ("
		L"SELECT NAME, VALUE, EXPIRES, TYPE, CLASS "
		L"FROM RECORD "
		L"WHERE NAME = ? AND EXPIRES > {fn NOW()} "
		L"UNION ALL "
		L"SELECT RECORD.NAME, RECORD.VALUE, RECORD.EXPIRES, RECORD.TYPE, RECORD.CLASS "
		L"FROM RECORD "
		L"JOIN RESULTS "
		L"ON RECORD.NAME = RESULTS.VALUE AND RECORD.TYPE IN(5, 1) "
		L"AND RECORD.EXPIRES > {fn NOW()} " 
		L") SELECT { fn TIMESTAMPDIFF(SQL_TSI_SECOND, { fn NOW() }, "
		L"EXPIRES) } TTL, NAME, VALUE, TYPE, CLASS FROM RESULTS";
	SQLCHAR nameBuf[256];
	SQLCHAR valueBuf[256];
	TYPE typeBuf;
	CLASS classBuf;
	SQLINTEGER ttlBuf;

	// Bind name parameter
	LOG4CXX_TRACE(logger, "Binding SQL parameters for resolve");
	char name[MAX_DNS_NAME_LENGTH];
	strcpy_s(name, query.getQname());
	statement.bindString(1, MAX_DNS_NAME_LENGTH, name);

	// Prepare statement
	LOG4CXX_TRACE(logger, "Preparing SQL statement for resolve");
	statement.prepare(sqlQuery);

	// Execute
	LOG4CXX_TRACE(logger, "Executing SQL statement for resolve");
	SQLResultSet resultSet = statement.executeQuery(false);

	// Read results
	while (resultSet.next()) {
		LOG4CXX_TRACE(logger, "Got SQL data for resolve");
		resultSet.getLong(L"TTL", &ttlBuf);
		resultSet.getString(L"NAME", nameBuf);
		resultSet.getString(L"VALUE", valueBuf);
		resultSet.getUShort(L"TYPE", (SQLUSMALLINT*)&typeBuf);
		resultSet.getUShort(L"CLASS", (SQLUSMALLINT*)&classBuf);

		// Create the RDATA part of the record
		switch (typeBuf) {
		case TYPE_A:
			rdata = new DNSRDataA((char*)valueBuf);
			++numARecords;
			break;
		case TYPE_CNAME:
			rdata = new DNSRDataCNAME((char*)valueBuf);
			break;
		}
		FixedRRInfo info;

		// Get CLASS, TTL, and TYPE
		info.rclass = classBuf;
		info.ttl = ttlBuf;
		info.type = typeBuf;

		// Construct and encode resource record
		DNSResourceRecord rr((char*)nameBuf, info, rdata);
		if (logger->isInfoEnabled() && validateDescrArrays(OPCODE_QUERY, RCODE_NO_ERROR, rr.getType(), rr.getClass())) {
			LOG4CXX_INFO(logger, "Encoding RR: " << rr.getName() << ", type = "
				<< TYPE_DESCR[rr.getType()] << ", class = " << CLASS_DESCR[rr.getClass()] <<
				", ttl = " << rr.getTTL() << ", value = " << rdata->getValue());
		}
		readBytes = rr.encode(ptr, remLen, pointerLocations);

		// Save pointers for CNAME records
		LOG4CXX_TRACE(logger, "Saving pointers");
		if (typeBuf == TYPE_CNAME && pointerLocations.find((char*)valueBuf) == pointerLocations.end()) {
			pointerLocations[(char*)valueBuf] = (uint16_t)((ptr - (char*)packet) + readBytes - strlen((char*)valueBuf) - 2);
		}

		// Check for truncation
		LOG4CXX_TRACE(logger, "Checking truncation");
		remLen -= (int)(readBytes);
		if (remLen <= 0) {
			responseInfo->truncationOccurred = true;
			--responseInfo->answers;
			delete rdata;
			statement.commit();
			return ptr - responseData;
		}

		// Incriment and cleanup
		ptr += readBytes;
		++responseInfo->answers;
		delete rdata;
	}

	if (numARecords <= 0 && packet->header.rd) {
		responseInfo->recursionOccurred = true;
		statement.commit();
		LOG4CXX_TRACE(logger, "No A records, cache miss");
		return recursiveResolve(packet, packet, packetLen);
	}
	
	LOG4CXX_INFO(logger, "Cache hit");
	statement.commit();
	return ptr - responseData;
}

ResponseInfo DNSResolver::resolve(const vector<DNSQuery> &queries, char *responseData, DNSPacket *packet, size_t packetLen) {
	ResponseInfo responseInfo;
	for (DNSQuery query : queries) {
		switch (query.getQtype()) {
		case TYPE_A:
			responseInfo.length = resolveA(query, responseData, &responseInfo, packet, packetLen);
			break;
		default:
			if (packet->header.rd) {
				LOG4CXX_TRACE(logger, "Query type not supported, cache miss");
				responseInfo.length = recursiveResolve(packet, packet, packetLen);
				responseInfo.recursionOccurred = true;
			}
			else {
				LOG4CXX_TRACE(logger, "Query type not supported, no recursion (RCODE 4)");
				responseInfo.rcode = RCODE_NOT_IMPLIMENTED;
			}
			return responseInfo;
		}
	}
	return responseInfo;
}

size_t DNSResolver::recursiveResolve(DNSPacket *inputPacket, DNSPacket *outputPacket, size_t packetLen) {
	if (recursiveServer->sin_addr.s_addr == 4294967295) {
		return packetLen;
	}
	LOG4CXX_INFO(logger, "Cache miss, requesting recursively");
	int len = sizeof(sockaddr_in);
	unsigned int originalId = inputPacket->header.id;
	LOG4CXX_TRACE(logger, "Sending request to recursive server");
	int retval = sendto(sock, (char*)inputPacket, (int)packetLen, 0, (sockaddr*)recursiveServer, len); 
	if (retval == SOCKET_ERROR) {
		throw WSAGetLastError();
	}
	LOG4CXX_TRACE(logger, "Waiting for data from recursive server");
	retval = select(0, &fd, NULL, NULL, NULL);
	if (retval == SOCKET_ERROR) {
		throw WSAGetLastError();
	}
	retval = recv(sock, (char*)outputPacket, MAX_DNS_PACKET_LENGTH, 0);
	if (retval == SOCKET_ERROR) {
		throw WSAGetLastError();
	}
	LOG4CXX_TRACE(logger, "Recieved data from recursive server");
	if (outputPacket->header.id != originalId) {
		throw exception("Recursive response ID mismatch");
	}
	return retval - packetLen;
}

size_t DNSResolver::extractQueries(const DNSPacket *packet, vector<DNSQuery> *outputQueries) {
	int numQueries = ntohs(packet->header.qdcount);
	char *ptr = (char*) &packet->data[0];
	size_t length = 0;
	for (int i = 0; i < numQueries; ++i) {
		DNSQuery query(ptr, (char*)&packet); 
		if (logger->isInfoEnabled() && validateDescrArrays(OPCODE_QUERY, RCODE_NO_ERROR, query.getQtype(), query.getQclass())) {
			LOG4CXX_INFO(logger, "QUERY: " << query.getQname() << ", type = "
				<< TYPE_DESCR[query.getQtype()] << ", class = " << CLASS_DESCR[query.getQclass()]);
		}
		pointerLocations[query.getQname()] = (uint16_t)((char*)(&packet->data[0]) - (char*)packet);
		outputQueries->push_back(query);
		length += query.getLength();
		ptr += query.getLength();
	}
	return length;
}

void DNSResolver::cacheResponse(char *data, DNSPacket *packet, int numResponses, size_t bufLen) {
	char *ptr = data;
	DNSRData *rdata = NULL;

	// SQL stuff
	SQLStatement insertStatement(connection);
	SQLStatement deleteStatement(connection);
	SQLCHAR name[256];
	SQLCHAR value[256];
	SQLINTEGER ttl;
	SQLUSMALLINT type;
	SQLUSMALLINT rclass;
	SQLLEN inserted = 0;
	SQLLEN totalInserted = 0;
	SQLLEN totalDeleted = 0;

	// Delete query
	LOG4CXX_TRACE(logger, "Setting up delete query for cache");
	SQLWCHAR *deleteQuery = L"DELETE FROM RECORD WHERE NAME = ? "
		L"AND EXPIRES <= {fn NOW()} ";
	deleteStatement.bindString(1, 256, name);
	deleteStatement.prepare(deleteQuery);

	// Insert query
	LOG4CXX_TRACE(logger, "Setting up insert query for cache");
	SQLWCHAR *insertQuery = L"IF NOT EXISTS (SELECT TOP 1 1 FROM RECORD WHERE NAME = ? AND "
		L"VALUE = ? AND EXPIRES >= {fn NOW()}) "
		L"BEGIN "
		L"INSERT INTO RECORD(NAME, VALUE, EXPIRES, TYPE, "
		L"CLASS) VALUES(?, ?, { fn TIMESTAMPADD(SQL_TSI_SECOND, ?, "
		L"{ fn NOW() }) }, ?, ?) "
		L"END";
	insertStatement.bindString(1, 256, name);
	insertStatement.bindString(2, 256, value);
	insertStatement.bindString(3, 256, name);
	insertStatement.bindString(4, 256, value);
	insertStatement.bindLong(5, &ttl);
	insertStatement.bindUShort(6, &type);
	insertStatement.bindUShort(7, &rclass);
	insertStatement.prepare(insertQuery);

	for (int i = 0; i < numResponses; ++i) {
		DNSResourceRecord rr = DNSResourceRecord::decode(ptr, packet, bufLen, &ptr);
		LOG4CXX_TRACE(logger, "Got resource record for cache");
		rdata = rr.getRData();
		if (!rdata) {
			continue;
		}
		inserted = 0;
		strcpy_s((char*)name, 256, rr.getName());
		strcpy_s((char*)value, 256, rdata->getValue());
		ttl = rr.getTTL();
		type = rr.getType();
		rclass = rr.getClass();
		LOG4CXX_TRACE(logger, "Executing delete");
		totalDeleted += deleteStatement.executeUpdate(false);
		LOG4CXX_TRACE(logger, "Executing insert");
		inserted = insertStatement.executeUpdate(false);
		totalInserted += inserted;
		if (rdata && inserted) {
			LOG4CXX_INFO(logger, "Cached RR: " << rr.getName() << ", type = "
				<< TYPE_DESCR[rr.getType()] << ", class = " << CLASS_DESCR[rr.getClass()] <<
				", ttl = " << rr.getTTL() << ", value = " << rdata->getValue());
		}
	}
	LOG4CXX_TRACE(logger, "Committing delete");
	deleteStatement.commit();
	LOG4CXX_TRACE(logger, "Committing insert");
	insertStatement.commit();
	LOG4CXX_DEBUG(logger, "Deleted " << totalDeleted << " records, inserted " << totalInserted);
}

void DNSResolver::clean() {
	pointerLocations.clear();
}
