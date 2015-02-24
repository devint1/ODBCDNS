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

#include <map>
#include <string>
#include "log4cxx\logger.h"
#include "DNSMessage.h"
#include "DNSRData.h"

using std::map;
using std::string;
using namespace log4cxx;

enum TYPE : uint16_t;
enum CLASS : uint16_t;

struct DNSPacket;

struct FixedRRInfo {
	TYPE type;
	CLASS rclass;
	uint32_t ttl;
	uint16_t rdlength;
};

class DNSResourceRecord {
	LoggerPtr logger;
	char *name;
	FixedRRInfo info;
	DNSRData *rdata;
	bool decoded = false;
public:
	DNSResourceRecord(char *name, FixedRRInfo info, DNSRData *rdata);
	~DNSResourceRecord();

	// Encodes the resource record to the output buffer with given length.
	// Uses pointer locations to enable DNS compression.
	// Returns the number of bytes that were written to the buffer.
	size_t encode(__out char *outputBuffer, __in size_t bufLen, __in map<string, uint16_t> &pointerLocations);

	char* getName() { return name; }
	TYPE getType() { return info.type; }
	CLASS getClass() { return info.rclass; }
	int32_t getTTL() { return info.ttl; }
	uint16_t getRDLength() { return info.rdlength; }
	DNSRData* getRData() { return rdata; }

	// Decodes a resource record from the buffer with given length. Outputs
	// a pointer to the buffer after the record.
	static DNSResourceRecord decode(__in char *buffer, __in DNSPacket *packet, __in size_t bufLen, __out char **bufferAfterRecord);
};
