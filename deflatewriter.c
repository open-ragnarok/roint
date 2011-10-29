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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>


#define CAST_UP(type, member, ptr) (type*)( (char*)ptr - offsetof(type,member) )
#define CAST_DOWN(ptr, member) ( &ptr->member )


struct _deflatewriter {
	struct _writer base;
	struct _writer *parent;
	z_stream stream;
};


const int DEFLATEWRITER_LEVEL = Z_BEST_COMPRESSION;
const int DEFLATEWRITER_METHOD = Z_DEFLATED;
const int DEFLATEWRITER_WINDOW_BITS = MAX_WBITS;
const int DEFLATEWRITER_MEM_LEVEL = MAX_MEM_LEVEL;
const int DEFLATEWRITER_STRATEGY = Z_DEFAULT_STRATEGY;


/// zlib error
void _deflatewriter_zerror(const char *funcname, int err) {
	switch(err) {
		case Z_MEM_ERROR:
			_xlog("deflatewriter.%s : Z_MEM_ERROR\n", funcname);
			break;
		case Z_BUF_ERROR:
			_xlog("deflatewriter.%s : Z_BUF_ERROR\n", funcname);
			break;
		case Z_STREAM_ERROR:
			_xlog("deflatewriter.%s : Z_STREAM_ERROR\n", funcname);
			break;
		case Z_DATA_ERROR:
			_xlog("deflatewriter.%s : Z_DATA_ERROR\n", funcname);
			break;
		default:
			_xlog("deflatewriter.%s : zlib error number %d\n", funcname, err);
			break;
	}
}


/// zlib allocator
voidpf _deflatewriter_zalloc_func(voidpf opaque, uInt items, uInt size) {
	if (_mul_over_limit(items, size, 0xFFFFFFFF))
		return(Z_NULL);
	return((voidpf)_xalloc(items * size));
}


/// zlib deallocator
void _deflatewriter_zfree_func(voidpf opaque, voidpf address) {
	_xfree(address);
}


/// Output data to parent writer.
/// flush=Z_FINISH : terminate the stream, writing all pending data
void _deflatewriter_output(struct _deflatewriter *deflatewriter, const void *data, unsigned int length, int flush) {
	struct _writer *parent = deflatewriter->parent;
	unsigned char buf[0x8000]; // 32k
	unsigned int bytes;
	int err;

	deflatewriter->stream.next_in = (Bytef*)data;
	deflatewriter->stream.avail_in = (uInt)length;
	for(;;) {
		deflatewriter->stream.next_out = (Bytef*)buf;
		deflatewriter->stream.avail_out = sizeof(buf);
		err = deflate(&deflatewriter->stream, flush);
		if (err != Z_OK && err != Z_STREAM_END) {
			_deflatewriter_zerror("output",err);
			deflatewriter->base.error = 1;
			return;
		}
		bytes = sizeof(buf) - deflatewriter->stream.avail_out;
		if (bytes > 0) {
			parent->write(buf, 1, bytes, parent);
			if (parent->error) {
				_xlog("deflatewriter.output : parent.write error\n");
				deflatewriter->base.error = 1;
				return;
			}
		}
		if (flush == Z_FINISH) {
			if (err == Z_STREAM_END)
				return;
		}
		else {
			if (deflatewriter->stream.avail_in == 0)
				return;
		}
	}
}


void deflatewriter_destroy(struct _writer *writer) {
	struct _deflatewriter *deflatewriter = CAST_UP(struct _deflatewriter,base,writer);
	struct _writer *parent = deflatewriter->parent;
	int err;

	_deflatewriter_output(deflatewriter, Z_NULL, 0, Z_FINISH);
	err = deflateEnd(&deflatewriter->stream);
	if (err != Z_OK) {
		_deflatewriter_zerror("destroy", err);
		writer->error = 1;
	}
	parent->error = writer->error; // propagate
	_xfree(deflatewriter);
}


int deflatewriter_write(const void *src, unsigned long size, unsigned int count, struct _writer *writer) {
	struct _deflatewriter *deflatewriter = CAST_UP(struct _deflatewriter,base,writer);
	unsigned int bytes;

	writer->error = 0;
	if (_mul_over_limit(size, count, 0xFFFFFFFF)) {
		_xlog("deflatewriter.write : overflow\n");
		writer->error = 1;
		return(writer->error);
	}

	bytes = (unsigned int)size * count;
	_deflatewriter_output(deflatewriter, src, bytes, Z_NO_FLUSH);
	if (writer->error)
		_xlog("deflatewriter.write : error\n");
	return(writer->error);
}


int deflatewriter_resize(unsigned long size, struct _writer *writer) {
	struct _deflatewriter *deflatewriter = CAST_UP(struct _deflatewriter,base,writer);

	_xlog("deflatewriter.resize : not supported\n");
	writer->error = 1;
	return(writer->error);
}


int deflatewriter_seek(struct _writer *writer, long pos, int origin) {
	struct _deflatewriter *deflatewriter = CAST_UP(struct _deflatewriter,base,writer);

	_xlog("deflatewriter.seek : not supported\n");
	writer->error = 1;
	return(writer->error);
}


unsigned long deflatewriter_tell(struct _writer *writer) {
	struct _deflatewriter *deflatewriter = CAST_UP(struct _deflatewriter,base,writer);

	_xlog("deflatewriter.seek : not supported\n");
	writer->error = 1;
	return(0);
}


struct _writer *deflatewriter_init(struct _writer *parent, unsigned char type) {
	struct _deflatewriter *ret = (struct _deflatewriter*)_xalloc(sizeof(struct _deflatewriter));
	int windowBits;
	int err;

	ret->base.destroy = &deflatewriter_destroy;
	ret->base.write = &deflatewriter_write;
	ret->base.resize = &deflatewriter_resize;
	ret->base.seek = &deflatewriter_seek;
	ret->base.tell = &deflatewriter_tell;
	ret->base.error = 0;

	ret->stream.zalloc = (alloc_func)&_deflatewriter_zalloc_func;
	ret->stream.zfree = (free_func)&_deflatewriter_zfree_func;
	ret->stream.opaque = (voidpf)ret;
	ret->stream.next_in = Z_NULL;
	ret->stream.avail_in = 0;

	ret->parent = parent;

	if (type == 255)
		windowBits = -DEFLATEWRITER_WINDOW_BITS;
	else
		windowBits = DEFLATEWRITER_WINDOW_BITS + (int)type * 16;
	err = deflateInit2(&ret->stream,
		DEFLATEWRITER_LEVEL, 
		DEFLATEWRITER_METHOD,
		windowBits,
		DEFLATEWRITER_MEM_LEVEL,
		DEFLATEWRITER_STRATEGY);
	if (err != Z_OK) {
		_deflatewriter_zerror("init",err);
		ret->base.error = 1;
	}

	return(CAST_DOWN(ret,base));
}
