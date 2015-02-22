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
#include "..\DNS\DNSResolver.h"
#include <exception>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using std::exception;

namespace DNSTest
{
	TEST_CLASS(DNSResolverTest) {
		static DNSPacket *packet;
		static char *buf;
		static SQLEnvironment env;
		TEST_CLASS_INITIALIZE(Initialize) {
			packet = (DNSPacket*) malloc(MAX_DNS_PACKET_LENGTH);
			memset(packet, 0, MAX_DNS_PACKET_LENGTH);
			buf = packet->data;
			packet->header.id = 1337;
			packet->header.qr = 0;
			packet->header.opcode = OPCODE_QUERY;
			packet->header.aa = 0;
			packet->header.tc = 0;
			packet->header.rd = 1;
			packet->header.ra = 0;
			packet->header.z = 0;
			packet->header.rcode = RCODE_NO_ERROR;
			packet->header.qdcount = 1;
			packet->header.ancount = 0;
			packet->header.nscount = 0;
			packet->header.arcount = 0;
		}
		TEST_CLASS_CLEANUP(Cleanup) {
			delete[] buf;
			free(packet);
		}
		TEST_METHOD(ExtractQueries) {
			packet->header.qdcount = htons(2);
			memcpy_s(packet->data, MAX_DNS_PACKET_LENGTH - sizeof(DNSHeader),
				"\x03www\x06google\003com\x00\x00\x01\x00\x01"
				"\x03www\x09microsoft\003com\x00\x00\x01\x00\x01", 43);
			vector<DNSQuery> queries;
			DNSResolver resolver(env, L"DRIVER={SQL Server Native Client 11.0};"
				L"SERVER=(local);DATABASE=DNS;Trusted_Connection=yes;", NULL);
			buf += resolver.extractQueries(packet, &queries);
			Assert::IsTrue(2 == queries.size());
			DNSQuery query1 = queries[0];
			Assert::AreEqual("www.google.com", query1.getQname());
			Assert::IsTrue(TYPE_A == query1.getQtype());
			Assert::IsTrue(CLASS_IN == query1.getQclass());
			DNSQuery query2 = queries[1];
			Assert::AreEqual("www.microsoft.com", query2.getQname());
			Assert::IsTrue(TYPE_A == query2.getQtype());
			Assert::IsTrue(CLASS_IN == query2.getQclass());
			Assert::IsTrue(buf == (char*)(packet->data + 43));
		}
	};

	DNSPacket *DNSResolverTest::packet;
	char *DNSResolverTest::buf;
	SQLEnvironment DNSResolverTest::env;
}
