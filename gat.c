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
#include "memloader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct ROGat *gat_loadFromData(const unsigned char *data, unsigned int length) {
	struct ROGat *ret;
	struct _memloader *loader;
	unsigned int cellcount;
	char magic[4];

	ret = (struct ROGat*)_xalloc(sizeof(struct ROGat));
	memset(ret, 0, sizeof(struct ROGat));
	loader = loader_init(data, length);

	loader_read(&magic, 4, 1, loader);
	if (strncmp("GRAT", magic, 4) != 0) {
		_xlog("Invalid GAT header: '%c%c%c%c'\n", magic[0], magic[1], magic[2], magic[3]);
		gat_unload(ret);
		loader_free(loader);
		return(NULL);
	}

	loader_read(&ret->vermajor, 1, 1, loader);
	loader_read(&ret->verminor, 1, 1, loader);
	_xlog("GAT Version: %u.%u\n", ret->vermajor, ret->verminor);
	if (ret->vermajor == 1 && ret->verminor == 2)
		;// supported
	else {
		_xlog("Unsupported GAT version (%u.%u)", ret->vermajor, ret->verminor);
		gat_unload(ret);
		loader_free(loader);
		return(NULL);
	}

	loader_read(&ret->width, 4, 1, loader);
	loader_read(&ret->height, 4, 1, loader);
	cellcount = ret->width * ret->height;
	if (cellcount > 0) {
		ret->cells = (struct ROGatCell*)_xalloc(sizeof(struct ROGatCell) * cellcount);
		loader_read(ret->cells, sizeof(struct ROGatCell), cellcount, loader);
	}

	if (loader_error(loader)) {
		// data was missing
		_xlog("GAT is incomplete or invalid\n");
		gat_unload(ret);
		loader_free(loader);
		return(NULL);
	}
	loader_free(loader);

	return(ret);
}


struct ROGat *gat_loadFromFile(const char *fn) {
	FILE *fp;
	unsigned char *data;
	long length;
	struct ROGat *ret;

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

	data = (unsigned char*)malloc((unsigned int)length);
	fseek(fp, 0, SEEK_SET);
	clearerr(fp);
	fread(data, (unsigned int)length, 1, fp);
	if (ferror(fp)) {
		_xlog("%s : %s\n", fn, strerror(errno));
		free(data);
		fclose(fp);
		return(NULL);
	}

	ret = gat_loadFromData(data, (unsigned int)length);
	free(data);
	fclose(fp);

	return(ret);
}


struct ROGat *gat_loadFromGrf(struct ROGrfFile *file) {
	struct ROGat *ret = NULL;
	if (file->data == NULL) {
		grf_getdata(file);
		if (file->data != NULL) {
			ret = gat_loadFromData(file->data, file->uncompressedLength);
		}
		_xfree(file->data);
		file->data = NULL;
	}
	else {
		ret = gat_loadFromData(file->data, file->uncompressedLength);
	}

	return(ret);
}


void gat_unload(struct ROGat* gat) {
	if (gat == NULL)
		return;

	if (gat->cells != NULL)
		_xfree(gat->cells);

	_xfree(gat);
}
