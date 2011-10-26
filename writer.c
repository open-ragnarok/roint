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
#	include "writer.h"
#	define _xlog printf
#	define _xalloc malloc
#	define _xfree free
#endif

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(HAVE_UNISTD_H)
#include <unistd.h> // ftruncate/ftruncate64/chsize
#endif
#if defined(HAVE_IO_H)
#include <io.h> // _chsize_s/chsize
#endif


#define CAST_UP(type, member, ptr) (type*)( (char*)ptr - offsetof(type,member) )
#define CAST_DOWN(ptr, member) ( &ptr->member )


struct _memwriter {
	struct _writer base;
	unsigned char **data_out;
	unsigned long *size_out;

	unsigned char *buf;
	unsigned long bufsize;
	unsigned long size;

	unsigned long offset;
	unsigned char *ptr;
};


struct _filewriter {
	struct _writer base;
	FILE *fp;
};


/// Initial size of the memwriter buffer.
const unsigned long MEMWRITER_BUFSIZE = 1024;


/// Ensures the buffer is big enough.
void _memwriter_reserve(unsigned long size, struct _memwriter *memwriter) {
	unsigned long bufsize = memwriter->bufsize;
	while (size < bufsize)
		bufsize = bufsize * 4 + 3;
	if (bufsize != memwriter->bufsize) {
		unsigned char *buf = (unsigned char*)_xalloc(bufsize);
		memcpy(buf, memwriter->buf, memwriter->bufsize);
		memset(buf + memwriter->bufsize, 0, bufsize - memwriter->bufsize);
		_xfree(memwriter->buf);
		memwriter->buf = buf;
		memwriter->bufsize = bufsize;
		memwriter->ptr = memwriter->buf + memwriter->offset;
	}
}


void memwriter_free(struct _writer *writer) {
	struct _memwriter *memwriter = CAST_UP(struct _memwriter,base,writer);
	if (memwriter->data_out != NULL) {
		if (memwriter->size == memwriter->bufsize) {
			*memwriter->data_out = memwriter->buf;
			memwriter->buf = NULL;
		}
		else {
			unsigned char *buf = (unsigned char*)_xalloc(memwriter->size);
			memcpy(buf, memwriter->buf, memwriter->size);
			*memwriter->data_out = buf;
		}
	}
	if (memwriter->size_out != NULL)
		*memwriter->size_out = memwriter->size;

	if (memwriter->buf != NULL)
		_xfree(memwriter->buf);
	_xfree(memwriter);
}


int memwriter_write(void *src, unsigned long size, unsigned int count, struct _writer *writer) {
	struct _memwriter *memwriter = CAST_UP(struct _memwriter,base,writer);
	unsigned long total = size * count;

	_memwriter_reserve(memwriter->offset + total, memwriter);
	memcpy(memwriter->ptr, src, total);
	memwriter->ptr += total;
	memwriter->offset += total;
	if (memwriter->size < memwriter->offset)
		memwriter->size = memwriter->offset;
	writer->error = 0;
	return(writer->error);
}


int memwriter_resize(unsigned long size, struct _writer *writer) {
	struct _memwriter *memwriter = CAST_UP(struct _memwriter,base,writer);

	_memwriter_reserve(size, memwriter);
	memwriter->size = size;
	writer->error = 0;
	return(writer->error);
}


int memwriter_seek(struct _writer *writer, long pos, int origin) {
	struct _memwriter *memwriter = CAST_UP(struct _memwriter,base,writer);
	writer->error = 0;
	switch(origin){
		case SEEK_SET:
			if (pos < 0) {
				writer->error = 1;
				break;
			}
			memwriter->offset = pos;
			break;
		case SEEK_CUR:
			if ((pos < 0 && (long)(pos + memwriter->offset) < 0)) {
				writer->error = 1;
				break;
			}
			memwriter->offset += pos;
			break;
		case SEEK_END:
			if (pos < 0 && (long)(pos + memwriter->size) < 0) {
				writer->error = 1;
				break;
			}
			memwriter->offset = pos + memwriter->size;
			break;
		default:
			writer->error = 1;
			break;
	}

	memwriter->ptr = memwriter->buf + memwriter->offset;

	return(writer->error);
}


unsigned long memwriter_tell(struct _writer *writer) {
	struct _memwriter *memwriter = CAST_UP(struct _memwriter,base,writer);
	return(memwriter->offset);
}


struct _writer *memwriter_init(unsigned char **data_out, unsigned long *size_out) {
	struct _memwriter *ret = (struct _memwriter*)_xalloc(sizeof(struct _memwriter));

	ret->base.destroy = &memwriter_free;
	ret->base.write = &memwriter_write;
	ret->base.resize = &memwriter_resize;
	ret->base.seek = &memwriter_seek;
	ret->base.tell = &memwriter_tell;
	ret->base.error = 0;

	ret->data_out = data_out;
	ret->size_out = size_out;

	ret->buf = (unsigned char*)_xalloc(MEMWRITER_BUFSIZE);
	ret->bufsize = MEMWRITER_BUFSIZE;
	ret->size = 0;

	ret->offset = 0;
	ret->ptr = ret->buf;

	return(CAST_DOWN(ret,base));
}


//-------------------------------------------------------------------


void filewriter_free(struct _writer *writer) {
	struct _filewriter *filewriter = CAST_UP(struct _filewriter,base,writer);
	fclose(filewriter->fp);
	_xfree(filewriter);
}


int filewriter_write(void *src, unsigned long size, unsigned int count, struct _writer *writer) {
	struct _filewriter *filewriter = CAST_UP(struct _filewriter,base,writer);
	size_t donecount;

	donecount = fwrite(src, size, count, filewriter->fp);
	if (donecount == count)
		writer->error = 0;
	else
		writer->error = 1;
	return(writer->error);
}


int filewriter_resize(unsigned long size, struct _writer *writer) {
	struct _filewriter *filewriter = CAST_UP(struct _filewriter,base,writer);

	writer->error = 0;
#if defined(HAVE_FTRUNCATE64)
	if (ftruncate64(fileno(filewriter->fp), size) != 0)
		writer->error = 1;
#elif defined(HAVE__CHSIZE_S)
	if (_chsize_s(fileno(filewriter->fp), size) != 0)
		writer->error = 1;
#elif defined(HAVE_FTRUNCATE)
	if (ftruncate(fileno(filewriter->fp), size) != 0)
		writer->error = 1;
#elif defined(HAVE_CHSIZE)
	if (chsize(fileno(filewriter->fp), size) != 0)
		writer->error = 1;
#else
	// TODO
	writer->error = 1;
#endif
	return(writer->error);
}


int filewriter_seek(struct _writer *writer, long pos, int origin) {
	struct _filewriter *filewriter = CAST_UP(struct _filewriter,base,writer);
	writer->error = fseek(filewriter->fp, pos, origin);
	return(writer->error);
}


unsigned long filewriter_tell(struct _writer *writer) {
	struct _filewriter *filewriter = CAST_UP(struct _filewriter,base,writer);
	return(ftell(filewriter->fp));
}


struct _writer *filewriter_init(const char *fn) {
	struct _filewriter *ret = (struct _filewriter*)_xalloc(sizeof(struct _filewriter));

	ret->base.destroy = &filewriter_free;
	ret->base.write = &filewriter_write;
	ret->base.resize = &filewriter_resize;
	ret->base.seek = &filewriter_seek;
	ret->base.tell = &filewriter_tell;
	ret->base.error = 0;

	ret->fp = fopen(fn,"wb");
	if (ret->fp == 0) {
		_xlog("Cannot open file %s for writing\n", fn);
		ret->base.error = 1;
	}

	return(CAST_DOWN(ret,base));
}
