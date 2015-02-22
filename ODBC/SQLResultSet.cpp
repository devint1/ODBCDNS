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
#include "SQLMessageHandler.h"
#include "SQLResultSet.h"
#include <sqlext.h>

using std::exception;

SQLResultSet::SQLResultSet(SQLHSTMT hstmt) : hstmt(hstmt) {
	SQLSMALLINT colNum = 1;
	SQLWCHAR colName[MAX_COL_LENGTH];
	SQLULEN colLength;
	SQLSMALLINT dataType;
	while (SQLDescribeCol(hstmt, colNum, colName, MAX_COL_LENGTH, NULL,
		&dataType, &colLength, NULL, NULL) == SQL_SUCCESS) {
		wstring colNameStr(colName);
		colPositions[colNameStr] = colNum;
		dataTypes.push_back(dataType);
		colLengths.push_back(colLength);
		++colNum;
	}
}

SQLResultSet::~SQLResultSet() {
	delete[] errorBuf;
}

bool SQLResultSet::next() {
	SQLRETURN retval;
	retval = SQLFetch(hstmt);
	handleErrors(retval);
	return retval == SQL_SUCCESS || retval == SQL_SUCCESS_WITH_INFO;
}

SQLULEN SQLResultSet::getColLength(SQLSMALLINT colNum) {
	return colLengths[colNum - 1];
}

SQLULEN SQLResultSet::getColLength(wstring colName) {
	return getColLength(colPositions[colName]);
}

SQLSMALLINT SQLResultSet::getDataType(SQLSMALLINT colNum) {
	return dataTypes[colNum - 1];
}

SQLSMALLINT SQLResultSet::getDataType(wstring colName) {
	return getDataType(colPositions[colName]);
}

void SQLResultSet::getUShort(SQLSMALLINT colNum, SQLUSMALLINT *outputBuffer) {
	SQLRETURN retval = SQLGetData(hstmt, colNum, SQL_C_USHORT, outputBuffer, 0, NULL);
	handleErrors(retval);
}

void SQLResultSet::getUShort(wstring colName, SQLUSMALLINT *outputBuffer) {
	SQLSMALLINT colNum = colPositions[colName];
	getUShort(colNum, outputBuffer);
}

void SQLResultSet::getLong(SQLSMALLINT colNum, SQLINTEGER *outputBuffer) {
	SQLRETURN retval = SQLGetData(hstmt, colNum, SQL_C_SLONG, outputBuffer, 0, NULL);
	handleErrors(retval);
}

void SQLResultSet::getLong(wstring colName, SQLINTEGER *outputBuffer) {
	SQLSMALLINT colNum = colPositions[colName];
	getLong(colNum, outputBuffer);
}

void SQLResultSet::getString(SQLSMALLINT colNum, SQLCHAR *outputBuffer) {
	SQLLEN colLen = colLengths[colNum - 1] + 1;
	SQLRETURN retval = SQLGetData(hstmt, colNum, SQL_C_CHAR, outputBuffer, colLen, NULL);
	handleErrors(retval);
}

void SQLResultSet::getString(wstring colName, SQLCHAR *outputBuffer) {
	SQLSMALLINT colNum = colPositions[colName];
	getString(colNum, outputBuffer);
}

void SQLResultSet::getWString(SQLSMALLINT colNum, SQLWCHAR *outputBuffer) {
	SQLLEN colLen = colLengths[colNum - 1] + 1;
	SQLRETURN retval = SQLGetData(hstmt, colNum, SQL_C_WCHAR, outputBuffer, colLen, NULL);
	handleErrors(retval);
}

void SQLResultSet::getWString(wstring colName, SQLWCHAR *outputBuffer) {
	SQLSMALLINT colNum = colPositions[colName];
	getWString(colNum, outputBuffer);
}

void SQLResultSet::handleErrors(SQLRETURN retval) {
	if (retval == SQL_ERROR) {
		if (!errorBuf) {
			errorBuf = new SQLCHAR[SQL_MAX_MESSAGE_LENGTH];
		}
		SQLMessageHandler::getSqlMessage(SQL_HANDLE_STMT, hstmt, errorBuf, SQL_MAX_MESSAGE_LENGTH);
		throw exception((char*)errorBuf);
	}
	else if (retval == SQL_INVALID_HANDLE) {
		throw exception("A handle needed by the result set is invalid");
	}
}
