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

#include <errno.h>
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


struct _filewriter {
	struct _writer base;
	FILE *fp;
};


void _filewriter_ferror(const char *funcname) {
	_xlog("filewriter.%s : %s\n", funcname, strerror(errno));
}


void filewriter_destroy(struct _writer *writer) {
	struct _filewriter *filewriter = CAST_UP(struct _filewriter,base,writer);

	if (filewriter->fp != NULL)
		fclose(filewriter->fp);
	_xfree(filewriter);
}


int filewriter_write(const void *src, unsigned long size, unsigned int count, struct _writer *writer) {
	struct _filewriter *filewriter = CAST_UP(struct _filewriter,base,writer);
	size_t donecount;

	writer->error = 0;
	donecount = fwrite(src, size, count, filewriter->fp);
	if (donecount != count) {
		_filewriter_ferror("write");
		writer->error = 1;
	}
	return(writer->error);
}


int filewriter_resize(unsigned long size, struct _writer *writer) {
	struct _filewriter *filewriter = CAST_UP(struct _filewriter,base,writer);

	writer->error = 0;
#if defined(HAVE_FTRUNCATE64)
	if (ftruncate64(fileno(filewriter->fp), size) != 0) {
		_filewriter_ferror("resize");
		writer->error = 1;
	}
#elif defined(HAVE__CHSIZE_S)
	if (_chsize_s(fileno(filewriter->fp), size) != 0) {
		_filewriter_ferror("resize");
		writer->error = 1;
	}
#elif defined(HAVE_FTRUNCATE)
	if (ftruncate(fileno(filewriter->fp), size) != 0) {
		_filewriter_ferror("resize");
		writer->error = 1;
	}
#elif defined(HAVE_CHSIZE)
	if (chsize(fileno(filewriter->fp), size) != 0) {
		_filewriter_ferror("resize");
		writer->error = 1;
	}
#else
	_xlog("filewriter.resize : TODO\n");
	writer->error = 1;
#endif
	return(writer->error);
}


int filewriter_seek(struct _writer *writer, long pos, int origin) {
	struct _filewriter *filewriter = CAST_UP(struct _filewriter,base,writer);

	writer->error = 0;
	if (fseek(filewriter->fp, pos, origin) != 0) {
		_filewriter_ferror("seek");
		writer->error = 1;
	}
	return(writer->error);
}


unsigned long filewriter_tell(struct _writer *writer) {
	struct _filewriter *filewriter = CAST_UP(struct _filewriter,base,writer);
	long pos;

	writer->error = 0;
	pos = ftell(filewriter->fp);
	if (pos == -1) {
		pos = 0;
		_filewriter_ferror("tell");
		writer->error = 1;
	}
	return((unsigned long)pos);
}


struct _writer *filewriter_init(const char *fn) {
	struct _filewriter *ret = (struct _filewriter*)_xalloc(sizeof(struct _filewriter));

	ret->base.destroy = &filewriter_destroy;
	ret->base.write = &filewriter_write;
	ret->base.resize = &filewriter_resize;
	ret->base.seek = &filewriter_seek;
	ret->base.tell = &filewriter_tell;
	ret->base.error = 0;

	ret->fp = fopen(fn,"wb");
	if (ret->fp == NULL) {
		_filewriter_ferror("init");
		ret->base.error = 1;
	}

	return(CAST_DOWN(ret,base));
}
