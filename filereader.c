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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define CAST_UP(type, member, ptr) (type*)( (char*)ptr - offsetof(type,member) )
#define CAST_DOWN(ptr, member) ( &ptr->member )


struct _filereader {
	struct _reader base;
	FILE *fp;
};


void _filereader_ferror(const char *funcname) {
	_xlog("filereader.%s : %s\n", funcname, strerror(errno));
}


void filereader_destroy(struct _reader *reader) {
	struct _filereader *filereader = CAST_UP(struct _filereader,base,reader);

	if (fclose(filereader->fp) != 0) {
		_filereader_ferror("destroy");
		reader->error = 1;
	}
	_xfree(filereader);
}


int filereader_read(void *dest, unsigned long size, unsigned int count, struct _reader *reader) {
	struct _filereader *filereader = CAST_UP(struct _filereader,base,reader);
	size_t donecount;

	reader->error = 0;
	donecount = fread(dest, size, count, filereader->fp);
	if (donecount != count) {
		unsigned long wanted = size * count;
		unsigned long complete = size * donecount;
		memset((unsigned char*)dest + complete, 0, wanted - complete);
		_xlog("filereader.read : not enough data\n");
		reader->error = 1;
	}
	return(reader->error);
}


int filereader_seek(struct _reader *reader, long pos, int origin) {
	struct _filereader *filereader = CAST_UP(struct _filereader,base,reader);

	reader->error = 0;
	if (fseek(filereader->fp, pos, origin) != 0) {
		_filereader_ferror("seek");
		reader->error = 1;
	}
	return(reader->error);
}


unsigned long filereader_tell(struct _reader *reader) {
	struct _filereader *filereader = CAST_UP(struct _filereader,base,reader);
	long pos;

	reader->error = 0;
	pos = ftell(filereader->fp);
	if (pos == -1) {
		pos = 0;
		_filereader_ferror("tell");
		reader->error = 1;
	}
	return((unsigned long)pos);
}


struct _reader *filereader_init(const char *fn) {
	struct _filereader *ret = (struct _filereader*)_xalloc(sizeof(struct _filereader));

	ret->base.destroy = &filereader_destroy;
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
