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

#define MAX_COL_LENGTH 128

#include <map>
#include <sql.h>
#include <string>
#include <vector>

using std::map;
using std::wstring;
using std::vector;

class SQLResultSet {
	friend class SQLStatement;
	SQLHSTMT hstmt;
	SQLCHAR *errorBuf = NULL;
	vector<SQLULEN> colLengths;
	vector<SQLSMALLINT> dataTypes;
	map<wstring, int> colPositions;

	SQLResultSet(SQLHSTMT hstmt);
	void handleErrors(SQLRETURN retval);
public:
	~SQLResultSet();
	bool next();
	SQLSMALLINT getDataType(SQLSMALLINT colNum);
	SQLSMALLINT getDataType(wstring colName);
	SQLULEN getColLength(SQLSMALLINT colNum);
	SQLULEN getColLength(wstring colName);
	void getUShort(SQLSMALLINT colNum, SQLUSMALLINT *outputBuffer);
	void getUShort(wstring colName, SQLUSMALLINT *outputBuffer);
	void getLong(SQLSMALLINT colNum, SQLINTEGER *outputBuffer);
	void getLong(wstring colName, SQLINTEGER *outputBuffer);
	void getString(SQLSMALLINT colNum, SQLCHAR *outputBuffer);
	void getString(wstring colName, SQLCHAR *outputBuffer);
	void getWString(SQLSMALLINT colNum, SQLWCHAR *outputBuffer);
	void getWString(wstring colName, SQLWCHAR *outputBuffer);
};

