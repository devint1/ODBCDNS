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

#include "stdafx.h"
#include "CppUnitTest.h"
#include "..\DNS\DNSRdataA.h"
#include "..\DNS\DNSResourceRecord.h"
#include <exception>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using std::exception;

namespace DNSTest
{
	TEST_CLASS(DNSResourceRecordTest) {
	public:
		// Check a normal, nothing-special query
		TEST_METHOD(EncodeResourceRecord) {
			char *responseData = new char[500];
			DNSRDataA *rdata = new DNSRDataA("23.11.60.133");
			FixedRRInfo info;
			info.rclass = CLASS_IN;
			info.ttl = INT32_MAX;
			info.type = TYPE_A;
			DNSResourceRecord rr("www.apple.com", info, rdata);
			map<string, uint16_t> pointerLocations;
			rr.encode(responseData, MAX_DNS_PACKET_LENGTH - sizeof(DNSHeader), pointerLocations);
			Assert::AreEqual("\x03www\x05apple\x03com\x00\x00\x01\x00\x01\x7F\xFF\xFF\xFF\x00\x04\x17\x0B\x3C\x85", responseData);
		}
	};
}