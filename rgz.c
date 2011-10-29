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


struct RORgz *rgz_load(struct _reader *reader) {
	struct RORgz *ret;
	unsigned int entrylimit;
	struct _reader *gzipreader;

	gzipreader = deflatereader_init(reader, 1); // gzip only
	if (gzipreader->error) {
		_xlog("rgz.load : gzipreader init failed\n");
		gzipreader->destroy(gzipreader);
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

		gzipreader->read(&entry->type, 1, 1, gzipreader);
		if (gzipreader->error)
			break;

		gzipreader->read(&pathlen, 1, 1, gzipreader);
		if (gzipreader->error)
			break;

		if (pathlen > 0) {
			gzipreader->read(&entry->path, 1, pathlen, gzipreader);
			if (gzipreader->error)
				break;
			entry->path[pathlen - 1] = 0;
		}

		if (entry->type == 'f') {
			gzipreader->read(&entry->datalength, 4, 1, gzipreader);
			if (gzipreader->error)
				break;

			if (entry->datalength > 0) {
				entry->data = (unsigned char*)_xalloc(sizeof(unsigned char) * entry->datalength);
				gzipreader->read(&entry->data, 1, entry->datalength, gzipreader);
				if (gzipreader->error)
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
	if (ret != NULL && gzipreader->error) {
		_xlog("rgz.load : read error\n");
		rgz_unload(ret);
		ret = NULL;
	}
	if (ret != NULL && ret->entrycount != entrylimit) {
		struct RORgzEntry *tmp = ret->entries;
		ret->entries = (struct RORgzEntry*)_xalloc(sizeof(struct RORgzEntry) * ret->entrycount);
		memcpy(ret->entries, tmp, sizeof(struct RORgzEntry) * ret->entrycount);
		_xfree(tmp);
	}
	gzipreader->destroy(gzipreader);

	return(ret);
}


struct RORgz *rgz_loadFromData(const unsigned char *data, unsigned int length) {
	struct RORgz *ret;
	struct _reader *reader;

	reader = memreader_init(data, length);
	ret = rgz_load(reader);
	reader->destroy(reader);

	return(ret);
}


struct RORgz *rgz_loadFromFile(const char *fn) {
	struct RORgz *ret;
	struct _reader *reader;

	reader = filereader_init(fn);
	ret = rgz_load(reader);
	reader->destroy(reader);

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
