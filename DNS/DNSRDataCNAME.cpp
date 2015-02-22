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

#include "DNSRDataCNAME.h"
#include "DNSUtil.h"

DNSRDataCNAME::DNSRDataCNAME(char *name) {
	strcpy_s(this->name, name);
}

DNSRDataCNAME::~DNSRDataCNAME() {
}

size_t DNSRDataCNAME::encode(char *outputBuf, int bufLen) {
	return DNSUtil::encodeName(name, outputBuf, bufLen);
}

DNSRDataCNAME DNSRDataCNAME::decode(char *buffer, DNSPacket *packet, int bufLen) {
	char *name = new char[MAX_DNS_NAME_LENGTH + 1];
	bool pointer = false;
	char *srcLoc = buffer;
	if ((buffer[0] & 0xC0) == 0xC0) {
		pointer = true;
		uint16_t ptr = buffer[1] | buffer[0] << 8;
		ptr &= 0x3FFF;
		srcLoc = (char*)packet + ptr;
	}
	DNSUtil::decodeName(srcLoc, packet, name, MAX_DNS_NAME_LENGTH + 1);
	DNSRDataCNAME *rdata = new DNSRDataCNAME(name);
	return *rdata;
}
