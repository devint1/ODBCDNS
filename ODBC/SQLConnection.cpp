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
#include "SQLConnection.h"
#include "SQLMessageHandler.h"
#include <sqlext.h>

using std::exception;

SQLConnection::SQLConnection(const SQLEnvironment &environment) {
	SQLRETURN retval = SQLAllocHandle(SQL_HANDLE_DBC, environment.henv, &hdbc);
	handleErrors(retval);
	retval = SQLSetConnectAttr(hdbc, SQL_ATTR_AUTOCOMMIT, SQL_AUTOCOMMIT_OFF, 0);
	handleErrors(retval);
}

SQLConnection::~SQLConnection() {
	SQLDisconnect(hdbc);
	SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
	delete[] errorBuf;
}

void SQLConnection::connect(SQLWCHAR *connString) {
	SQLRETURN retval = SQLDriverConnect(hdbc, NULL, connString, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
	handleErrors(retval);
}

bool SQLConnection::isConnected() {
	long dead = SQL_CD_FALSE;
	SQLRETURN retval = SQLGetConnectAttr(hdbc, SQL_ATTR_CONNECTION_DEAD, &dead, sizeof(long), NULL);
	handleErrors(retval);
	return !dead;
}

void SQLConnection::handleErrors(int retval) {
	if (retval == SQL_ERROR) {
		if (!errorBuf) {
			errorBuf = new SQLCHAR[SQL_MAX_MESSAGE_LENGTH];
		}
		SQLMessageHandler::getSqlMessage(SQL_HANDLE_DBC, hdbc, errorBuf, SQL_MAX_MESSAGE_LENGTH);
		throw exception((char*)errorBuf);
	}
	else if (retval == SQL_INVALID_HANDLE) {
		throw exception("A handle needed by the connection is invalid");
	}
}
