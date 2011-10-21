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
#include <string.h> // memcpy

struct ROPal *pal_loadFromData(const unsigned char *data, unsigned int length) {
	struct ROPal *ret;

	if (length < sizeof(struct ROPal)) {
		return(NULL);
	}

	ret = (struct ROPal*)_xalloc(sizeof(struct ROPal));
	memcpy(ret, data, sizeof(struct ROPal));

	return(ret);
}

struct ROPal *pal_loadFromGrf(struct ROGrfFile *file) {
	struct ROPal *ret = NULL;
	if (file->data == NULL) {
		grf_getdata(file);
		if (file->data != NULL) {
			ret = pal_loadFromData(file->data, file->uncompressedLength);
		}
		_xfree(file->data);
		file->data = NULL;
	}
	else {
		ret = pal_loadFromData(file->data, file->uncompressedLength);
	}

	return(ret);
}

void pal_unload(struct ROPal* pal) {
	if (pal == NULL)
		return;

	_xfree(pal);
}