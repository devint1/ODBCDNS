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

#include "stdafx.h"
#include "CppUnitTest.h"
#include "..\DNS\\DNSQuery.h"
#include <exception>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using std::exception;

namespace DNSTest
{
	TEST_CLASS(DNSQueryTest) {
		static DNSHeader header;
		static char *buf;
		TEST_CLASS_INITIALIZE(Initialize) {
			buf = new char[MAX_DNS_PACKET_LENGTH];
			header.id = 1337;
			header.qr = 0;
			header.opcode = OPCODE_QUERY;
			header.aa = 0;
			header.tc = 0;
			header.rd = 1;
			header.ra = 0;
			header.z = 0;
			header.rcode = RCODE_NO_ERROR;
			header.qdcount = 1;
			header.ancount = 0;
			header.nscount = 0;
			header.arcount = 0;
		}
		TEST_CLASS_CLEANUP(Cleanup) {
			delete[] buf;
		}
		static DNSQuery getQuery(char *queryStr, int len) {
			memset(buf, 0, MAX_DNS_PACKET_LENGTH);
			memcpy_s(buf, MAX_DNS_PACKET_LENGTH, &header, sizeof(header));
			memcpy_s(buf + sizeof(header), MAX_DNS_PACKET_LENGTH - sizeof(header), queryStr, len);
			DNSQuery query(buf + sizeof(DNSHeader), buf);
			return query;
		}
		static void invalidLabelLength() {
			DNSQuery query = getQuery("\x63veryveryveryvery"
				"veryveryveryveryveryveryveryveryveryveryveryverylonglabelthat"
				"shouldthrowanexception\000\000\001\000\001", 105);
		}
		static void invalidQNAMELength() {
			DNSQuery query = getQuery("\x0ciamaverylong\x0bmessagethat\x0b"
				"shouldthrow\x0b""anexception\x0b""becauseitis\x0ajustwaytoo\x04"
				"long\x0e""anormalmessage\x0eissupposedtobe\x03""255\x0d"
				"charactersmax\x0c""butthisoneis\x0elongerthanthat\x0b""asyoucansee"
				"\x0e""bylookingatthe\x0c""debuggeritis\x09quiteabit\x0alongerthan"
				"\x03""255\x0a""characters\x08nowiwill\x0a""fillitwith\x09"
				"bogusdata\x00\x00\x01\x00\x01", 264);
		}
	public:
		// Check a normal, nothing-special query
		TEST_METHOD(ValidQuery) {
			DNSQuery query = getQuery("\x03www\x06google\003com\x00\x00\x01\x00\x01", 20);
			Assert::AreEqual("www.google.com", query.getQname());
			Assert::IsTrue(TYPE_A == query.getQtype());
			Assert::IsTrue(CLASS_IN == query.getQclass());
		}

		// Check a query that is the maximum length
		TEST_METHOD(LongValidQuery) {
			char queryStr[260];
			int count = 0;
			for (int i = 0; i < 253; i += 2) {
				queryStr[i] = '\x01';
				queryStr[i + 1] = '1';
				++count;
			}
			queryStr[252] = '\2';
			queryStr[253] = '1';
			queryStr[254] = '1';
			queryStr[255] = '\0';
			queryStr[256] = '\0';
			queryStr[257] = '\1';
			queryStr[258] = '\0';
			queryStr[259] = '\1';
			DNSQuery query = getQuery(queryStr, 260);
			Assert::AreEqual("1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1."
				"1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1"
				".1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1."
				"1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1"
				".1.1.1.1.1.1.1.1.1.1.11", query.getQname());
			Assert::IsTrue(TYPE_A == query.getQtype());
			Assert::IsTrue(CLASS_IN == query.getQclass());
		}

		// Check a query with invalid label lengths
		TEST_METHOD(InvalidLabelLength) {
			Assert::ExpectException<exception>(invalidLabelLength);
		}

		// Check a query with invalid QNAME length
		TEST_METHOD(InvalidQNAMELength) {
			Assert::ExpectException<exception>(invalidQNAMELength);
		}

		// Check pointers
		TEST_METHOD(PointerQuery) {
			char *queryStr = new char[260];
			char *name = (queryStr + 100);
			memcpy_s(queryStr, 6, "\xc0\x70\x00\x01\x00\x01", 6);
			memcpy_s(name, 16, "\x03www\x06google\003com\x00", 16);
			DNSQuery query = getQuery(queryStr, 260);
			Assert::AreEqual("www.google.com", query.getQname());
			Assert::IsTrue(TYPE_A == query.getQtype());
			Assert::IsTrue(CLASS_IN == query.getQclass());
			delete[] queryStr;
		}
	};

	DNSHeader DNSQueryTest::header;
	char *DNSQueryTest::buf;
}