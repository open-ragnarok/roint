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


const unsigned int STR_MAX_LAYER_COUNT = ((unsigned int)0-1) / sizeof(struct ROStrLayer);
const unsigned int STR_MAX_TEXTURE_COUNT = ((unsigned int)0-1) / sizeof(struct ROStrTexture);
const unsigned int STR_MAX_KEYFRAME_COUNT = ((unsigned int)0-1) / sizeof(struct ROStrKeyFrame);
const char STR_MAGIC[4] = {'S','T','R','M'};


unsigned int str_inspect(const struct ROStr *str) {
	unsigned int layerId, textureId;

	if (str == NULL) {
		_xlog("str.inspect : invalid argument (str=%p)\n", str);
		return(0);
	}

	if (str->layercount > 0 && str->layers == NULL) {
		_xlog("str.inspect : expected non-NULL textures\n");
		return(0);
	}
	if (str->layercount == 0 && str->layers != NULL) {
		_xlog("str.inspect : expected NULL textures\n");
		return(0);
	}
	if (str->layercount > STR_MAX_LAYER_COUNT) {
		_xlog("str.inspect : too many layers (%u)\n", str->layercount);
		return(0);
	}
	for (layerId = 0; layerId < str->layercount; layerId++) {
		const struct ROStrLayer *layer = &str->layers[layerId];
		if (layer->texturecount > 0 && layer->textures == NULL) {
			_xlog("str.inspect : [%u] expected non-NULL textures\n", layerId);
			return(0);
		}
		if (layer->texturecount == 0 && layer->textures != NULL) {
			_xlog("str.inspect : [%u] expected NULL textures\n", layerId);
			return(0);
		}
		for (textureId = 0; textureId < layer->texturecount; textureId++) {
			const struct ROStrTexture *texture = &layer->textures[textureId];
			if (memchr(texture->name, 0, sizeof(texture->name)) == NULL) {
				_xlog("str.inspect : [%u] not NUL-terminated\n", layerId, textureId);
				return(0);
			}
		}
		if (layer->keyframecount > 0 && layer->keyframes == NULL) {
			_xlog("str.inspect : [%u] expected non-NULL keyframes\n", layerId);
			return(0);
		}
		if (layer->keyframecount == 0 && layer->keyframes != NULL) {
			_xlog("str.inspect : [%u] expected NULL keyframes\n", layerId);
			return(0);
		}
	}

	return(148);
}


struct ROStr *str_load(struct _reader *reader) {
	struct ROStr *ret;
	unsigned int layerId;
	char magic[4];

	if (reader == NULL || reader->error) {
		_xlog("str.load : invalid argument (reader=%p reader.error=%d)\n", reader, reader->error);
		return(NULL);
	}

	ret = (struct ROStr*)_xalloc(sizeof(struct ROStr));
	memset(ret, 0, sizeof(struct ROStr));

	reader->read(&magic, 4, 1, reader);
	if (memcmp(STR_MAGIC, magic, 4) != 0) {
		_xlog("str.load : invalid header x%02X%02X%02X%02X (\"%-4s\")\n", magic[0], magic[1], magic[2], magic[3], magic);
		str_unload(ret);
		return(NULL);
	}

	reader->read(&ret->version, 4, 1, reader);
	if (ret->version == 148)
		;// supported
	else {
		_xlog("str.load : unknown version v%u\n", ret->version);
		str_unload(ret);
		return(NULL);
	}

	reader->read(&ret->framecount, 4, 1, reader);
	reader->read(&ret->fps, 4, 1, reader);
	reader->read(&ret->layercount, 4, 1, reader);
	if (ret->layercount > STR_MAX_LAYER_COUNT) {
		_xlog("str.load : too many layers (%u)\n", ret->layercount);
		str_unload(ret);
		return(NULL);
	}
	reader->read(ret->reserved, 16, 1, reader);
	if (ret->layercount > 0) {
		ret->layers = (struct ROStrLayer*)_xalloc(sizeof(struct ROStrLayer) * ret->layercount);
		memset(ret->layers, 0, sizeof(struct ROStrLayer) * ret->layercount);
		for (layerId = 0; layerId < ret->layercount; layerId++) {
			struct ROStrLayer *layer = &ret->layers[layerId];
			reader->read(&layer->texturecount, 4, 1, reader);
			if (layer->texturecount > STR_MAX_TEXTURE_COUNT) {
				_xlog("str.load : [%u] too many textures (%u)\n", layerId, layer->texturecount);
				str_unload(ret);
				return(NULL);
			}
			if (layer->texturecount > 0) {
				layer->textures = (struct ROStrTexture*)_xalloc(sizeof(struct ROStrTexture) * layer->texturecount);
				reader->read(layer->textures, sizeof(struct ROStrTexture), layer->texturecount, reader);
			}
			reader->read(&layer->keyframecount, 4, 1, reader);
			if (layer->keyframecount > STR_MAX_KEYFRAME_COUNT) {
				_xlog("str.load : [%u] too many keyframes (%u)\n", layerId, layer->keyframecount);
				str_unload(ret);
				return(NULL);
			}
			if (layer->keyframecount > 0) {
				layer->keyframes = (struct ROStrKeyFrame*)_xalloc(sizeof(struct ROStrKeyFrame) * layer->keyframecount);
				reader->read(layer->keyframes, sizeof(struct ROStrKeyFrame), layer->keyframecount, reader);
			}
		}
	}

	if (reader->error) {
		_xlog("str.load : read error\n");
		str_unload(ret);
		return(NULL);
	}

	return(ret);
}


struct ROStr *str_loadFromData(const unsigned char *data, unsigned long length) {
	struct ROStr *ret;
	struct _reader *reader;

	reader = memreader_init(data, length);
	ret = str_load(reader);
	reader->destroy(reader);

	return(ret);
}


struct ROStr *str_loadFromFile(const char *fn) {
	struct ROStr *ret;
	struct _reader *reader;

	reader = filereader_init(fn);
	ret = str_load(reader);
	reader->destroy(reader);

	return(ret);
}


struct ROStr *str_loadFromGrf(struct ROGrfFile *file) {
	struct ROStr *ret = NULL;
	if (file->data == NULL) {
		grf_getdata(file);
		if (file->data != NULL) {
			ret = str_loadFromData(file->data, file->uncompressedLength);
		}
		_xfree(file->data);
		file->data = NULL;
	}
	else {
		ret = str_loadFromData(file->data, file->uncompressedLength);
	}

	return(ret);
}


int str_save(const struct ROStr *str, struct _writer *writer) {
	unsigned int layerId, textureId;

	if (str == NULL || writer == NULL || writer->error) {
		_xlog("str.save : invalid argument (str=%p writer=%p writer.error=%d)\n", str, writer, writer->error);
		return(1);
	}

	if (str_inspect(str) == 0) {
		_xlog("str.save : invalid\n");
		return(1);
	}
	if (str->version != 148) {
		_xlog("str.save : unknown version v%u\n", str->version);
		return(1);
	}

	writer->write(STR_MAGIC, 4, 1, writer);
	writer->write(&str->version, 4, 1, writer);
	writer->write(&str->framecount, 4, 1, writer);
	writer->write(&str->fps, 4, 1, writer);
	writer->write(&str->layercount, 4, 1, writer);
	writer->write(str->reserved, 16, 1, writer);
	for (layerId = 0; layerId < str->layercount; layerId++) {
		const struct ROStrLayer *layer = &str->layers[layerId];
		writer->write(&layer->texturecount, 4, 1, writer);
		for (textureId = 0; textureId < layer->texturecount; textureId++) {
			struct ROStrTexture texture;
			memset(&texture, 0, sizeof(struct ROStrTexture));
			strcpy(texture.name, layer->textures[textureId].name);
			writer->write(&texture, sizeof(struct ROStrTexture), 1, writer);
		}
		writer->write(&layer->keyframecount, 4, 1, writer);
		if (layer->keyframecount > 0)
			writer->write(layer->keyframes, sizeof(struct ROStrKeyFrame), layer->keyframecount, writer);
	}

	if (writer->error) {
		_xlog("str.save : write error\n");
		return(1);
	}

	return(0);
}


int str_saveToData(const struct ROStr *str, unsigned char **data_out, unsigned long *size_out) {
	int ret;
	struct _writer *writer;

	writer = memwriter_init(data_out, size_out);
	ret = str_save(str, writer);
	writer->destroy(writer);

	return(ret);
}


int str_saveToFile(const struct ROStr *str, const char *fn) {
	int ret;
	struct _writer *writer;

	writer = filewriter_init(fn);
	ret = str_save(str, writer);
	writer->destroy(writer);

	return(ret);
}


void str_unload(struct ROStr *str) {
	unsigned int layerId;

	if (str == NULL)
		return;

	if (str->layers != NULL) {
		for (layerId = 0; layerId < str->layercount; layerId++) {
			if (str->layers[layerId].textures != NULL)
				_xfree(str->layers[layerId].textures);
			if (str->layers[layerId].keyframes != NULL)
				_xfree(str->layers[layerId].keyframes);
		}
		_xfree(str->layers);
	}

	_xfree(str);
}
