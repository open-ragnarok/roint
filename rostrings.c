/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of The Open Ragnarok Project
    Copyright 2007 - 2011 The Open Ragnarok Team
    For the latest information visit http://www.open-ragnarok.org
    ------------------------------------------------------------------------------------
    This program is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by the Free Software
    Foundation; either version 2 of the License, or (at your option) any later
    version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License along with
    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
    http://www.gnu.org/copyleft/lesser.txt.
    ------------------------------------------------------------------------------------
*/
#include "internal.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define CP949_COUNT 17048

const unsigned short cp949_start = 0x8141;
const unsigned short cp949[][2] = {
#include "cp949.h"
};

char *stringFromuString(unsigned short *utf16);

unsigned int ustrlen(const unsigned short *x) {
	int ret = 0;

	while (x[ret] != 0)
		ret++;

	return(ret);
}

// EUCKR -> UTF8
unsigned short cp949_conv(const unsigned short x) {
	unsigned int i = 0;
	for (i = 0; i < CP949_COUNT; i++)
		if (cp949[i][1] == x)
			return(cp949[i][0]);
    
	return(0);
}

// UTF8 -> EUCKR
unsigned short cp949_convr(const unsigned short x) {
	unsigned int i = 0;
	for (i = 0; i < CP949_COUNT; i++)
		if (cp949[i][0] == x)
			return(cp949[i][1]);
    
    return(0);
}

// From EUCKR to UTF16 (array of short)
unsigned short *ustringFromEUCKR(const char *s) {
	unsigned short aux[512];
	unsigned char c;
	unsigned short s_euc, s_utf;
	unsigned int i, k;
#ifdef __APPLE__
    size_t len;
#else
    unsigned int len;
#endif
	unsigned short *ret;

	len = strlen(s);

	memset(aux, 0, 512 * sizeof(unsigned short));
	k = 0; // aux array pointer
	for (i = 0; i < len; i++) {
		c = s[i];
		if ((0x80 & c) > 0) {
			// Encoded stuff
			s_euc = c;
			s_euc <<= 8;
			s_euc += s[++i];

			if (s_euc < cp949_start) {
				fprintf(stderr, "Error converting EUCKR encoded value 0x%04x\n", s_euc);
				s_utf = 0xfffd;
			}
			else {
				s_utf = cp949_conv(s_euc);
			}
		}
		else {
			s_utf = c;
		}
		aux[k++] = s_utf;
	}

	ret = (unsigned short*)_xalloc(sizeof(unsigned short) * (k+1));
	memcpy(ret, aux, sizeof(unsigned short) * (k+1));
	return(ret);
}

char *stringFromEUCKR(const char *s) {
	unsigned short *utf16;
	char *ret;

	utf16 = ustringFromEUCKR(s);
	ret = stringFromuString(utf16);
	_xfree(utf16);

	return(ret);
}

char *stringFromuString(unsigned short *utf16) {
	unsigned short *x, c;
	unsigned char c_utf[5];
	unsigned int offset;
	char *ret;
	unsigned int i, len;

	len = ustrlen(utf16);
	ret = (char*)_xalloc(3 * (len+1));
	memset(ret, 0, 3 * (len+1));
	x = utf16;
	offset = 0;

	while (*x != 0) {
		c = *x;
		if (c <= 127) {
			ret[offset++] = (char)*x;
		}
		else if (c <= 2047) {
			// 2-byte encoding
			c_utf[0] = (c & 0x03c0) >> 6;
			c_utf[1] = (c & 0x003f);
			
			c_utf[0] |= 0xc0;
			c_utf[1] |= 0x80;
			c_utf[2] = 0;

			for (i = 0; i < 2; i++)
				ret[offset++] = c_utf[i];
		}
		else {
			// 3-byte encoding
			c_utf[0] = (c & 0xf000) >> 12;
			c_utf[1] = (c & 0x0fc0) >> 6;
			c_utf[2] = (c & 0x003f);

			c_utf[0] |= 0xe0;
			c_utf[1] |= 0x80;
			c_utf[2] |= 0x80;
			c_utf[3] = 0;

			for (i = 0; i < 3; i++)
				ret[offset++] = c_utf[i];
		}
		x++;
	}

	return(ret);
}