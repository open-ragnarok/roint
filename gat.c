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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/// Maximum cell count. (original format uses int)
const unsigned int MAX_GAT_CELL_COUNT = ((unsigned int)0-1) / sizeof(struct ROGatCell);
const char GAT_MAGIC[4] = {'G','R','A','T'};


unsigned short gat_inspect(const struct ROGat *gat) {
	unsigned int cellcount;

	if (gat == NULL)
		return(0);

	if (_mul_over_limit(gat->width, gat->height, MAX_GAT_CELL_COUNT)) {
		_xlog("GAT dimensions are too big (%ux%u)\n", gat->width, gat->height);
		return(0);
	}

	cellcount = gat->width * gat->height;
	if (cellcount > 0 && gat->cells == NULL) {
		_xlog("expected NULL cells in GAT\n");
		return(0);
	}
	if (cellcount == 0 && gat->cells != NULL) {
		_xlog("expected non-NULL cells in GAT\n");
		return(0);
	}

	return(0x102); // v1.2
}


struct ROGat *gat_load(struct _reader *reader) {
	struct ROGat *ret;
	unsigned int cellcount;
	char magic[4];

	ret = (struct ROGat*)_xalloc(sizeof(struct ROGat));
	memset(ret, 0, sizeof(struct ROGat));

	reader->read(&magic, 4, 1, reader);
	if (strncmp(GAT_MAGIC, magic, 4) != 0) {
		_xlog("Invalid GAT header: '%c%c%c%c'\n", magic[0], magic[1], magic[2], magic[3]);
		gat_unload(ret);
		return(NULL);
	}

	reader->read(&ret->vermajor, 1, 1, reader);
	reader->read(&ret->verminor, 1, 1, reader);
	//_xlog("GAT Version: %u.%u\n", ret->vermajor, ret->verminor);
	if (ret->vermajor == 1 && ret->verminor == 2)
		;// supported
	else {
		_xlog("Unsupported GAT version (%u.%u)", ret->vermajor, ret->verminor);
		gat_unload(ret);
		return(NULL);
	}

	reader->read(&ret->width, 4, 1, reader);
	reader->read(&ret->height, 4, 1, reader);
	if (_mul_over_limit(ret->width, ret->height, MAX_GAT_CELL_COUNT)) {
		_xlog("GAT dimensions are too big (%ux%u)\n", ret->width, ret->height);
		gat_unload(ret);
		return(NULL);
	}
	cellcount = ret->width * ret->height;
	if (cellcount > 0) {
		ret->cells = (struct ROGatCell*)_xalloc(sizeof(struct ROGatCell) * cellcount);
		reader->read(ret->cells, sizeof(struct ROGatCell), cellcount, reader);
	}

	if (reader->error) {
		// data was missing
		_xlog("GAT is incomplete or invalid\n");
		gat_unload(ret);
		return(NULL);
	}

	return(ret);
}


struct ROGat *gat_loadFromData(const unsigned char *data, unsigned long length) {
	struct ROGat *ret;
	struct _reader *reader;

	reader = memreader_init(data, length);
	ret = gat_load(reader);
	reader->destroy(reader);

	return(ret);
}


struct ROGat *gat_loadFromFile(const char *fn) {
	struct ROGat *ret;
	struct _reader *reader;

	reader = filereader_init(fn);
	ret = gat_load(reader);
	reader->destroy(reader);

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


int gat_save(const struct ROGat *gat, struct _writer *writer) {
	unsigned int cellcount;

	if (gat == NULL || writer == NULL || writer->error)
		return(1);

	if (gat_inspect(gat) == 0) {
		_xlog("GAT is invalid\n");
		return(1);
	}
	if (gat->vermajor != 1 || gat->verminor != 2) {
		_xlog("unknown GAT version (v%u.%u)\n", gat->vermajor, gat->verminor);
		return(1);
	}

	writer->write(GAT_MAGIC, 4, 1, writer);
	writer->write(&gat->vermajor, 1, 1, writer);
	writer->write(&gat->verminor, 1, 1, writer);
	writer->write(&gat->width, 4, 1, writer);
	writer->write(&gat->height, 4, 1, writer);
	cellcount = gat->width * gat->height;
	if (cellcount > 0)
		writer->write(gat->cells, sizeof(struct ROGatCell), cellcount, writer);

	return(writer->error);
}


int gat_saveToData(const struct ROGat *gat, unsigned char **data_out, unsigned long *size_out) {
	int ret;
	struct _writer *writer;

	writer = memwriter_init(data_out, size_out);
	ret = gat_save(gat, writer);
	writer->destroy(writer);

	return(ret);
}


int gat_saveToFile(const struct ROGat *gat, const char *fn) {
	int ret;
	struct _writer *writer;

	writer = filewriter_init(fn);
	ret = gat_save(gat, writer);
	writer->destroy(writer);

	return(ret);
}


void gat_unload(struct ROGat *gat) {
	if (gat == NULL)
		return;

	if (gat->cells != NULL)
		_xfree(gat->cells);

	_xfree(gat);
}
