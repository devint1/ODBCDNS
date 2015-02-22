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

#include <stdint.h>
#include "DNSRData.h"

class DNSRDataA : public DNSRData {
	unsigned char addr[4];
	char value[16];
public:
	DNSRDataA() {}
	DNSRDataA(char *addrStr);
	~DNSRDataA();

	// Encodes the RDATA record to output buffer of length bufLen. Returns
	// number of bytes written.
	size_t encode(__out char *outputBuf, __in int bufLen);

	char *getValue() { return value; }

	static DNSRDataA decode(__in char *buffer, __in int bufLen);
};
