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

#define MAX_DNS_PACKET_LENGTH 512
#define MAX_DNS_NAME_LENGTH 255

#include <cstdint>
#include <vector>
#include "DNSResourceRecord.h"
#include "DNSQuery.h"

using std::vector;

class DNSQuery;
class DNSResourceRecord;

enum OPCODE : unsigned int {
	OPCODE_QUERY = 0, OPCODE_IQUERY = 1,
	OPCODE_STATUS = 2
};

static const char* const OPCODE_DESCR[] = { "QUERY", "IQUERY", "STATUS" };

enum RCODE : unsigned int {
	RCODE_NO_ERROR = 0, RCODE_FORMAT_ERROR = 1,
	RCODE_SERVER_FAILURE = 2, RCODE_NAME_ERROR = 3, RCODE_NOT_IMPLIMENTED = 4,
	RCODE_REFUSED = 5
};

static const char* const RCODE_DESCR[] = { "NO_ERROR", "FORMAT_ERROR",
"SERVER_FAILURE", "NAME_ERROR", "NOT_IMPLIMENTED", "REFUSED" };

enum TYPE : uint16_t {
	TYPE_A = 1, TYPE_NS = 2, TYPE_MD = 3, TYPE_MF = 4,
	TYPE_CNAME = 5, TYPE_SOA = 6, TYPE_MB = 7, TYPE_MG = 8, TYPE_MR = 9,
	TYPE_NULL = 10, TYPE_WKS = 11, TYPE_PTR = 12, TYPE_HINFO = 13,
	TYPE_MINFO = 14, TYPE_MX = 15, TYPE_TXT = 16, TYPE_AAAA = 28, TYPE_SRV = 33,
	QTYPE_AXFR = 252, QTYPE_MAILB = 253, QTYPE_MAILA = 254, QTYPE_ALL = 255
};

static const char* const TYPE_DESCR[] = { "", "A", "NS", "MD", "MF", "CNAME",
"SOA", "MB", "MG", "MR", "NULL", "WKS", "PTR", "HINFO", "MINFO", "MX", "TXT",
"", "", "", "", "", "", "", "", "", "", "", "AAAA", "", "", "", "", "SRV",
"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
"", "", "", "", "", "", "", "", "", "AXFR", "MAILB", "MAILA", "ALL" };

enum CLASS : uint16_t {
	CLASS_IN = 1, CLASS_CS = 2, CLASS_CH = 3,
	CLASS_HS = 4, QCLASS_ALL = 255
};

static const char* const CLASS_DESCR[] = { "", "IN", "CS", "CH", "HS", "",
"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
"", "", "ALL" };

struct DNSHeader {
	// Will need to be converted net <-> host
	unsigned int id : 16;

	// If we are big endian, arranging these this way will give us n <-> h if
	// cast from recv buffer without doing some kind of bit-shifting
#if BYTE_ORDER == BIG_ENDIAN
	// 8 bits
	unsigned int rd : 1;
	unsigned int tc : 1;
	unsigned int aa : 1;
	OPCODE opcode : 4;
	unsigned int qr : 1;

	// 8 bits
	RCODE rcode : 4;
	unsigned int z : 3;
	unsigned int ra : 1;
#else
	// 8 bits
	unsigned int qr : 1;
	OPCODE opcode : 4;
	unsigned int aa : 1;
	unsigned int tc : 1;
	unsigned int rd : 1;

	// 8 bits
	unsigned int ra : 1;
	unsigned int z : 3;
	RCODE rcode : 4;
#endif

	// These need to be converted from net <-> host order
	unsigned int qdcount : 16;
	unsigned int ancount : 16;
	unsigned int nscount : 16;
	unsigned int arcount : 16;
};

struct DNSPacket {
	DNSHeader header;
	char data[1];
};
