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
#include "..\DNS\DNSUtil.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace DNSTest
{
	TEST_CLASS(DNSUtilTest) {
	public:
		// Check a normal, nothing-special query
		TEST_METHOD(EncodeName) {
			char *name = "www.google.com";
			char *encodedName = new char[256];
			size_t bytesWritten = DNSUtil::encodeName(name, encodedName, 256);
			Assert::AreEqual("\x03www\x06google\003com", encodedName);
			Assert::AreEqual(strlen(encodedName) + 1, bytesWritten);
		}
	};
}
