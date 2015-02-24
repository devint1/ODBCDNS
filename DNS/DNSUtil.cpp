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

#include <cstring>
#include <exception>
#include <stdint.h>
#include "DNSUtil.h"

using std::exception;

size_t DNSUtil::decodeName(char *name, DNSPacket *packet, char *outputBuffer, size_t bufLen) {
	LoggerPtr logger(Logger::getLogger("DNSUtil"));
	char *srcLoc(name);
	unsigned int count;
	int dstLoc = 0;
	size_t qnameLength = strlen(srcLoc);
	size_t bytesRead = 0;
	
	//Check name length
	if (qnameLength > 255) {
		throw exception("QNAME length longer than maximum of 255");
	}
	do {
		// Check label length or pointer
		LOG4CXX_TRACE(logger, "Checking label length/pointer");
		count = (unsigned int)srcLoc[0];
		if ((count & 0xC0) == 0xC0) {
			uint16_t ptr = srcLoc[1] | srcLoc[0] << 8;
			ptr &= 0x3FFF;
			srcLoc = (char*)packet + ptr;
			continue;
		}
		else if (count > 63) {
			throw exception("Label length longer than maximum of 63", count);
		}
		
		// Write characters
		LOG4CXX_TRACE(logger, "Writing characters");
		for (unsigned int i = 1; i <= count; ++i) {
			if (dstLoc + 1 > bufLen) {
				throw exception("Buffer too small");
			}
			outputBuffer[dstLoc++] = srcLoc[i];
			++bytesRead;
		}

		// Write dot
		if (dstLoc + 1 > bufLen) {
			throw exception("Buffer too small");
		}
		outputBuffer[dstLoc++] = '.';
		++bytesRead;

		// Incriment pointer
		srcLoc += (count + 1);
	} while (count > 0);

	// Close string, return
	outputBuffer[dstLoc - 2] = '\0';
	LOG4CXX_TRACE(logger, "Done decoding name");
	return bytesRead;
}

size_t DNSUtil::encodeName(char *name, char *outputBuffer, size_t bufLen) {
	LoggerPtr logger(Logger::getLogger("DNSUtil"));
	char *lastPtr = name;
	char *outputPtr = outputBuffer;
	bool read = true;
	size_t totalLen = 0;

	// While we are finding dots
	while(read) {
		// Get dot position
		char *ptr = strchr(lastPtr, '.');

		// If we are about to write the last label, look for null terminator
		// Only do this if there is data to read
		if (!ptr) {
			LOG4CXX_TRACE(logger, "Final iteration");
			ptr = strchr(lastPtr, '\0');
			read = false;
		}

		// Determine the length of this label
		uint8_t len = (uint8_t)(ptr - lastPtr);

		// Write the length of the label
		LOG4CXX_TRACE(logger, "Writing label length: " << len);
		outputPtr[0] = len;
		++outputPtr;

		// Write the label
		LOG4CXX_TRACE(logger, "Writing label");
		memcpy_s(outputPtr, bufLen, lastPtr, len);

		// Incriment pointers
		lastPtr += len + 1;
		outputPtr += len;
		totalLen += len + 1;
	}

	// Close string, return strlen + null terminator
	outputBuffer[totalLen] = '\0';
	LOG4CXX_TRACE(logger, "Done encoding name");
	return totalLen + 1;
}
