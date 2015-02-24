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

#include <string>
#include "SQLEnvironment.h"
#include "SQLMessageHandler.h"
#include <sqlext.h>

using std::exception;

SQLEnvironment::SQLEnvironment() {
	SQLRETURN retval = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
	if (retval == SQL_ERROR) {
		throw exception("Could not initialize SQL environment");
	}
	retval = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
	if (retval == SQL_ERROR) {
		throw exception("Could not set SQL environment version");
	}
}

SQLEnvironment::~SQLEnvironment() {
	SQLFreeHandle(SQL_HANDLE_ENV, henv);
}
