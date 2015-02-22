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

#include "SQLEnvironment.h"

class SQLConnection {
	friend class SQLStatement;
	SQLHDBC hdbc;
	SQLCHAR *errorBuf = NULL;

	void handleErrors(int retval);
public:
	SQLConnection(const SQLEnvironment &environment);
	void connect(SQLWCHAR *connString);
	bool isConnected();
	~SQLConnection();
};

