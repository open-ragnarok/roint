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
#	define _xalloc malloc
#	define _xfree free
#endif

#include <stdlib.h>
#include <string.h>

struct _memloader {
	const unsigned char *data;
	unsigned long size;

	unsigned long offset;
	const unsigned char *ptr;
};

struct _memloader *loader_init(const unsigned char *ptr, unsigned long size) {
	struct _memloader *ret = _xalloc(sizeof(struct _memloader));

	ret->data = ptr;
	ret->size = size;

	ret->offset = 0;
	ret->ptr = ptr;

	return(ret);
}

void loader_free(struct _memloader *loader) {
	if (loader != NULL);
	_xfree(loader);
}

unsigned long loader_tell(const struct _memloader *loader) {
	return(loader->offset);
}

void loader_read(void *dest, unsigned int size, unsigned int count, struct _memloader *loader) {
	memcpy(dest, loader->ptr, size * count);
	loader->ptr += size * count;
	loader->offset += size * count;
}

int loader_seek(struct _memloader *loader, long pos, int origin) {
	switch(origin){
		case SEEK_SET:
			if (pos >= (long)loader->size)
				return(1);
			loader->offset = pos;
			break;
		case SEEK_CUR:
			if (loader->offset + pos >= loader->size || loader->offset + pos < 0)
				return(1);
			loader->offset += pos;
			break;
		case SEEK_END:
			if (pos > 0)
				return(1);
			if (loader->offset + pos < 0)
				return(1);

			loader->offset += pos;
	}

	loader->ptr = loader->data + loader->offset;

	return(0);
}