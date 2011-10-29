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


struct ROPal *pal_load(struct _reader *reader) {
	struct ROPal *ret;

	ret = (struct ROPal*)_xalloc(sizeof(struct ROPal));
	reader->read(ret, sizeof(struct ROPal), 1, reader);
	
	if (reader->error) {
		// data was missing
		_xlog("PAL is incomplete or invalid\n");
		pal_unload(ret);
		return(NULL);
	}

	return(ret);
}


struct ROPal *pal_loadFromData(const unsigned char *data, unsigned long length) {
	struct ROPal *ret;
	struct _reader *reader;

	reader = memreader_init(data, length);
	ret = pal_load(reader);
	reader->destroy(reader);

	return(ret);
}


struct ROPal *pal_loadFromFile(const char *fn) {
	struct ROPal *ret;
	struct _reader *reader;

	reader = filereader_init(fn);
	ret = pal_load(reader);
	reader->destroy(reader);

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


int pal_save(const struct ROPal *pal, struct _writer *writer) {
	if (pal == NULL || writer == NULL || writer->error)
		return(1);

	writer->write(pal, sizeof(struct ROPal), 1, writer);

	return(writer->error);
}


int pal_saveToData(const struct ROPal *pal, unsigned char **data, unsigned long *length) {
	int ret;
	struct _writer *writer;

	writer = memwriter_init(data, length);
	ret = pal_save(pal, writer);
	writer->destroy(writer);

	return(ret);
}


int pal_saveToFile(const struct ROPal *pal, const char *fn) {
	int ret;
	struct _writer *writer;

	writer = filewriter_init(fn);
	ret = pal_save(pal, writer);
	writer->destroy(writer);

	return(ret);
}


void pal_unload(struct ROPal* pal) {
	if (pal == NULL)
		return;

	_xfree(pal);
}
