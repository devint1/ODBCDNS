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
#include "DNSQuery.h"
#include "DNSUtil.h"

using std::exception;

// FIXME: Need to take another look at the pointer logic
DNSQuery::DNSQuery(char *start, char *message) : logger(Logger::getLogger("DNSQuery")) {
	// Check for pointer
	char *srcLoc = start;
	char pointer = false;
	LOG4CXX_TRACE(logger, "Checking for pointer");
	if ((start[0] & 0xC0) == 0xC0) {
		pointer = true;
		uint16_t ptr = start[1] | start[0] << 8;
		ptr &= 0x3FFF;
		srcLoc = message + ptr;
	}

	// Parse the qname
	LOG4CXX_TRACE(logger, "Decoding name");
	srcLoc += DNSUtil::decodeName(srcLoc, (DNSPacket*)message, qname, MAX_DNS_NAME_LENGTH + 1);

	// Get type and class
	if (pointer) {
		qtype = (TYPE)*(start + 3);
		qclass = (CLASS)*(start + 5);
		length = 6;
	}
	else {
		qtype = (TYPE)*(srcLoc + 1);
		qclass = (CLASS)*(srcLoc + 3);
		length = srcLoc - start + 4;
	}
}

DNSQuery::~DNSQuery()
{
}
