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

#include <stddef.h>
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


struct _filereader {
	struct _reader base;
	FILE *fp;
};


void memreader_free(struct _reader *reader) {
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
				reader->error = 1;
				break;
			}
			memreader->offset = pos;
			break;
		case SEEK_CUR:
			if ((pos >= 0 && pos + memreader->offset >= memreader->size) || (pos < 0 && (long)(pos + memreader->offset) < 0)) {
				reader->error = 1;
				break;
			}
			memreader->offset += pos;
			break;
		case SEEK_END:
			if (pos > 0 || (long)(pos + memreader->size) < 0) {
				reader->error = 1;
				break;
			}
			memreader->offset = pos + memreader->size;
			break;
		default:
			reader->error = 1;
			break;
	}

	memreader->ptr = memreader->data + memreader->offset;

	return(reader->error);
}


unsigned long memreader_tell(struct _reader *reader) {
	struct _memreader *memreader = CAST_UP(struct _memreader,base,reader);
	return(memreader->offset);
}


struct _reader *memreader_init(const unsigned char *ptr, unsigned long size) {
	struct _memreader *ret = (struct _memreader*)_xalloc(sizeof(struct _memreader));

	ret->base.destroy = &memreader_free;
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


//-------------------------------------------------------------------


void filereader_free(struct _reader *reader) {
	struct _filereader *filereader = CAST_UP(struct _filereader,base,reader);
	fclose(filereader->fp);
	_xfree(filereader);
}


int filereader_read(void *dest, unsigned long size, unsigned int count, struct _reader *reader) {
	struct _filereader *filereader = CAST_UP(struct _filereader,base,reader);
	size_t donecount;

	donecount = fread(dest, size, count, filereader->fp);
	if (donecount == count) {
		reader->error = 0;
	}
	else {
		unsigned long wanted = size * count;
		unsigned long complete = size * donecount;
		memset((unsigned char*)dest + complete, 0, wanted - complete);
		reader->error = 1;
	}
	return(reader->error);
}


int filereader_seek(struct _reader *reader, long pos, int origin) {
	struct _filereader *filereader = CAST_UP(struct _filereader,base,reader);
	reader->error = fseek(filereader->fp, pos, origin);
	return(reader->error);
}


unsigned long filereader_tell(struct _reader *reader) {
	struct _filereader *filereader = CAST_UP(struct _filereader,base,reader);
	return(ftell(filereader->fp));
}


struct _reader *filereader_init(const char *fn) {
	struct _filereader *ret = (struct _filereader*)_xalloc(sizeof(struct _filereader));

	ret->base.destroy = &filereader_free;
	ret->base.read = &filereader_read;
	ret->base.seek = &filereader_seek;
	ret->base.tell = &filereader_tell;
	ret->base.error = 0;

	ret->fp = fopen(fn,"rb");
	if (ret->fp == 0) {
		_xlog("Cannot open file %s\n", fn);
		ret->base.error = 1;
	}

	return(CAST_DOWN(ret,base));
}
