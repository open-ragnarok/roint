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
#ifndef __ROINT_INTERNAL_READER_H
#define __ROINT_INTERNAL_READER_H

// For ROInt internal use only

struct _reader {
	/// Destructor.
	void (*destroy)(struct _reader *reader);

	/// Read 'count' elements of size 'size' into 'dest'. (updates error indicator)
	/// If not enough data, zero out incomplete elements.
	/// Returns 0 on success.
	int (*read)(void *dest, unsigned long size, unsigned int count, struct _reader *reader);

	/// Set position indicator. (updates error indicator)
	/// pos : offset from origin
	/// origin : SEEK_SET or SEEK_CUR or SEEK_END
	/// Returns 0 on success.
	int (*seek)(struct _reader *reader, long pos, int origin);

	/// Get position indicator. (updates error indicator)
	unsigned long (*tell)(struct _reader *reader);

	/// Error indicator. (0 for success)
	int error;
};

// Constructors.

/// Reader that takes input from 'parent' and inflates it with zlib DEFLATE.
/// type=0 : decode only the zlib format (default)
/// type=1 : decode only the gzip format
/// type=2 : zlib and gzip decoding with automatic header detection
/// type=255 : process raw deflate data
struct _reader *deflatereader_init(struct _reader *parent, unsigned char type);
struct _reader *memreader_init(const unsigned char *ptr, unsigned long size);
struct _reader *filereader_init(const char *fn);

#endif /* __ROINT_INTERNAL_READER_H */
