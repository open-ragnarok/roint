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
#ifndef __ROINT_TEXT_H
#define __ROINT_TEXT_H

#ifndef WITHOUT_ROINT_CONFIG
#	include "roint/config.h"
#endif
#include <stddef.h> // size_t

#ifdef __cplusplus
extern "C" {
#endif

// UNICODE int code points (20 bits used)
// CP949 1/2-byte code points


/// encode CP949 code point (1/2 bytes)
ROINT_DLLAPI size_t roint_encode_cp949(const unsigned short cp949_c, char *cp949);
/// decode CP949 code point (1/2 bytes)
ROINT_DLLAPI size_t roint_decode_cp949(const char *cp949, unsigned short *cp949_c);

/// convert code point from CP949 to UNICODE (0 on error)
ROINT_DLLAPI unsigned int roint_convert_cp949_to_unicode(const unsigned short cp949_c);
/// convert code point from UNICODE to CP949 (0 on error)
ROINT_DLLAPI unsigned short roint_convert_unicode_to_cp949(const unsigned int unicode_c);

/// convert string from CP949 to UNICODE (NULL on error)
ROINT_DLLAPI unsigned int *roint_string_cp949_to_unicode(const char *cp949);
/// convert string from UNICODE to CP949 (NULL on error)
ROINT_DLLAPI char *roint_string_unicode_to_cp949(const unsigned int *unicode);


// UTF8 1/2/3/4-byte code points


/// encode UTF8 code point (1/2/3/4 bytes)
ROINT_DLLAPI size_t roint_encode_utf8(const unsigned int unicode_c, char *utf8);
/// decode UTF8 code point (1/2/3/4 bytes)
ROINT_DLLAPI size_t roint_decode_utf8(const char *utf8, unsigned int *unicode_c);

/// convert string from UTF8 to UNICODE (NULL on error)
ROINT_DLLAPI unsigned int *roint_string_utf8_to_unicode(const char *utf8);
/// convert string from UNICODE to UTF8 (NULL on error)
ROINT_DLLAPI char *roint_string_unicode_to_utf8(const unsigned int *unicode);

/// convert string from CP949 to UTF8 (NULL on error)
ROINT_DLLAPI char *roint_string_cp949_to_utf8(const char *cp949);
/// convert string from UTF8 to CP949 (NULL on error)
ROINT_DLLAPI char *roint_string_utf8_to_cp949(const char *utf8);


// UTF16 1/2-short code points


/// encode UTF16 code point (1/2 shorts)
ROINT_DLLAPI size_t roint_encode_utf16(const unsigned int unicode_c, unsigned short *utf16);
/// decode UTF16 code point (1/2 shorts)
ROINT_DLLAPI size_t roint_decode_utf16(const unsigned short *utf16, unsigned int *unicode_c);

/// convert string from UTF16 to UNICODE (NULL on error)
ROINT_DLLAPI unsigned int *roint_string_utf16_to_unicode(const unsigned short *utf16);
/// convert string from UNICODE to UTF16 (NULL on error)
ROINT_DLLAPI unsigned short *roint_string_unicode_to_utf16(const unsigned int *unicode);

/// convert string from CP949 to UTF16 (NULL on error)
ROINT_DLLAPI unsigned short *roint_string_cp949_to_utf16(const char *cp949);
/// convert string from UTF16 to CP949 (NULL on error)
ROINT_DLLAPI char *roint_string_utf16_to_cp949(const unsigned short *utf16);

#ifdef __cplusplus
}
#endif 

#endif /* __ROINT_STRINGS_H */
