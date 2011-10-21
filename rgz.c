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
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>


voidpf _zalloc_func(voidpf opaque, uInt items, uInt size) {
	return((voidpf)_xalloc(items * size));
}


void _zfree_func(voidpf opaque, voidpf address) {
	_xfree(address);
}


void _zerror(int err) {
	switch(err) {
		case Z_MEM_ERROR:
			_xlog("Error uncompressing data Z_MEM_ERROR\n");
			break;
		case Z_BUF_ERROR:
			_xlog("Error uncompressing data Z_BUF_ERROR\n");
			break;
		case Z_STREAM_ERROR:
			_xlog("Error uncompressing data Z_STREAM_ERROR\n");
			break;
		case Z_DATA_ERROR:
			_xlog("Error uncompressing data Z_DATA_ERROR\n");
			break;
		case Z_STREAM_END:
			_xlog("Error uncompressing data Z_STREAM_END\n");
			break;
		default:
			_xlog("Unknown error when uncompressing data: %d\n", err);
			break;
	}
}


int _zread(z_stream *s, void *buf, unsigned int len) {
	int err;
	s->next_out = (Bytef*)buf;
	s->avail_out = (uInt)len;
	err = inflate(s, Z_SYNC_FLUSH);
	if (err == Z_STREAM_END && s->avail_out == 0)
		err = Z_OK;// buffer was filled, all ok
	return(err);
}


struct RORgz *rgz_loadFromData(const unsigned char *data, unsigned int length) {
	z_stream stream;
	int err;
	struct RORgz *ret;
	unsigned int entrylimit;

	if (data == NULL || length == 0) {
		_xlog("No input data for RGZ\n");
		return(NULL);
	}

	stream.next_in = (Bytef*)data;
	stream.avail_in = (uInt)length;
	stream.zalloc = (alloc_func)&_zalloc_func;
	stream.zfree = (free_func)&_zfree_func;
	stream.opaque = Z_NULL;

	err = inflateInit2(&stream, 15 + 16);// gzip format only
	if (err != Z_OK) {
		_zerror(err);
		return(NULL);
	}

	ret = (struct RORgz*)_xalloc(sizeof(struct RORgz));
	ret->entries = NULL;
	ret->entrycount = entrylimit = 0;
	for (;;) {
		struct RORgzEntry *entry;
		unsigned char pathlen;

		if (ret->entrycount == entrylimit) {
			struct RORgzEntry *old = ret->entries;
			entrylimit = entrylimit * 4 + 3;
			ret->entries = (struct RORgzEntry*)_xalloc(sizeof(struct RORgzEntry) * entrylimit);
			if (ret->entrycount > 0)
				memcpy(ret->entries, old, sizeof(struct RORgzEntry) * ret->entrycount);
		}
		entry = ret->entries + ret->entrycount;
		memset(entry, 0, sizeof(struct RORgzEntry));
		ret->entrycount++;

		err = _zread(&stream, &entry->type, 1);
		if (err != Z_OK)
			break;

		err = _zread(&stream, &pathlen, 1);
		if (err != Z_OK)
			break;

		if (pathlen > 0) {
			err = _zread(&stream, &entry->path, pathlen);
			if (err != Z_OK)
				break;
			entry->path[pathlen - 1] = 0;
		}

		if (entry->type == 'f') {
			err = _zread(&stream, &entry->datalength, 4);
			if (err != Z_OK)
				break;

			if (entry->datalength > 0) {
				entry->data = (unsigned char*)_xalloc(sizeof(unsigned char) * entry->datalength);
				err = _zread(&stream, entry->data, entry->datalength);
				if (err != Z_OK)
					break;
			}
		}
		else if (entry->type == 'd') {
		}
		else if (entry->type == 'e') {
			break; // ignore rest of data
		}
		else {
			_xlog("Unknown entry type '%c' (%s)\n", entry->type, entry->path);
			rgz_unload(ret);
			ret = NULL;
			break;
		}
	}
	if (err != Z_OK) {
		_zerror(err);
		rgz_unload(ret);
		ret = NULL;
	} else if (ret != NULL && ret->entrycount != entrylimit) {
		struct RORgzEntry *old = ret->entries;
		ret->entries = (struct RORgzEntry*)_xalloc(sizeof(struct RORgzEntry) * ret->entrycount);
		memcpy(ret->entries, old, sizeof(struct RORgzEntry) * ret->entrycount);
		_xfree(old);
	}
	err = inflateEnd(&stream);

	return(ret);
}

struct RORgz *rgz_loadFromFile(const char *fn) {
	FILE *fp;
	unsigned char *data;
	long length;
	struct RORgz *ret;

	fp = fopen(fn, "rb");
	if (fp == NULL) {
		_xlog("Cannot open file %s\n", fn);
		return(NULL);
	}

	fseek(fp, 0, SEEK_END);
	length = ftell(fp);
	if (length == -1) {
		_xlog("%s : %s\n", fn, strerror(errno));
		fclose(fp);
		return(NULL);
	}

	data = (unsigned char*)_xalloc((unsigned int)length);
	fseek(fp, 0, SEEK_SET);
	clearerr(fp);
	fread(data, (unsigned int)length, 1, fp);
	if (ferror(fp)) {
		_xlog("%s : %s\n", fn, strerror(errno));
		_xfree(data);
		fclose(fp);
		return(NULL);
	}

	ret = rgz_loadFromData(data, (unsigned int)length);
	_xfree(data);
	fclose(fp);

	return(ret);
}


void rgz_unload(struct RORgz* rgz) {
	unsigned int i;

	if (rgz == NULL)
		return;

	for (i = 0; i < rgz->entrycount; i++)
		if (rgz->entries[i].data != NULL)
			_xfree(rgz->entries[i].data);
	if (rgz->entries != NULL)
		_xfree(rgz->entries);
	_xfree(rgz);
}
