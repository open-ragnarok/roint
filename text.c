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

struct s_converter
{
	unsigned short unicode;// cp949.h does not have bigger unicodes
	unsigned short cp949;
};

const struct s_converter converter[] = {
#include "_cp949.h"
};
#define CONVERTER_COUNT sizeof(converter)/sizeof(converter[0])
#define CP949_START 0x8141


//-------------------------------------------------------------------
// UNICODE int code points (20 bits used)
// CP949 1/2-byte code points



/// encode CP949 code point (1/2 bytes)
size_t roint_encode_cp949(const unsigned short cp949_c, char *cp949) {
	if ((cp949_c&0x80) == 0) {// 1-byte cp949
		if (cp949 != NULL)
			cp949[0] = (char)(cp949_c);
		return(1);
	}
	else {// 2-byte cp949
		if (cp949 != NULL) {
			cp949[0] = (char)((cp949_c>>8)&0xFF);
			cp949[1] = (char)( cp949_c    &0xFF);
		}
		return(2);
	}
}


/// decode CP949 code point (1/2 bytes)
size_t roint_decode_cp949(const char *cp949, unsigned short *cp949_c) {
	if ((cp949[0]&0x80) == 0) {// 1-byte cp949
		if (cp949_c != NULL)
			*cp949_c = (unsigned short)(cp949[0]);
		return(1);
	}
	else if (cp949[1] != 0) {// 2-byte cp949
		if (cp949_c != NULL) {
			*cp949_c  = (unsigned short)((cp949[0]&0xFF)<<8);
			*cp949_c |= (unsigned short)( cp949[1]&0xFF    );
		}
		return(2);
	}
	return(0);// invalid
}


/// convert code point from CP949 to UNICODE (0 on error)
unsigned int roint_convert_cp949_to_unicode(const unsigned short cp949_c) {
	size_t i;
	if (cp949_c <= 0x7F) {// direct translation
		return(cp949_c);
	}
	if (cp949_c < CP949_START)
		return(0);// invalid
	for (i = 0; i < CONVERTER_COUNT; ++i)
		if (converter[i].cp949 == cp949_c)
			return(converter[i].unicode);
	return(0);// not found
}


/// convert code point from UNICODE to CP949 (0 on error)
unsigned short roint_convert_unicode_to_cp949(const unsigned int unicode) {
	size_t i;
	if (unicode <= 0x7F) {// direct translation
		return(unsigned short)(unicode);
	}
	for (i = 0; i < CONVERTER_COUNT; ++i)
		if (converter[i].unicode == unicode)
			return(converter[i].cp949);
	return(0);// not found
}


/// convert string from CP949 to UNICODE (NULL on error)
unsigned int* roint_string_cp949_to_unicode(const char *cp949) {
	unsigned short cp949_c;
	unsigned int unicode_c;
	unsigned int *unicode;
	size_t unicode_len;
	size_t i, len;

	if (cp949 == NULL)
		return(NULL);

	// calculate and validate
	unicode_len = 0;
	for (i = 0; cp949[i] != 0; ) {
		len = roint_decode_cp949(cp949 + i, &cp949_c);
		if (len == 0)
			return(NULL);// invalid
		unicode_c = roint_convert_cp949_to_unicode(cp949_c);
		if (unicode_c == 0)
			return(NULL);// unable to translate
		i += len;
		unicode_len++;
	}

	// convert
	unicode = (unsigned int*)_xalloc(sizeof(unsigned int) * (unicode_len+1));
	unicode_len = 0;
	for (i = 0; cp949[i] != 0; ) {
		len = roint_decode_cp949(cp949 + i, &cp949_c);
		unicode_c = roint_convert_cp949_to_unicode(cp949_c);
		i += len;
		unicode[unicode_len++] = unicode_c;
	}
	unicode[unicode_len] = 0;
	return(unicode);
}


/// convert string from UNICODE to CP949 (NULL on error)
char *roint_string_unicode_to_cp949(const unsigned int *unicode) {
	unsigned short cp949_c;
	char *cp949;
	size_t cp949_len;
	size_t i, len;

	if (unicode == NULL)
		return(NULL);

	// calculate and validate
	cp949_len = 0;
	for (i = 0; unicode[i] != 0; i++) {
		cp949_c = roint_convert_unicode_to_cp949(unicode[i]);
		if (cp949_c == 0)
			return(NULL);// unable to translate
		len = roint_encode_cp949(cp949_c, NULL);
		if (len == 0)
			return(NULL);// invalid
		cp949_len += len;
	}

	// convert
	cp949 = (char*)_xalloc(sizeof(char) * (cp949_len+1));
	cp949_len = 0;
	for (i = 0; unicode[i] != 0; i++) {
		cp949_c = roint_convert_unicode_to_cp949(unicode[i]);
		len = roint_encode_cp949(cp949_c, cp949 + cp949_len);
		cp949_len += len;
	}
	cp949[cp949_len] = 0;
	return(cp949);
}


//-------------------------------------------------------------------
// UTF8 1/2/3/4-byte code points


/// encode UTF8 code point (1/2/3/4 bytes)
size_t roint_encode_utf8(const unsigned int unicode_c, char *utf8) {
	if (unicode_c >= 0x110000) {// invalid
		return(0);
	}
	else if (unicode_c >= 0x10000) {// 4-byte utf8
		if (utf8 != NULL) {
			utf8[0] = (char)(((unicode_c>>18)&0x7 ) + 0xF0);
			utf8[1] = (char)(((unicode_c>>12)&0x3F) + 0x80);
			utf8[2] = (char)(((unicode_c>>6 )&0x3F) + 0x80);
			utf8[3] = (char)(( unicode_c     &0x3F) + 0x80);
		}
		return(4);
	}
	else if (unicode_c >= 0x800) {// 3-byte utf8
		if (utf8 != NULL) {
			utf8[0] = (char)(((unicode_c>>12)&0xF ) + 0xE0);
			utf8[1] = (char)(((unicode_c>>6 )&0x3F) + 0x80);
			utf8[2] = (char)(( unicode_c     &0x3F) + 0x80);
		}
		return(3);
	}
	else if (unicode_c >= 0x80) {// 2-byte utf8
		if (utf8 != NULL) {
			utf8[0] = (char)(((unicode_c>>6)&0x1F) + 0xC0);
			utf8[1] = (char)(( unicode_c    &0x3F) + 0x80);
		}
		return(2);
	}
	else {// 1-byte utf8
		if (utf8 != NULL)
			utf8[0] = (char)(unicode_c&0x7F);
		return(1);
	}
}


/// decode UTF8 code point (1/2/3/4 bytes)
size_t roint_decode_utf8(const char *utf8, unsigned int *unicode_c) {
	if ((utf8[0]&0x80) == 0) {// 1-byte utf8
		if (unicode_c != NULL)
			*unicode_c = (unsigned int)(utf8[0]&0x7F);
		return(1);
	}
	else if ((utf8[0]&0xE0) == 0xC0) {// 2-byte utf8
		if ((utf8[1]&0x80) == 0)
			return(0);// invalid
		if (unicode_c != NULL) {
			*unicode_c  = (unsigned int)((utf8[0]&0x1F)<<6);
			*unicode_c |= (unsigned int)( utf8[1]&0x3F    );
		}
		return(2);
	}
	else if ((utf8[0]&0xF0) == 0xE0) {// 3-byte utf8
		if ((utf8[1]&0x80) == 0 || (utf8[2]&0x80) == 0)
			return(0);// invalid
		if (unicode_c != NULL) {
			*unicode_c  = (unsigned int)((utf8[0]&0xF )<<12);
			*unicode_c |= (unsigned int)((utf8[1]&0x3F)<<6 );
			*unicode_c |= (unsigned int)( utf8[2]&0x3F     );
		}
		return(3);
	}
	else if ((utf8[0]&0xF8) == 0xF0) {// 4-byte utf8
		if ((utf8[1]&0x80) == 0 || (utf8[2]&0x80) == 0 || (utf8[3]&0x80) == 0)
			return(0);// invalid
		if (unicode_c != NULL) {
			*unicode_c  = (unsigned int)((utf8[0]&0x7 )<<18);
			*unicode_c |= (unsigned int)((utf8[1]&0x3F)<<12);
			*unicode_c |= (unsigned int)((utf8[2]&0x3F)<<6 );
			*unicode_c |= (unsigned int)( utf8[3]&0x3F     );
		}
		return(4);
	}
	return(0);// invalid
}


/// convert string from UTF8 to UNICODE (NULL on error)
unsigned int *roint_string_utf8_to_unicode(const char *utf8) {
	unsigned int *unicode;
	size_t unicode_len;
	size_t i, len;

	if (utf8 == NULL)
		return(NULL);

	// calculate and validate
	unicode_len = 0;
	for (i = 0; utf8[i] != 0; ) {
		len = roint_decode_utf8(utf8 + i, NULL);
		if (len == 0)
			return(NULL);// invalid
		i += len;
		unicode_len++;
	}

	// convert
	unicode = (unsigned int*)_xalloc(sizeof(unsigned int) * (unicode_len+1));
	unicode_len = 0;
	for (i = 0; utf8[i] != 0; ) {
		len = roint_decode_utf8(utf8 + i, unicode + unicode_len);
		i += len;
		unicode_len++;
	}
	unicode[unicode_len] = 0;
	return(unicode);
}


/// convert string from UNICODE to UTF8 (NULL on error)
char *roint_string_unicode_to_utf8(const unsigned int *unicode) {
	char *utf8;
	size_t utf8_len;
	size_t i, len;

	if (unicode == NULL)
		return(NULL);

	// calculate and validate
	utf8_len = 0;
	for (i = 0; unicode[i] != 0; i++) {
		len = roint_encode_utf8(unicode[i], NULL);
		if (len == 0)
			return(NULL);// invalid
		utf8_len += len;
	}

	// translate
	utf8 = (char*)_xalloc(sizeof(char) * (utf8_len+1));
	utf8_len = 0;
	for (i = 0; unicode[i] != 0; i++) {
		len = roint_encode_utf8(unicode[i], utf8 + utf8_len);
		utf8_len += len;
	}
	utf8[utf8_len] = 0;
	return(utf8);
}


/// convert string from CP949 to UTF8 (NULL on error)
char* roint_string_cp949_to_utf8(const char *cp949) {
	unsigned short cp949_c;
	unsigned int unicode_c;
	char *utf8;
	size_t utf8_len;
	size_t i, in, out;

	if (cp949 == NULL)
		return(NULL);

	// calculate and validate
	utf8_len = 0;
	for (i = 0; cp949[i] != 0; ) {
		cp949_c = 0;
		in = roint_decode_cp949(cp949 + i, &cp949_c);
		if (in == 0)
			return(NULL);// invalid
		unicode_c = roint_convert_cp949_to_unicode(cp949_c);
		if (unicode_c == 0)
			return(NULL);// unable to translate
		out = roint_encode_utf8(unicode_c, NULL);
		if (out == 0)
			return(NULL);// invalid
		i += in;
		utf8_len += out;
	}

	// convert
	utf8 = (char*)_xalloc(sizeof(char) * (utf8_len+1));
	utf8_len = 0;
	for (i = 0; cp949[i] != 0; ) {
		in = roint_decode_cp949(cp949 + i, &cp949_c);
		unicode_c = roint_convert_cp949_to_unicode(cp949_c);
		out = roint_encode_utf8(unicode_c, utf8 + utf8_len);
		i += in;
		utf8_len += out;
	}
	utf8[utf8_len] = 0;
	return(utf8);
}


/// convert string from UTF8 to CP949 (NULL on error)
char *roint_string_utf8_to_cp949(const char *utf8) {
	unsigned int unicode_c;
	unsigned short cp949_c;
	char *cp949;
	size_t cp949_len;
	size_t i, in, out;

	if (utf8 == NULL)
		return(NULL);

	// calculate and validate
	cp949_len = 0;
	for (i = 0; utf8[i] != 0; ) {
		in = roint_decode_utf8(utf8 + i, &unicode_c);
		if (in == 0)
			return(NULL);// invalid
		cp949_c = roint_convert_unicode_to_cp949(unicode_c);
		if (cp949_c == 0)
			return(NULL);// unable to translate
		out = roint_encode_cp949(cp949_c, NULL);
		if (out == 0)
			return(NULL);// invalid
		i += in;
		cp949_len += out;
	}

	// convert
	cp949 = (char*)_xalloc(sizeof(char) * (cp949_len+1));
	cp949_len = 0;
	for (i = 0; utf8[i] != 0; ) {
		in = roint_decode_utf8(utf8 + i, &unicode_c);
		cp949_c = roint_convert_unicode_to_cp949(unicode_c);
		out = roint_encode_cp949(cp949_c, cp949 + cp949_len);
		i += in;
		cp949_len += out;
	}
	cp949[cp949_len] = 0;
	return(cp949);
}



//-------------------------------------------------------------------
// UTF16 1/2-short code points


/// encode UTF16 code point (1/2 shorts)
size_t roint_encode_utf16(const unsigned int unicode_c, unsigned short *utf16) {
	if (unicode_c >= 0x110000) {// invalid
		return(0);
	}
	else if (unicode_c > 0xFFFF || (unicode_c >= 0xD800 && unicode_c <= 0xDFFF) ) {// 2-short utf16
		if (utf16 != NULL) {
			utf16[0] = (unsigned short)(((unicode_c>>10)&0x3FF) + 0xD800);
			utf16[1] = (unsigned short)(( unicode_c     &0x3FF) + 0xDC00);
		}
		return(2);
	}
	else {// 1-short utf16
		if (utf16 != NULL)
			utf16[0] = (unsigned short)(unicode_c);
		return(1);
	}
}


/// decode UTF16 code point (1/2 shorts)
size_t roint_decode_utf16(const unsigned short *utf16, unsigned int *unicode_c) {
	if (utf16[0] >= 0xD800 && utf16[0] <= 0xDBFF) {// 2-short utf16
		if (utf16[1] < 0xDC00 || utf16[1] > 0xDFFF)
			return(0);// invalid
		if (unicode_c != NULL) {
			*unicode_c  = (unsigned int)((utf16[0]&0x3FF)<<10);
			*unicode_c |= (unsigned int)( utf16[1]&0x3FF     );
		}
		return(1);
	}
	else {// 1-short utf16
		if (unicode_c != NULL)
			*unicode_c = (unsigned int)(utf16[0]);
		return(1);
	}
}


/// convert string from UTF16 to UNICODE (NULL on error)
unsigned int *roint_string_utf16_to_unicode(const unsigned short *utf16) {
	unsigned int *unicode;
	size_t unicode_len;
	size_t i, len;

	if (utf16 == NULL)
		return(NULL);

	// calculate and validate
	unicode_len = 0;
	for (i = 0; utf16[i] != 0; ) {
		len = roint_decode_utf16(utf16 + i, NULL);
		if (len == 0)
			return(NULL);// invalid
		i += len;
		unicode_len++;
	}

	// convert
	unicode = (unsigned int*)_xalloc(sizeof(unsigned int) * (unicode_len+1));
	unicode_len = 0;
	for (i = 0; utf16[i] != 0; ) {
		len = roint_decode_utf16(utf16 + i, unicode + unicode_len);
		i += len;
		unicode_len++;
	}
	unicode[unicode_len] = 0;
	return(unicode);
}


/// convert string from UNICODE to UTF16 (NULL on error)
unsigned short *roint_string_unicode_to_utf16(const unsigned int *unicode) {
	unsigned short *utf16;
	size_t utf16_len;
	size_t i, len;

	if (unicode == NULL)
		return(NULL);

	// calculate and validate
	utf16_len = 0;
	for (i = 0; unicode[i] != 0; i++) {
		len = roint_encode_utf16(unicode[i], NULL);
		if (len == 0)
			return(NULL);// invalid
		utf16_len += len;
	}

	// translate
	utf16 = (unsigned short*)_xalloc(sizeof(unsigned short) * (utf16_len+1));
	utf16_len = 0;
	for (i = 0; unicode[i] != 0; i++) {
		len = roint_encode_utf16(unicode[i], utf16 + utf16_len);
		utf16_len += len;
	}
	utf16[utf16_len] = 0;
	return(utf16);
}


/// convert string from CP949 to UTF16 (NULL on error)
unsigned short* roint_string_cp949_to_utf16(const char *cp949) {
	unsigned short cp949_c;
	unsigned int unicode_c;
	unsigned short *utf16;
	size_t utf16_len;
	size_t i, in, out;

	if (cp949 == NULL)
		return(NULL);

	// calculate and validate
	utf16_len = 0;
	for (i = 0; cp949[i] != 0; ) {
		cp949_c = 0;
		in = roint_decode_cp949(cp949 + i, &cp949_c);
		if (in == 0)
			return(NULL);// invalid
		unicode_c = roint_convert_cp949_to_unicode(cp949_c);
		if (unicode_c == 0)
			return(NULL);// unable to translate
		out = roint_encode_utf16(unicode_c, NULL);
		if (out == 0)
			return(NULL);// invalid
		i += in;
		utf16_len += out;
	}

	// convert
	utf16 = (unsigned short*)_xalloc(sizeof(unsigned short) * (utf16_len+1));
	utf16_len = 0;
	for (i = 0; cp949[i] != 0; ) {
		in = roint_decode_cp949(cp949 + i, &cp949_c);
		unicode_c = roint_convert_cp949_to_unicode(cp949_c);
		out = roint_encode_utf16(unicode_c, utf16 + utf16_len);
		i += in;
		utf16_len += out;
	}
	utf16[utf16_len] = 0;
	return(utf16);
}


/// convert string from UTF16 to CP949 (NULL on error)
char *roint_string_utf16_to_cp949(const unsigned short *utf16) {
	unsigned int unicode_c;
	unsigned short cp949_c;
	char *cp949;
	size_t cp949_len;
	size_t i, in, out;

	if (utf16 == NULL)
		return(NULL);

	// calculate and validate
	cp949_len = 0;
	for (i = 0; utf16[i] != 0; ) {
		in = roint_decode_utf16(utf16 + i, &unicode_c);
		if (in == 0)
			return(NULL);// invalid
		cp949_c = roint_convert_unicode_to_cp949(unicode_c);
		if (cp949_c == 0)
			return(NULL);// unable to translate
		out = roint_encode_cp949(cp949_c, NULL);
		if (out == 0)
			return(NULL);// invalid
		i += in;
		cp949_len += out;
	}

	// convert
	cp949 = (char*)_xalloc(sizeof(char) * (cp949_len+1));
	cp949_len = 0;
	for (i = 0; utf16[i] != 0; ) {
		in = roint_decode_utf16(utf16 + i, &unicode_c);
		cp949_c = roint_convert_unicode_to_cp949(unicode_c);
		out = roint_encode_cp949(cp949_c, cp949 + cp949_len);
		i += in;
		cp949_len += out;
	}
	cp949[cp949_len] = 0;
	return(cp949);
}
