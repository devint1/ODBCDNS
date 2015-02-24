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
#include "DNSMessage.h"
#include "DNSRData.h"

using std::map;
using std::string;

class DNSRDataCNAME : public DNSRData {
	char name[MAX_DNS_NAME_LENGTH + 1];
public:
	DNSRDataCNAME(char *name);
	~DNSRDataCNAME();

	char* getValue() { return name; }

	// Encodes the RDATA record to output buffer of length bufLen. Returns
	// number of bytes written.
	size_t encode(__out char *outputBuf, __in int bufLen);

	static DNSRDataCNAME decode(__in char *buffer, __in DNSPacket *packet, __in int bufLen);
};
