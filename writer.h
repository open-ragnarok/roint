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
#ifndef __ROINT_INTERNAL_WRITER_H
#define __ROINT_INTERNAL_WRITER_H

// For ROInt internal use only

struct _writer {
	/// Destructor.
	void (*destroy)(struct _writer *writer);

	/// Write 'count' elements of size 'size' from 'src'. (updates error indicator)
	/// Returns 0 on success.
	int (*write)(const void *src, unsigned long size, unsigned int count, struct _writer *writer);

	/// Resize the data. (updates error indicator)
	/// The expanded area will be 0-filled.
	/// The position indicator will remain unchanged.
	/// Returns 0 on success.
	int (*resize)(unsigned long size, struct _writer *writer);

	/// Set position indicator. (updates error indicator)
	/// pos : offset from origin
	/// origin : SEEK_SET or SEEK_CUR or SEEK_END
	/// Returns 0 on success.
	int (*seek)(struct _writer *writer, long pos, int origin);

	/// Get position indicator.
	unsigned long (*tell)(struct _writer *writer);

	/// Error indicator. (0 for success)
	int error;
};

// Constructors.

/// Writer that uses an internal memory buffer.
/// Fills data_out and size_out when destroyed.
/// WARNING : the 'data_out' data has to be released with the roint free function
struct _writer *memwriter_init(unsigned char **data_out, unsigned long *size_out);
struct _writer *filewriter_init(const char *fn);

#endif /* __ROINT_INTERNAL_WRITER_H */
