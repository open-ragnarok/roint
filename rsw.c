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

const char RSW_MAGIC[4] = {'G','R','S','W'};

void RswReadQuadtree(struct RORswQuadTreeNode* quadtree, struct _reader *reader, unsigned int level, unsigned int *i);
struct RORsw *rsw_load(struct _reader *reader);

struct RORsw *rsw_load(struct _reader *reader) {
	int i;
	struct RORsw *ret;
	char magic[4];
	struct RORswObject* rswobj;

	if (reader == NULL || reader->error) {
		_xlog("rsw.load : invalid argument (reader=%p reader.error=%d)\n", reader, reader->error);
		return(NULL);
	}

	ret = (struct RORsw*)_xalloc(sizeof(struct RORsw));
	ret->objects = NULL; // Just to be sure
	ret->quadtree = NULL;
	memset(ret, 0, sizeof(struct RORsw));

	reader->read(&magic, 4, 1, reader);
	if (memcmp(RSW_MAGIC, magic, 4) != 0) {
		_xlog("rsw.load : invalid header x%02X%02X%02X%02X (\"%-4s\")\n", magic[0], magic[1], magic[2], magic[3], magic);
		rsw_unload(ret);
		return(NULL);
	}

	reader->read(&ret->vermajor, 1, 1, reader);
	reader->read(&ret->verminor, 1, 1, reader);

	if (ret->vermajor == 1 && ret->verminor >= 2 && ret->verminor <= 9) {
		// supported [1.2 1.9]
	}
	else if (ret->vermajor == 2 && ret->verminor <= 2) {
		// supported [2.0 2.1]
	}
	else {
		_xlog("I don't know how to properly read rsw version %d.%d, but i'm gonna try...\n", ret->vermajor, ret->verminor);
	}

	reader->read(ret->m_iniFile, 40, 1, reader);
	reader->read(ret->m_gndFile, 40, 1, reader);
	if (ret->vermajor >= 2 || (ret->vermajor == 1 && ret->verminor >= 4)) { // v > 1.4
		reader->read(ret->m_gatFile, 40, 1, reader);
	}
	else {
		memset(ret->m_gatFile, 0, 40);
	}

	reader->read(ret->m_scrFile, 40, 1, reader);

	// make strings finish where they are supposed to.
	ret->m_iniFile[39] = ret->m_gndFile[39] = ret->m_gatFile[39] = ret->m_scrFile[39] = 0;

	// == == == WATER == == ==
	if (ret->vermajor >= 2 || (ret->vermajor == 1 && ret->verminor >= 3)) { // v > 1.3
		reader->read(&(ret->water), sizeof(float), 1, reader);
	}
	else {
		ret->water.level = 0.0f;
	}

	if (ret->vermajor >= 2 || (ret->vermajor == 1 && ret->verminor >= 8)) { // v > 1.8
		reader->read(&ret->water.type, sizeof(int), 1, reader);
		reader->read(&ret->water.waveHeight, sizeof(float), 3, reader); // reads waveHeight, waveSpeed and wavePitch
	}
	else {
		ret->water.type = 0;
		ret->water.waveHeight = 1.0f;
		ret->water.waveSpeed = 2.0f;
		ret->water.wavePitch = 50.0f;
	}
	
	if (ret->vermajor >= 2 || (ret->vermajor == 1 && ret->verminor >= 9)) { // v > 1.9
		reader->read(&ret->water.animSpeed, sizeof(int), 1, reader);
	}
	else {
		ret->water.animSpeed = 3;
	}


	// == == == LIGHT == == ==
	if (ret->vermajor >= 2 || (ret->vermajor == 1 && ret->verminor >= 5)) { // v > 1.5
		reader->read(&ret->light.longitude, sizeof(int), 2, reader); // longitude & latitude
		reader->read(&ret->light.diffuse, sizeof(float), 6, reader); // diffuse[3] & ambient[3]
	}
	else {
		ret->light.longitude = 45;
		ret->light.latitude = 45;
		ret->light.diffuse[0] = ret->light.diffuse[1] = ret->light.diffuse[2] = 1.0f;
		ret->light.ambient[0] = ret->light.ambient[1] = ret->light.ambient[2] = 0.3f;
	}

	if (ret->vermajor >= 2 || (ret->vermajor == 1 && ret->verminor >= 7)) { // v > 1.7
		reader->read(&ret->light.ignored, sizeof(float), 1, reader);
	}

	// == == == GROUND == == ==
	if (ret->vermajor >= 2 || (ret->vermajor == 1 && ret->verminor >= 6)) { // v > 1.6
		reader->read(ret->ground.gnd, sizeof(int), 4, reader);
	}
	else {
		ret->ground.top = -500;
		ret->ground.bottom = 500;
		ret->ground.left = -500;
		ret->ground.right = 500;
	}

	// == == == OBJECTS == == ==
	reader->read(&ret->obj_count, sizeof(int), 1, reader);
	ret->objects = (struct RORswObject*)_xalloc(sizeof(struct RORswObject) * ret->obj_count);
	for (i = 0; i < ret->obj_count; i++) {
		rswobj = &ret->objects[i];
		reader->read(&rswobj->type, sizeof(int), 1, reader);
		switch(rswobj->type) {
		case RORSW_OBJECT_MODEL:	// Model
			if (ret->vermajor >= 2 || (ret->vermajor == 1 && ret->verminor >= 3)) { // v > 1.3
				reader->read(rswobj->model.name, 40, 1, reader);
				reader->read(&rswobj->model.animType, sizeof(int), 1, reader);
				reader->read(&rswobj->model.animSpeed, sizeof(float), 1, reader);
				reader->read(&rswobj->model.blockType, sizeof(int), 1, reader);
				// Sanity settings
				rswobj->model.name[39] = 0;
				if (rswobj->model.animSpeed < 0.0f || rswobj->model.animSpeed >= 100.0f)
					rswobj->model.animSpeed = 1.0f;
			}
			else {
				rswobj->model.name[0] = 0;
				rswobj->model.animType = 0;
				rswobj->model.animSpeed = 1.0f;
				rswobj->model.blockType = 0;
			}
			reader->read(rswobj->model.modelName, 80, 1, reader);
			reader->read(rswobj->model.nodeName, 80, 1, reader);
			reader->read(&rswobj->model.pos, sizeof(float), 3, reader);
			reader->read(&rswobj->model.rot, sizeof(float), 3, reader);
			reader->read(&rswobj->model.scale, sizeof(float), 3, reader);
			break;
		case RORSW_OBJECT_LIGHT:	// Light
			reader->read(rswobj->light.name, 80, 1, reader);
			reader->read(rswobj->light.pos, sizeof(float), 3, reader);
			reader->read(rswobj->light.color, sizeof(int), 3, reader);
			reader->read(&rswobj->light.range, sizeof(float), 1, reader);

			// Sanity
			rswobj->light.name[39] = 0;
			break;
		case RORSW_OBJECT_SOUND:	// Sound
			reader->read(rswobj->sound.name, 80, 1, reader);
			reader->read(rswobj->sound.waveName, 80, 1, reader);
			reader->read(rswobj->sound.pos, sizeof(float), 3, reader);
			reader->read(&rswobj->sound.vol, sizeof(float), 1, reader);
			reader->read(&rswobj->sound.width, sizeof(int), 1, reader);
			reader->read(&rswobj->sound.height, sizeof(int), 1, reader);
			reader->read(&rswobj->sound.range, sizeof(float), 1, reader);
			if (ret->vermajor >= 2) { // v > 2.0
				reader->read(&rswobj->sound.cycle, sizeof(float), 1, reader);
			}
			else {
				rswobj->sound.cycle = 4.0f;
			}

			// Sanity
			rswobj->sound.name[39] = 0;
			rswobj->sound.waveName[39] = 0;
			break;
		case RORSW_OBJECT_EFFECT:	// Effect
			reader->read(rswobj->effect.name, 80, 1, reader);
			reader->read(rswobj->effect.pos, sizeof(float), 3, reader);
			reader->read(&rswobj->effect.type, sizeof(int), 1, reader);
			reader->read(&rswobj->effect.emitSpeed, sizeof(float), 1, reader);
			reader->read(rswobj->effect.param, sizeof(float), 4, reader);

			// Sanity
			rswobj->effect.name[39] = 0;
			break;
		default:
			_xlog("I don't understand rsw object type %d", rswobj->type);
			break;
		}
	}

	// == == == QUADTREE == == ==
	if (ret->vermajor >= 3 || (ret->vermajor == 2 && ret->verminor >= 1)) { // v > 2.1
		unsigned int i = 0;

		ret->quadtree = (struct RORswQuadTreeNode*)_xalloc(sizeof(struct RORswQuadTreeNode) * 1365);
		
		RswReadQuadtree(ret->quadtree, reader, 0, &i);
	}

	return(ret);
}

void RswReadQuadtree(struct RORswQuadTreeNode* quadtree, struct _reader *reader, unsigned int level, unsigned int *i) {
	struct RORswQuadTreeNode* node;
	int k;

	node = &quadtree[*i];
	reader->read(node->max, sizeof(float), 3, reader);
	reader->read(node->min, sizeof(float), 3, reader);
	reader->read(node->halfSize, sizeof(float), 3, reader);
	reader->read(node->center, sizeof(float), 3, reader);

	*i = *i + 1;

	if (level < 5) {
		for (k = 0; k < 4; k++) {
			node->child[k] = *i;
			RswReadQuadtree(quadtree, reader, level + 1, i);
		}
	}
	else {
		node->child[0] = 0;
		node->child[1] = 0;
		node->child[2] = 0;
		node->child[3] = 0;
	}

}

struct RORsw *rsw_loadFromData(const unsigned char *data, unsigned long length) {
	struct RORsw *ret;
	struct _reader *reader;

	reader = memreader_init(data, length);
	ret = rsw_load(reader);
	reader->destroy(reader);

	return(ret);
}


struct RORsw *rsw_loadFromFile(const char *fn) {
	struct RORsw *ret;
	struct _reader *reader;

	reader = filereader_init(fn);
	ret = rsw_load(reader);
	reader->destroy(reader);

	return(ret);
}


struct RORsw *rsw_loadFromGrf(struct ROGrfFile *file) {
	struct RORsw *ret = NULL;
	if (file->data == NULL) {
		grf_getdata(file);
		if (file->data != NULL) {
			ret = rsw_loadFromData(file->data, file->uncompressedLength);
		}
		_xfree(file->data);
		file->data = NULL;
	}
	else {
		ret = rsw_loadFromData(file->data, file->uncompressedLength);
	}

	return(ret);
}

void rsw_unload(struct RORsw *rsw) {
	if (rsw == NULL)
		return;

	_xfree(rsw->objects);
	if (NULL != rsw->quadtree)
		_xfree(rsw->quadtree);

	_xfree(rsw);
}
