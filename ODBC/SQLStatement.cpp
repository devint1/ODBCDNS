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
#include "SQLStatement.h"
#include <sqlext.h>

using std::exception;

SQLStatement::SQLStatement(const SQLConnection &connection) : hdbc(connection.hdbc) {
	SQLRETURN retval = SQLAllocHandle(SQL_HANDLE_STMT, connection.hdbc, &hstmt);
	handleErrors(retval);
}

SQLStatement::~SQLStatement() {
	SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	delete[] errorBuf;
}

void SQLStatement::prepare(SQLWCHAR *statementText) {
	SQLRETURN retval = SQLPrepare(hstmt, statementText, SQL_NTS);
	handleErrors(retval);
}

void SQLStatement::bindString(SQLUSMALLINT paramNum, SQLULEN paramLen, SQLPOINTER param) {
	SQLRETURN retval = SQLBindParameter(hstmt, paramNum, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, paramLen, 0, param, 0, NULL);
	handleErrors(retval);
}

void SQLStatement::bindUShort(SQLUSMALLINT paramNum, SQLPOINTER param) {
	SQLRETURN retval = SQLBindParameter(hstmt, paramNum, SQL_PARAM_INPUT, SQL_C_USHORT, SQL_SMALLINT, 0, 0, param, 0, NULL);
	handleErrors(retval);
}

void SQLStatement::bindLong(SQLUSMALLINT paramNum, SQLPOINTER param) {
	SQLRETURN retval = SQLBindParameter(hstmt, paramNum, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, param, 0, NULL);
	handleErrors(retval);
}

void SQLStatement::bindULong(SQLUSMALLINT paramNum, SQLPOINTER param) {
	SQLRETURN retval = SQLBindParameter(hstmt, paramNum, SQL_PARAM_INPUT, SQL_C_ULONG, SQL_INTEGER, 0, 0, param, 0, NULL);
	handleErrors(retval);
}

bool SQLStatement::execute(bool commit) {
	SQLRETURN retval = SQLExecute(hstmt);
	handleErrors(retval);
	if (commit) {
		retval = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT);
		handleErrors(retval);
	}
	return (retval = SQLMoreResults(hstmt)) == SQL_SUCCESS || retval == SQL_SUCCESS_WITH_INFO;
}

SQLLEN SQLStatement::executeUpdate(bool commit) {
	SQLRETURN retval = SQLExecute(hstmt);
	handleErrors(retval);
	if (commit) {
		retval = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT);
		handleErrors(retval);
	}
	SQLLEN len = 0;
	retval = SQLRowCount(hstmt, &len);
	handleErrors(retval);
	return len;
}

SQLResultSet SQLStatement::executeQuery(bool commit) {
	int retval = SQLExecute(hstmt);
	handleErrors(retval);
	if (commit) {
		retval = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT);
		handleErrors(retval);
	}
	return SQLResultSet(hstmt);
}

SQLResultSet SQLStatement::executeQuery(SQLWCHAR *statementText, bool commit) {
	SQLRETURN retval = SQLExecDirect(hstmt, (SQLWCHAR*)statementText, SQL_NTS);
	handleErrors(retval);
	if (commit) {
		retval = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT);
		handleErrors(retval);
	}
	return SQLResultSet(hstmt);
}

void SQLStatement::handleErrors(SQLRETURN retval) {
	if (retval == SQL_ERROR) {
		if (!errorBuf) {
			errorBuf = new SQLCHAR[SQL_MAX_MESSAGE_LENGTH];
		}
		SQLMessageHandler::getSqlMessage(SQL_HANDLE_STMT, hstmt, errorBuf, SQL_MAX_MESSAGE_LENGTH);
		throw exception((char*)errorBuf);
	}
	else if (retval == SQL_INVALID_HANDLE) {
		throw exception("A handle needed by the statement is invalid");
	}
}

void SQLStatement::commit() {
	SQLRETURN retval = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT);
	handleErrors(retval);
}
