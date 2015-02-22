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
#include <exception>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using std::exception;

namespace DNSTest
{
	TEST_CLASS(DNSRdataATest) {
	public:
		// Check a normal, nothing-special query
		TEST_METHOD(EncodeIPV4) {
			DNSRDataA rdata("127.0.0.1");
			char addr[4];
			rdata.encode(addr, 4);
			Assert::AreEqual("\x7f\x00\x00\x01", (char*)&addr);
		}
		TEST_METHOD(DecodeIPV4) {
			char buf[5] = "\x7f\x7f\x7f\x7f";
			DNSRDataA rdata = DNSRDataA::decode(buf, 4);
			Assert::AreEqual("127.127.127.127", rdata.getValue());
		}
	};
}
