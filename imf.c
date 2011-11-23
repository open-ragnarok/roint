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

const unsigned int IMF_MAX_LAYER_COUNT = ((unsigned int)0-1) / sizeof(struct ROImfLayer);
const unsigned int IMF_MAX_ACTION_COUNT = ((unsigned int)0-1) / sizeof(struct ROImfAction);
const unsigned int IMF_MAX_MOTION_COUNT = ((unsigned int)0-1) / sizeof(struct ROImfMotion);


float imf_inspect(const struct ROImf *imf) {
	unsigned int layerId, actionId;

	if (imf == NULL) {
		_xlog("imf.inspect : invalid argument (imf=%p)\n", imf);
		return(0.0f);
	}

	if (imf->lastlayer < -1) {
		_xlog("imf.inspect : invalid lastlayer (%d)\n", imf->lastlayer);
		return(0.0f);
	}
	if (imf->lastlayer == -1 && imf->layers != NULL) {
		_xlog("imf.inspect : expected NULL layers\n");
		return(0.0f);
	}
	if (imf->lastlayer > -1) {
		unsigned int layercount = (unsigned int)imf->lastlayer + 1;
		if (imf->layers == NULL) {
			_xlog("imf.inspect : expected non-NULL layers\n");
			return(0.0f);
		}
		if (layercount > IMF_MAX_LAYER_COUNT) {
			_xlog("imf.inspect : too many layers (%u)\n", layercount);
			return(0.0f);
		}
		for (layerId = 0; layerId < layercount; layerId++) {
			const struct ROImfLayer *layer = &imf->layers[layerId];
			if (layer->actioncount > 0 && layer->actions == NULL) {
				_xlog("imf.inspect : [%u] expected non-NULL actions\n", layerId);
				return(0.0f);
			}
			if (layer->actioncount == 0 && layer->actions != NULL) {
				_xlog("imf.inspect : [%u] expected NULL actions\n", layerId);
				return(0.0f);
			}
			if (layer->actioncount > IMF_MAX_ACTION_COUNT) {
				_xlog("imf.inspect : [%u] too many actions (%u)\n", layerId, layer->actioncount);
				return(0.0f);
			}
			for (actionId = 0; actionId < layer->actioncount; actionId++) {
				const struct ROImfAction *action = &layer->actions[actionId];
				if (action->motioncount > 0 && action->motions == NULL) {
					_xlog("imf.inspect : [%u][%u] expected non-NULL motions\n", layerId, actionId);
					return(0.0f);
				}
				if (action->motioncount == 0 && action->motions != NULL) {
					_xlog("imf.inspect : [%u][%u] expected NULL motions\n", layerId, actionId);
					return(0.0f);
				}
				if (action->motioncount > IMF_MAX_MOTION_COUNT) {
					_xlog("imf.inspect : [%u][%u] too many actions (%u)\n", layerId, actionId, action->motioncount);
					return(0.0f);
				}
			}
		}
	}

	return(1.01f);
}


struct ROImf *imf_load(struct _reader *reader) {
	struct ROImf *ret;
	unsigned int layerId, actionId;

	if (reader == NULL || reader->error) {
		_xlog("imf.load : invalid argument (reader=%p reader.error=%d)\n", reader, reader->error);
		return(NULL);
	}

	ret = (struct ROImf*)_xalloc(sizeof(struct ROImf));
	memset(ret, 0, sizeof(struct ROImf));

	reader->read(&ret->version, 4, 1, reader);
	if (ret->version == 1.01f)
		;// supported
	else {
		_xlog("imf.load : unknown version v%f\n", ret->version);
		imf_unload(ret);
		return(NULL);
	}

	reader->read(&ret->checksum, 4, 1, reader);
	reader->read(&ret->lastlayer, 4, 1, reader);
	if (ret->lastlayer >= 0) {
		unsigned int layercount = (unsigned int)ret->lastlayer + 1;
		if (layercount > IMF_MAX_LAYER_COUNT) {
			_xlog("imf.load : too many layers (%u)\n", layercount);
			imf_unload(ret);
			return(NULL);
		}
		ret->layers = (struct ROImfLayer*)_xalloc(sizeof(struct ROImfLayer) * layercount);
		memset(ret->layers, 0, sizeof(struct ROImfLayer) * layercount);
		for (layerId = 0; layerId < layercount; layerId++) {
			struct ROImfLayer *layer = &ret->layers[layerId];
			reader->read(&layer->actioncount, 4, 1, reader);
			if (layer->actioncount > IMF_MAX_ACTION_COUNT) {
				_xlog("imf.load : [%u] too many actions (%u)\n", layerId, layer->actioncount);
				imf_unload(ret);
				return(NULL);
			}
			if (layer->actioncount > 0) {
				layer->actions = (struct ROImfAction*)_xalloc(sizeof(struct ROImfAction) * layer->actioncount);
				memset(layer->actions, 0, sizeof(struct ROImfAction) * layer->actioncount);
				for (actionId = 0; actionId < layer->actioncount; actionId++) {
					struct ROImfAction *action = &layer->actions[actionId];
					reader->read(&action->motioncount, 4, 1, reader);
					if (action->motioncount > IMF_MAX_MOTION_COUNT) {
						_xlog("imf.load : [%u][%u] too many motions (%u)\n", layerId, actionId, action->motioncount);
						imf_unload(ret);
						return(NULL);
					}
					if (action->motioncount > 0) {
						action->motions = (struct ROImfMotion*)_xalloc(sizeof(struct ROImfMotion) * action->motioncount);
						reader->read(action->motions, sizeof(struct ROImfMotion), action->motioncount, reader);
					}
				}
			}
		}
	}

	if (reader->error) {
		_xlog("imf.load : read error\n");
		imf_unload(ret);
		return(NULL);
	}

	return(ret);
}


struct ROImf *imf_loadFromData(const unsigned char *data, unsigned long length) {
	struct ROImf *ret;
	struct _reader *reader;

	reader = memreader_init(data, length);
	ret = imf_load(reader);
	reader->destroy(reader);

	return(ret);
}


struct ROImf *imf_loadFromFile(const char *fn) {
	struct ROImf *ret;
	struct _reader *reader;

	reader = filereader_init(fn);
	ret = imf_load(reader);
	reader->destroy(reader);

	return(ret);
}


struct ROImf *imf_loadFromGrf(struct ROGrfFile *file) {
	struct ROImf *ret = NULL;
	if (file->data == NULL) {
		grf_getdata(file);
		if (file->data != NULL) {
			ret = imf_loadFromData(file->data, file->uncompressedLength);
		}
		_xfree(file->data);
		file->data = NULL;
	}
	else {
		ret = imf_loadFromData(file->data, file->uncompressedLength);
	}

	return(ret);
}


int imf_save(const struct ROImf *imf, struct _writer *writer) {
	unsigned int layerId, actionId;
	unsigned int layercount;

	if (imf == NULL || writer == NULL || writer->error) {
		_xlog("imf.save : invalid argument (imf=%p writer=%p writer.error=%d)\n", imf, writer, writer->error);
		return(1);
	}

	if (imf_inspect(imf) == 0.0f) {
		_xlog("imf.save : invalid\n");
		return(1);
	}
	if (imf->version != 1.01f) {
		_xlog("imf.save : unknown version v%f\n", imf->version);
		return(1);
	}

	writer->write(&imf->version, 4, 1, writer);
	writer->write(&imf->checksum, 4, 1, writer);
	writer->write(&imf->lastlayer, 4, 1, writer);
	layercount = (unsigned int)(imf->lastlayer + 1);
	for (layerId = 0; layerId < layercount; layerId++) {
		const struct ROImfLayer *layer = &imf->layers[layerId];
		writer->write(&layer->actioncount, 4, 1, writer);
		for (actionId = 0; actionId < layer->actioncount; actionId++) {
			const struct ROImfAction *action = &layer->actions[actionId];
			writer->write(&action->motioncount, 4, 1, writer);
			writer->write(action->motions, sizeof(struct ROImfMotion), action->motioncount, writer);
		}
	}

	if (writer->error) {
		_xlog("imf.save : write error\n");
		return(1);
	}

	return(0);
}


int imf_saveToData(const struct ROImf *imf, unsigned char **data_out, unsigned long *size_out) {
	int ret;
	struct _writer *writer;

	writer = memwriter_init(data_out, size_out);
	ret = imf_save(imf, writer);
	writer->destroy(writer);

	return(ret);
}


int imf_saveToFile(const struct ROImf *imf, const char *fn) {
	int ret;
	struct _writer *writer;

	writer = filewriter_init(fn);
	ret = imf_save(imf, writer);
	writer->destroy(writer);

	return(ret);
}


void imf_unload(struct ROImf *imf) {
	unsigned int layerId, actionId;

	if (imf == NULL)
		return;

	if (imf->layers != NULL) {
		unsigned int layercount = (imf->lastlayer >= 0)? (unsigned int)imf->lastlayer + 1: 0;
		for (layerId = 0; layerId < layercount; layerId++) {
			if (imf->layers[layerId].actions != NULL) {
				for (actionId = 0; actionId < imf->layers[layerId].actioncount; actionId++) {
					if (imf->layers[layerId].actions[actionId].motions != NULL)
						_xfree(imf->layers[layerId].actions[actionId].motions);
				}
				_xfree(imf->layers[layerId].actions);
			}
		}
		_xfree(imf->layers);
	}

	_xfree(imf);
}
