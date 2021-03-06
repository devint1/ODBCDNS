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
#include "..\ODBC\SQLStatement.h"
#include "..\ODBC\SQLResultSet.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace ODBCTest
{
	TEST_CLASS(SQLResultSetTest) {
		static SQLEnvironment env;
		static SQLConnection *connection;
		TEST_CLASS_INITIALIZE(Initialize) {
			connection = new SQLConnection(env);
			SQLWCHAR *connString = L"DRIVER={SQL Server Native Client 11.0};SERVER=(local);DATABASE=DNS;Trusted_Connection=yes;";
			connection->connect(connString);
		}
		TEST_CLASS_CLEANUP(Cleanup) {
			delete connection;
		}
	public:
		TEST_METHOD(GetData) {
			SQLStatement statement(*connection);
			SQLResultSet resultSet = statement.executeQuery(L"SELECT NAME, VALUE FROM RECORD", false);
			int numRecords = 0;
			SQLULEN nameLen = resultSet.getColLength(L"NAME");
			SQLULEN valueLen = resultSet.getColLength(L"VALUE");
			SQLCHAR *name = new SQLCHAR[nameLen + 1];
			SQLCHAR *value = new SQLCHAR[valueLen + 1];
			while (resultSet.next()) {
				++numRecords;
				resultSet.getString(L"NAME", name);
				resultSet.getString(L"VALUE", value);
			}
			delete[] name;
			delete[] value;
			Assert::IsTrue(numRecords > 0);
		}
	};

	SQLEnvironment SQLResultSetTest::env;
	SQLConnection *SQLResultSetTest::connection;
}