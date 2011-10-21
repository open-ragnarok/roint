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
#include "memloader.h"

#ifdef OPENRO_INTERNAL
#	include "internal.h"
#else
#	define _xlog printf
#	define _xalloc malloc
#	define _xfree free
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct _memloader {
	const unsigned char *data;
	unsigned long size;

	unsigned long offset;
	const unsigned char *ptr;
	int error;
};

struct _memloader *loader_init(const unsigned char *ptr, unsigned long size) {
	struct _memloader *ret = _xalloc(sizeof(struct _memloader));

	ret->data = ptr;
	ret->size = size;

	ret->offset = 0;
	ret->ptr = ptr;
	ret->error = 0;

	return(ret);
}

void loader_free(struct _memloader *loader) {
	if (loader != NULL)
        _xfree(loader);
}

unsigned long loader_tell(const struct _memloader *loader) {
	return(loader->offset);
}

int loader_read(void *dest, unsigned long size, unsigned int count, struct _memloader *loader) {
	unsigned long remaining = loader->size - loader->offset;
	unsigned long wanted = size * count;
	if (wanted <= remaining) {
		memcpy(dest, loader->ptr, wanted);
		loader->ptr += wanted;
		loader->offset += wanted;
		loader->error = 0;
	}
	else {
		unsigned long complete = (remaining / size) * size;
		if (complete > 0)
			memcpy(dest, loader->ptr, complete);
		memset((unsigned char*)dest + complete, 0, wanted - complete);
		loader->ptr = loader->data + loader->size;
		loader->offset = loader->size;
		loader->error = 1;
	}
	return(loader->error);
}

int loader_seek(struct _memloader *loader, long pos, int origin) {
	loader->error = 0;
	switch(origin){
		case SEEK_SET:
			if (pos < 0 || (unsigned long)pos >= loader->size) {
				loader->error = 1;
				break;
			}
			loader->offset = pos;
			break;
		case SEEK_CUR:
			if ((pos >= 0 && pos + loader->offset >= loader->size) || (pos < 0 && (long)(pos + loader->offset) < 0)) {
				loader->error = 1;
				break;
			}
			loader->offset += pos;
			break;
		case SEEK_END:
			if (pos > 0 || (long)(pos + loader->size) < 0) {
				loader->error = 1;
				break;
			}
			loader->offset = pos + loader->size;
			break;
		default:
			loader->error = 1;
			break;
	}

	loader->ptr = loader->data + loader->offset;

	return(loader->error);
}

int loader_error(struct _memloader *loader) {
	return(loader->error);
}