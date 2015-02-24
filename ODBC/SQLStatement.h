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

#pragma once

#include "SQLConnection.h"
#include "SQLResultSet.h"

class SQLStatement {
	SQLHSTMT hstmt;
	SQLHDBC hdbc;
	SQLCHAR *errorBuf = NULL;

	void handleErrors(SQLRETURN retval);
public:
	SQLStatement(const SQLConnection &connection);
	~SQLStatement();
	void prepare(SQLWCHAR *statementText);
	void bindString(SQLUSMALLINT paramNum, SQLULEN paramLen, SQLPOINTER param);
	void bindUShort(SQLUSMALLINT paramNum, SQLPOINTER param);
	void bindLong(SQLUSMALLINT paramNum, SQLPOINTER param);
	void bindULong(SQLUSMALLINT paramNum, SQLPOINTER param);
	bool execute(bool commit = true);
	SQLLEN executeUpdate(bool commit = true);
	SQLResultSet executeQuery(bool commit = false);
	SQLResultSet executeQuery(SQLWCHAR *statementText, bool commit = false);
	void commit();
};
