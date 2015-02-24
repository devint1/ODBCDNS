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

#include "SQLMessageHandler.h"

void SQLMessageHandler::getSqlMessage(SQLSMALLINT handleType, SQLHANDLE handle, SQLCHAR *errorBuf, int errorBufLen) {
	int i = 1;
	SQLRETURN retval;
	SQLCHAR msg[SQL_MAX_MESSAGE_LENGTH];
	while ((retval = SQLGetDiagRec(handleType, handle, i, NULL, NULL,
		msg, sizeof(msg), NULL)) != SQL_NO_DATA && retval != SQL_ERROR && 
		retval != SQL_INVALID_HANDLE) {
		i++;
	}
	memcpy_s(errorBuf, errorBufLen, msg, errorBufLen);
}
