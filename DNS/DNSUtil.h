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

#include "DNSMessage.h"

struct DNSUtil {
	// Decodes a DNS name and outputs it to a buffer with given length.
	// Returns the number of bytes read.
	static size_t decodeName(__in char *name, __in DNSPacket *packet, __out char *outputBuffer, __in size_t bufLen);

	// Encodes a DNS name and outputs it to a buffer with given length.
	// Returns the number of bytes written.
	static size_t encodeName(__in char *name, __out char *outputBuffer, __in size_t bufLen);
};
