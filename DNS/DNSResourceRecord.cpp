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

#include "DNSRdataA.h"
#include "DNSRDataCNAME.h"
#include "DNSResourceRecord.h"
#include "DNSUtil.h"
#include <exception>
#include <WinSock2.h>

using std::exception;

DNSResourceRecord::DNSResourceRecord(char *name, FixedRRInfo info,
	DNSRData *rdata) : name(name), info(info), rdata(rdata),
	logger(Logger::getLogger("DNSResolver")) {
}

DNSResourceRecord::~DNSResourceRecord() {
	if (decoded) {
		delete[] name;
		delete rdata;
	}
}

size_t DNSResourceRecord::encode(char *outputBuffer, size_t bufLen, map<string, uint16_t> &pointerLocations) {
	char *ptr = outputBuffer;
	__int64 remainingLen = bufLen;

	if (pointerLocations.find(name) != pointerLocations.end()) {
		// We have a pointer, use it
		LOG4CXX_TRACE(logger, "Pointer found");
		uint16_t loc = htons(pointerLocations[name] | 0xC000);
		memcpy_s(ptr, bufLen, &loc, sizeof(uint16_t));
		ptr += sizeof(uint16_t);
		remainingLen -= sizeof(uint16_t);
	}
	else {
		// No pointer, write full name
		LOG4CXX_TRACE(logger, "Encoding full name");
		ptr = outputBuffer + DNSUtil::encodeName(name, outputBuffer, bufLen);
		remainingLen -= (ptr - outputBuffer);
	}

	// Write encoded RDATA
	LOG4CXX_TRACE(logger, "Encoding RDATA");
	size_t rdlength = rdata->encode(ptr + 10, (int)remainingLen - 10);

	// Write encoded info
	LOG4CXX_TRACE(logger, "Writing encoded metadata");
	info.type = (TYPE)htons(info.type);
	info.rclass = (CLASS)htons(info.rclass);
	info.ttl = htonl(info.ttl);
	info.rdlength = htons((u_short)rdlength);
	remainingLen -= rdlength;
	memcpy_s(ptr, remainingLen, &info, 10);

	// Return the length
	remainingLen -= 10;
	return bufLen - remainingLen;
}

DNSResourceRecord DNSResourceRecord::decode(char *buffer, DNSPacket *packet, size_t bufLen, char **bufferAfterRecord) {
	LoggerPtr logger(Logger::getLogger("DNSResourceRecord"));
	char *name = new char[MAX_DNS_NAME_LENGTH + 1];
	bool pointer = false;
	char *srcLoc = buffer;
	LOG4CXX_TRACE(logger, "Checking for pointer");
	if ((buffer[0] & 0xC0) == 0xC0) {
		pointer = true;
		uint16_t ptr = buffer[1] | buffer[0] << 8;
		ptr &= 0x3FFF;
		srcLoc = (char*)packet + ptr;
	}

	if ((srcLoc - (char*)packet) > MAX_DNS_PACKET_LENGTH) {
		throw exception("Tried to access out of packet bounds");
	}

	LOG4CXX_TRACE(logger, "Decoding name");
	size_t nameBytes = DNSUtil::decodeName(srcLoc, packet, name, MAX_DNS_NAME_LENGTH + 1);

	// Parse the qname
	if (pointer) {
		srcLoc = buffer + 2;
	}
	else {
		srcLoc += nameBytes;
	}

	FixedRRInfo info;

	// Get RR info
	LOG4CXX_TRACE(logger, "Getting RR info");
	memcpy_s(&info, sizeof(FixedRRInfo), srcLoc, 10);
	info.type = (TYPE)ntohs(info.type);
	info.rclass = (CLASS)ntohs(info.rclass);
	info.ttl = ntohl(info.ttl);
	info.rdlength = htons(info.rdlength);
	srcLoc += 10;

	// Get RDATA
	LOG4CXX_TRACE(logger, "Getting RDATA");
	DNSRData *rdata = NULL;
	switch (info.type) {
	case TYPE_A:
		rdata = new DNSRDataA(DNSRDataA::decode(srcLoc, info.rdlength));
		break;
	case TYPE_CNAME:
		rdata = new DNSRDataCNAME(DNSRDataCNAME::decode(srcLoc, packet, info.rdlength));
		break;
	}

	// Set the buffer after the record
	*bufferAfterRecord = srcLoc + info.rdlength;

	// Return decoded resource record
	LOG4CXX_TRACE(logger, "Constructing RR");
	DNSResourceRecord *rr = new DNSResourceRecord(name, info, rdata);
	rr->decoded = true;
	return *rr;
}
