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
#ifdef ROINT_INTERNAL
#	include "internal.h"
#else
#	define _xlog printf
#	define _xalloc malloc
#	define _xfree free
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define CAST_UP(type, member, ptr) (type*)( (char*)ptr - offsetof(type,member) )
#define CAST_DOWN(ptr, member) ( &ptr->member )


struct _memreader {
	struct _reader base;
	const unsigned char *data;
	unsigned long size;

	unsigned long offset;
	const unsigned char *ptr;
};


void memreader_destroy(struct _reader *reader) {
	struct _memreader *memreader = CAST_UP(struct _memreader,base,reader);

	_xfree(memreader);
}


int memreader_read(void *dest, unsigned long size, unsigned int count, struct _reader *reader) {
	struct _memreader *memreader = CAST_UP(struct _memreader,base,reader);
	unsigned long remaining = memreader->size - memreader->offset;
	unsigned long wanted = size * count;

	if (wanted <= remaining) {
		memcpy(dest, memreader->ptr, wanted);
		memreader->ptr += wanted;
		memreader->offset += wanted;
		reader->error = 0;
	}
	else {
		unsigned long complete = (remaining / size) * size;
		if (complete > 0)
			memcpy(dest, memreader->ptr, complete);
		memset((unsigned char*)dest + complete, 0, wanted - complete);
		memreader->ptr = memreader->data + memreader->size;
		memreader->offset = memreader->size;
		_xlog("memreader.read : not enough data\n");
		reader->error = 1;
	}
	return(reader->error);
}


int memreader_seek(struct _reader *reader, long pos, int origin) {
	struct _memreader *memreader = CAST_UP(struct _memreader,base,reader);

	reader->error = 0;
	switch(origin){
		case SEEK_SET:
			if (pos < 0 || (unsigned long)pos >= memreader->size) {
				_xlog("memreader.seek : invalid position\n");
				reader->error = 1;
				break;
			}
			memreader->offset = pos;
			break;
		case SEEK_CUR:
			if ((pos >= 0 && pos + memreader->offset >= memreader->size) || (pos < 0 && (long)(pos + memreader->offset) < 0)) {
				_xlog("memreader.seek : invalid position\n");
				reader->error = 1;
				break;
			}
			memreader->offset += pos;
			break;
		case SEEK_END:
			if (pos > 0 || (long)(pos + memreader->size) < 0) {
				_xlog("memreader.seek : invalid position\n");
				reader->error = 1;
				break;
			}
			memreader->offset = pos + memreader->size;
			break;
		default:
			_xlog("memreader.seek : not supported (origin=%d)\n", origin);
			reader->error = 1;
			break;
	}

	memreader->ptr = memreader->data + memreader->offset;

	return(reader->error);
}


unsigned long memreader_tell(struct _reader *reader) {
	struct _memreader *memreader = CAST_UP(struct _memreader,base,reader);

	reader->error = 0;
	return(memreader->offset);
}


struct _reader *memreader_init(const unsigned char *ptr, unsigned long size) {
	struct _memreader *ret = (struct _memreader*)_xalloc(sizeof(struct _memreader));

	ret->base.destroy = &memreader_destroy;
	ret->base.read = &memreader_read;
	ret->base.seek = &memreader_seek;
	ret->base.tell = &memreader_tell;
	ret->base.error = 0;

	ret->data = ptr;
	ret->size = size;

	ret->offset = 0;
	ret->ptr = ptr;

	return(CAST_DOWN(ret,base));
}
