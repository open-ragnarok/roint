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


struct ROGndColorChannel {
	unsigned char buffer[40];
};

struct ROGndLightmapIndex {
	unsigned int a;
	unsigned int r;
	unsigned int g;
	unsigned int b;
};


const unsigned int GND_MAX_CELL_COUNT = ((unsigned int)0-1) / sizeof(struct ROGndCell);
const char GND_MAGIC[4] = {'G','R','G','N'};


unsigned short gnd_inspect(const struct ROGnd *gnd) {
	unsigned int cellcount;
	unsigned int i;

	if (gnd == NULL) {
		_xlog("gnd.inspect : invalid argument (gnd=%p)\n", gnd);
		return(0);
	}

	if (_mul_over_limit(gnd->width, gnd->height, GND_MAX_CELL_COUNT)) {
		_xlog("gnd.inspect : dimensions are too big (%ux%u)\n", gnd->width, gnd->height);
		return(0);
	}
	// textures
	if (gnd->texturecount > 0 && gnd->textures == NULL) {
		_xlog("gnd.inspect : expected non-NULL textures\n");
		return(0);
	}
	if (gnd->texturecount == 0 && gnd->textures != NULL) {
		_xlog("gnd.inspect : expected NULL textures\n");
		return(0);
	}
	for (i = 0; i < gnd->texturecount; i++) {
		const char *texture = gnd->textures[i];
		if (texture == NULL) {
			_xlog("gnd.inspect : [%u] expected non-NULL texture\n", i);
			return(0);
		}
	}
	// lightmaps
	if (gnd->lightmapcount > 0 && gnd->lightmaps == NULL) {
		_xlog("gnd.inspect : expected non-NULL lightmaps\n");
		return(0);
	}
	if (gnd->lightmapcount == 0 && gnd->lightmaps != NULL) {
		_xlog("gnd.inspect : expected NULL lightmaps\n");
		return(0);
	}
	// surfaces
	if (gnd->surfacecount > 0 && gnd->surfaces == NULL) {
		_xlog("gnd.inspect : expected non-NULL surfaces\n");
		return(0);
	}
	if (gnd->surfacecount == 0 && gnd->surfaces != NULL) {
		_xlog("gnd.inspect : expected NULL surfaces\n");
		return(0);
	}
	// cells
	cellcount = gnd->width * gnd->height;
	if (cellcount > 0 && gnd->cells == NULL) {
		_xlog("gnd.inspect : expected non-NULL cells\n");
		return(0);
	}
	if (cellcount == 0 && gnd->cells != NULL) {
		_xlog("gnd.inspect : expected NULL cells\n");
		return(0);
	}

	return(0x107); // v1.7
}


struct ROGnd *gnd_load(struct _reader *reader) {
	struct ROGnd *ret;
	unsigned int i;
	unsigned int texturenamelen;
	unsigned int cellcount;
	char magic[4];

	if (reader == NULL || reader->error) {
		_xlog("gnd.load : invalid argument (reader=%p reader.error=%d)\n", reader, reader->error);
		return(NULL);
	}

	ret = (struct ROGnd*)_xalloc(sizeof(struct ROGnd));
	memset(ret, 0, sizeof(struct ROGnd));

	reader->read(&magic, 4, 1, reader);
	if (memcmp(GND_MAGIC, magic, 4) != 0) {
		_xlog("gnd.load : invalid header x%02X%02X%02X%02X (\"%-4s\")\n", magic[0], magic[1], magic[2], magic[3], magic);
		gnd_unload(ret);
		return(NULL);
	}

	reader->read(&ret->vermajor, 1, 1, reader);
	reader->read(&ret->verminor, 1, 1, reader);
	if (ret->version == 0x107) {
		// supported
	}
	else if (ret->version == 0x106) {
		_xlog("gnd.load : v1.6 not supported\n");
		gnd_unload(ret);
	}
	else {
		_xlog("gnd.load : unknown version v%u.%u\n", ret->vermajor, ret->verminor);
		gnd_unload(ret);
		return(NULL);
	}

	reader->read(&ret->width, 4, 1, reader);
	reader->read(&ret->height, 4, 1, reader);
	reader->read(&ret->zoom, 4, 1, reader);
	if (_mul_over_limit(ret->width, ret->height, GND_MAX_CELL_COUNT)) {
		_xlog("gnd.load : dimensions are too big (%ux%u)\n", ret->width, ret->height);
		gnd_unload(ret);
		return(NULL);
	}
	// read textures
	reader->read(&ret->texturecount, 4, 1, reader);
	reader->read(&texturenamelen, 4, 1, reader);
	if (ret->texturecount > 0) {
		char *buf;
		size_t len;

		ret->textures = (char**)_xalloc(sizeof(char*) * ret->texturecount);
		memset(ret->textures, 0, sizeof(char*) * ret->texturecount);
		buf = (char*)_xalloc(texturenamelen + 1);
		buf[texturenamelen] = 0;
		for (i = 0; i < ret->texturecount; i++) {
			if (texturenamelen > 0)
				reader->read(buf, 1, texturenamelen, reader);
			len = strlen(buf);
			ret->textures[i] = (char*)_xalloc(len + 1);
			memcpy(ret->textures[i], buf, len + 1);
		}
		_xfree(buf);
	}
	// read lightmaps
	reader->read(&ret->lightmapcount, 4, 1, reader);
	if (ret->version >= 0x107) {
		unsigned int lightmapWidth; // width, must be 8
		unsigned int lightmapHeight; // height, must be 8
		unsigned int lightmapCells; // cells, must be 1
		reader->read(&lightmapWidth, 4, 1, reader);
		reader->read(&lightmapHeight, 4, 1, reader);
		reader->read(&lightmapCells, 4, 1, reader);
		if (lightmapWidth != 8 || lightmapHeight != 8 || lightmapCells != 1) {
			_xlog("gnd.load : unsupported lightmap dimensions (width=%u height=%u cells=%u)\n", lightmapWidth, lightmapHeight, lightmapCells);
			gnd_unload(ret);
			return(NULL);
		}
		if (ret->lightmapcount > 0) {
			ret->lightmaps = (struct ROGndLightmap*)_xalloc(sizeof(struct ROGndLightmap) * ret->lightmapcount);
			reader->read(ret->lightmaps, sizeof(struct ROGndLightmap), ret->lightmapcount, reader);
		}
	}
	else {
		// generate lightmaps from v1.6 data
		struct ROGndLightmapIndex *lightmapIndexes = NULL;
		struct ROGndColorChannel *colorchannels = NULL;
		unsigned int colorchannelcount;
		unsigned char error = 0;

		if (ret->lightmapcount > 0) {
			lightmapIndexes = (struct ROGndLightmapIndex*)_xalloc(sizeof(struct ROGndLightmapIndex) * ret->lightmapcount);
			reader->read(lightmapIndexes, sizeof(struct ROGndLightmapIndex), ret->lightmapcount, reader);
		}
		reader->read(&colorchannelcount, 4, 1, reader);
		if (colorchannelcount > 0) {
			colorchannels = (struct ROGndColorChannel*)_xalloc(sizeof(struct ROGndColorChannel) * colorchannelcount);
			reader->read(colorchannels, sizeof(struct ROGndColorChannel), colorchannelcount, reader);
		}
		if (ret->lightmapcount > 0) {
			ret->lightmaps = (struct ROGndLightmap*)_xalloc(sizeof(struct ROGndLightmap) * ret->lightmapcount);
			for (i = 0; i < ret->lightmapcount; i++) {
				struct ROGndLightmap *lightmap = &ret->lightmaps[i];
				struct ROGndLightmapIndex *lightmapIndex = &lightmapIndexes[i];
				struct ROGndColorChannel *colorchannel_a;
				struct ROGndColorChannel *colorchannel_r;
				struct ROGndColorChannel *colorchannel_g;
				struct ROGndColorChannel *colorchannel_b;
				unsigned int j,aux;

				if (lightmapIndex->a >= colorchannelcount ||
					lightmapIndex->r >= colorchannelcount ||
					lightmapIndex->g >= colorchannelcount ||
					lightmapIndex->b >= colorchannelcount) {
					_xlog("gnd.load : [%u] invalid lightmapIndex (a=%u r=%u g=%u b=%u colorchannelcount=%u)\n", i, lightmapIndex->a, lightmapIndex->r, lightmapIndex->g, lightmapIndex->b, colorchannelcount);
					break;
				}
				colorchannel_a = &colorchannels[lightmapIndex->a];
				colorchannel_r = &colorchannels[lightmapIndex->r];
				colorchannel_g = &colorchannels[lightmapIndex->g];
				colorchannel_b = &colorchannels[lightmapIndex->b];
				for (j = 0, aux = 0; j < 8 * 8; j++, aux += 5) {
					unsigned int lightmap_i = (j / 8) + 8 * (j % 8);
					unsigned int low = aux % 8;
					unsigned int high = aux / 8;
					unsigned char value_a;
					unsigned char value_r;
					unsigned char value_g;
					unsigned char value_b;
					value_a = (colorchannel_a->buffer[high] & (0xF8 >> low)) << low;
					value_r = (colorchannel_r->buffer[high] & (0xF8 >> low)) << low;
					value_g = (colorchannel_g->buffer[high] & (0xF8 >> low)) << low;
					value_b = (colorchannel_b->buffer[high] & (0xF8 >> low)) << low;
					if (low >= 4) {
						value_a += ((0xF8 << (8 - low)) & colorchannel_a->buffer[high + 1]) >> (8 - low);
						value_r += ((0xF8 << (8 - low)) & colorchannel_r->buffer[high + 1]) >> (8 - low);
						value_g += ((0xF8 << (8 - low)) & colorchannel_g->buffer[high + 1]) >> (8 - low);
						value_b += ((0xF8 << (8 - low)) & colorchannel_b->buffer[high + 1]) >> (8 - low);
					}
					lightmap->brightness[lightmap_i] = value_a;
					lightmap->intensity[lightmap_i].r = value_r;
					lightmap->intensity[lightmap_i].g = value_g;
					lightmap->intensity[lightmap_i].b = value_b;
				}
			}
		}
		if (lightmapIndexes != NULL)
			_xfree(lightmapIndexes);
		if (colorchannels != NULL)
			_xfree(colorchannels);
		if (i != ret->lightmapcount) {
			_xlog("gnd.load : failed to generate lightmaps from v1.6 data\n");
			gnd_unload(ret);
			return(NULL);
		}
	}
	// read surfaces
	reader->read(&ret->surfacecount, 4, 1, reader);
	if (ret->surfacecount > 0) {
		ret->surfaces = (struct ROGndSurface*)_xalloc(sizeof(struct ROGndSurface) * ret->surfacecount);
		reader->read(ret->surfaces, sizeof(struct ROGndSurface), ret->surfacecount, reader);
	}
	// read cells
	cellcount = ret->width * ret->height;
	if (cellcount > 0) {
		ret->cells = (struct ROGndCell*)_xalloc(sizeof(struct ROGndCell) * cellcount);
		if (ret->version >= 0x107)
			reader->read(ret->cells, sizeof(struct ROGndCell), cellcount, reader);
		else {
			for (i = 0; i < cellcount; i++) {
				struct ROGndCell *cell = &ret->cells[i];
				short topSurfaceId;
				short frontSurfaceId;
				short rightSurfaceId;
				reader->read(&cell->height, sizeof(cell->height), 1, reader);
				reader->read(&topSurfaceId, 2, 1, reader);
				reader->read(&frontSurfaceId, 2, 1, reader);
				reader->read(&rightSurfaceId, 2, 1, reader);
				cell->topSurfaceId = topSurfaceId;
				cell->frontSurfaceId = frontSurfaceId;
				cell->rightSurfaceId = rightSurfaceId;
			}
		}
	}

	if (reader->error) {
		_xlog("gnd.load : read error\n");
		gnd_unload(ret);
		return(NULL);
	}

	return(ret);
}


struct ROGnd *gnd_loadFromData(const unsigned char *data, unsigned long length) {
	struct ROGnd *ret;
	struct _reader *reader;

	reader = memreader_init(data, length);
	ret = gnd_load(reader);
	reader->destroy(reader);

	return(ret);
}


struct ROGnd *gnd_loadFromFile(const char *fn) {
	struct ROGnd *ret;
	struct _reader *reader;

	reader = filereader_init(fn);
	ret = gnd_load(reader);
	reader->destroy(reader);

	return(ret);
}


struct ROGnd *gnd_loadFromGrf(struct ROGrfFile *file) {
	struct ROGnd *ret = NULL;
	if (file->data == NULL) {
		grf_getdata(file);
		if (file->data != NULL) {
			ret = gnd_loadFromData(file->data, file->uncompressedLength);
		}
		_xfree(file->data);
		file->data = NULL;
	}
	else {
		ret = gnd_loadFromData(file->data, file->uncompressedLength);
	}

	return(ret);
}


int gnd_save(const struct ROGnd *gnd, struct _writer *writer) {
	unsigned int i;
	unsigned int texturenamelen;
	unsigned int lightmapWidth;
	unsigned int lightmapHeight;
	unsigned int lightmapCells;
	unsigned int cellcount;

	if (gnd == NULL || writer == NULL || writer->error) {
		_xlog("gnd.save : invalid argument (gnd=%p writer=%p writer.error=%d)\n", gnd, writer, writer->error);
		return(1);
	}

	if (gnd_inspect(gnd) == 0) {
		_xlog("gnd.save : invalid\n");
		return(1);
	}
	if (gnd->vermajor != 1 || gnd->verminor != 7) {
		_xlog("gnd.save : unknown version v%u.%u\n", gnd->vermajor, gnd->verminor);
		return(1);
	}

	writer->write(GND_MAGIC, 4, 1, writer);
	writer->write(&gnd->vermajor, 1, 1, writer);
	writer->write(&gnd->verminor, 1, 1, writer);
	writer->write(&gnd->width, 4, 1, writer);
	writer->write(&gnd->height, 4, 1, writer);
	writer->write(&gnd->zoom, 4, 1, writer);
	// write textures
	writer->write(&gnd->texturecount, 4, 1, writer);
	texturenamelen = 1;
	for (i = 0; i < gnd->texturecount; i++) {
		const char *texture = gnd->textures[i];
		size_t zlen = strlen(texture) + 1;
		if (texturenamelen < zlen)
			texturenamelen = zlen;
	}
	writer->write(&texturenamelen, 4, 1, writer);
	if (gnd->texturecount > 0) {
		char *buf = (char*)_xalloc(sizeof(char) * texturenamelen);
		for (i = 0; i < gnd->texturecount; i++) {
			const char *texture = gnd->textures[i];
			size_t len = strlen(texture);
			memcpy(buf, texture, len);
			memset(buf + len, 0, texturenamelen - len);
			writer->write(buf, 1, texturenamelen, writer);
		}
		_xfree(buf);
	}
	// write lightmaps
	writer->write(&gnd->lightmapcount, 4, 1, writer);
	lightmapWidth = 8;
	lightmapHeight = 8;
	lightmapCells = 1;
	writer->write(&lightmapWidth, 4, 1, writer);
	writer->write(&lightmapHeight, 4, 1, writer);
	writer->write(&lightmapCells, 4, 1, writer);
	if (gnd->lightmapcount)
		writer->write(gnd->lightmaps, sizeof(struct ROGndLightmap), gnd->lightmapcount, writer);
	// write surfaces
	writer->write(&gnd->surfacecount, 4, 1, writer);
	if (gnd->surfacecount > 0)
		writer->write(gnd->surfaces, sizeof(struct ROGndSurface), gnd->surfacecount, writer);
	// write cells
	cellcount = gnd->width * gnd->height;
	if (cellcount > 0) {
		writer->write(gnd->cells, sizeof(struct ROGndCell), cellcount, writer);
	}

	if (writer->error) {
		_xlog("gnd.save : write error\n");
		return(1);
	}

	return(0);
}


int gnd_saveToData(const struct ROGnd *gnd, unsigned char **data_out, unsigned long *size_out) {
	int ret;
	struct _writer *writer;

	writer = memwriter_init(data_out, size_out);
	ret = gnd_save(gnd, writer);
	writer->destroy(writer);

	return(ret);
}


int gnd_saveToFile(const struct ROGnd *gnd, const char *fn) {
	int ret;
	struct _writer *writer;

	writer = filewriter_init(fn);
	ret = gnd_save(gnd, writer);
	writer->destroy(writer);

	return(ret);
}


void gnd_unload(struct ROGnd *gnd) {
	unsigned int i;

	if (gnd == NULL)
		return;

	if (gnd->textures != NULL) {
		for (i = 0; i < gnd->texturecount; i++)
			if (gnd->textures[i] != NULL)
				_xfree(gnd->textures[i]);
		_xfree(gnd->textures);
	}

	if (gnd->lightmaps != NULL)
		_xfree(gnd->lightmaps);

	if (gnd->surfaces != NULL)
		_xfree(gnd->surfaces);

	if (gnd->cells != NULL)
		_xfree(gnd->cells);

	_xfree(gnd);
}
