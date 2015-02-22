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

#include <exception>
#include <WinSock2.h>
#include "DNSRdataA.h"

using std::exception;

DNSRDataA::DNSRDataA(char *addrStr) {
	strcpy_s(value, addrStr);
	char *ptr = addrStr;
	char *next;
	char temp[4];
	__int64 numChars = 0;
	for (int i = 0; i < 4; ++i) {
		next = strchr(ptr, '.');
		if (!next) {
			next = strchr(ptr, '\0');
		}
		numChars = next - ptr;
		memcpy_s(temp, 4, ptr, numChars);
		temp[numChars] = '\0';
		addr[i] = '\0';
		addr[i] |= atoi(temp);
		ptr += numChars + 1;
	}
}

DNSRDataA::~DNSRDataA() {
}

size_t DNSRDataA::encode(char *outputBuf, int bufLen) {
	memcpy_s(outputBuf, bufLen, &addr, 4);
	return 4;
}

DNSRDataA DNSRDataA::decode(__in char *buffer, __in int bufLen) {
	DNSRDataA *rdata = new DNSRDataA();
	memcpy_s(rdata->addr, 4, buffer, bufLen);
	rdata->value[0] = '\0';
	char *ptr = &rdata->value[0];
	size_t remLen = 16;
	for (int i = 0; i < 4; ++i) {
		_itoa_s(rdata->addr[i], ptr, remLen, 10);
		if (rdata->addr[i] > 99) {
			remLen -= 3;
		}
		else if (rdata->addr[i] > 9) {
			remLen -= 2;
		}
		else {
			--remLen;
		}
		if (i < 3) {
			strcat_s(ptr, remLen, ".");
			--remLen;
		}
		ptr = &rdata->value[16 - remLen];
	}
	ptr[0] = '\0';
	return *rdata;
}
