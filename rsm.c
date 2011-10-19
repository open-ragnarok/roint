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
#include "roint.h"
#include "rsm.h"

#ifdef OPENRO_INTERNAL
#	include "internal.h"
#else
#	define _xalloc malloc
#	define _xfree free
#endif
#include "memloader.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct RORsm *rsm_load(const unsigned char *data, unsigned int length) {
	struct RORsm *ret;
	struct _memloader *loader;

	ret = (struct RORsm*)_xalloc(sizeof(struct RORsm));
	loader = loader_init(data, length);

	loader_read(ret->magic, 4, 1, loader);
	loader_read(&ret->version, 2, 1, loader);

	if (strncmp("GRSM", ret->magic, 4) != 0) {
		fprintf(stderr, "Invalid RSM header: '%c%c%c%c'\n", ret->magic[0], ret->magic[1], ret->magic[2], ret->magic[3]);
		_xfree(ret);
		loader_free(loader);
		return(NULL);
	}

	printf("RSM Version: %u.%u\n", (unsigned int)ret->v.major, (unsigned int)ret->v.minor);

	loader_read(&ret->anim_length, sizeof(int), 1, loader);
	loader_read(&ret->shade_type, sizeof(int), 1, loader);
	if (ret->v.major > 1 || (ret->v.major == 1 && ret->v.minor >= 4)) {
		// Versions 1.4 and up
		loader_read(&ret->alpha, sizeof(unsigned char), 1, loader);
	}
	else {
		ret->alpha = 0xff;
	}
	loader_read(ret->reserved, 16, 1, loader);
	loader_read(&ret->texture_count, sizeof(int), 1, loader);

	// Load Textures
	if (ret->texture_count > 0) {
		int i;
		char texname[40];
		ret->textures = (char**)_xalloc(sizeof(char*) * ret->texture_count);
		for (i = 0; i < ret->texture_count; i++) {
			loader_read(texname, 40, 1, loader);
			texname[39] = 0;
			ret->textures[i] = (char*)_xalloc(sizeof(char) * (strlen(texname) + 1));
			strcpy(ret->textures[i], texname);
		}
	}
	else {
		ret->textures = NULL;
	}

	// Nodes
	loader_read(ret->main_node, 40, 1, loader);
	loader_read(&ret->node_count, sizeof(int), 1, loader);

	if (ret->node_count > 0) {
		int i;
		struct RORsmNode *currentNode;

		ret->nodes = (struct RORsmNode*)_xalloc(sizeof(struct RORsmNode) * ret->node_count);

		for (i = 0; i < ret->node_count; i++) {
			currentNode = &ret->nodes[i];

			loader_read(currentNode->name, 40, 1, loader);
			loader_read(currentNode->parent, 40, 1, loader);
			loader_read(&currentNode->texture_count, sizeof(int), 1, loader);

			if (currentNode->texture_count > 0) {
				currentNode->textures = (int*)_xalloc(sizeof(int) * currentNode->texture_count);
				loader_read(currentNode->textures, sizeof(int), currentNode->texture_count, loader);
			}
			else {
				currentNode->textures = NULL;
			}

			loader_read(currentNode->offsetMT, sizeof(float), 9, loader);
			loader_read(&currentNode->pos_, sizeof(struct RORsmVertex), 1, loader);
			loader_read(&currentNode->pos, sizeof(struct RORsmVertex), 1, loader);
			loader_read(&currentNode->rot_angle, sizeof(float), 1, loader);
			loader_read(&currentNode->rot_axis, sizeof(struct RORsmVertex), 1, loader);
			loader_read(&currentNode->scale, sizeof(struct RORsmVertex), 1, loader);

			// Node vertexes
			loader_read(&currentNode->vertice_count, sizeof(int), 1, loader);
			if (currentNode->vertice_count > 0) {
				currentNode->vertices = (struct RORsmVertex*)_xalloc(sizeof(struct RORsmVertex) * currentNode->vertice_count);
				loader_read(currentNode->vertices, sizeof(struct RORsmVertex), currentNode->vertice_count, loader);
			}
			else {
				currentNode->vertices = NULL;
			}

			// Texture vertices
			loader_read(&currentNode->texv_count, sizeof(int), 1, loader);
			if (currentNode->texv_count > 0) {
				currentNode->texv = (struct RORsmTexture*)_xalloc(sizeof(struct RORsmTexture) * currentNode->texv_count);
				if (ret->v.major > 1 || (ret->v.major == 1 && ret->v.minor >= 2)) {
					// Versions 1.2 and up
					loader_read(currentNode->texv, sizeof(struct RORsmTexture), currentNode->texv_count, loader);
				}
				else {
					int j;
					for (j = 0; j < currentNode->texture_count; i++) {
						currentNode->texv[j].color = 0xffffffff;
						loader_read(&currentNode->texv[j].u, sizeof(float), 2, loader);
					}
				}
			}
			else {
				currentNode->texv = NULL;
			}

			// Faces
			loader_read(&currentNode->face_count, sizeof(int), 1, loader);
			if (currentNode->face_count > 0) {
				currentNode->faces = (struct RORsmFace*)_xalloc(sizeof(struct RORsmFace) * currentNode->face_count);
				if (ret->v.major > 1 || (ret->v.major == 1 && ret->v.minor >= 2)) {
					// Versions 1.2 and up
					loader_read(currentNode->faces, sizeof(struct RORsmFace), currentNode->face_count, loader);
				}
				else {
					int j;
					for (j = 0; j < currentNode->face_count; j++) {
						loader_read(&currentNode->faces[j], sizeof(struct RORsmFace) - sizeof(int), 1, loader);
						currentNode->faces[j].smoothGroup = 0;
					}
				}
			}
			else {
				currentNode->faces = NULL;
			}

			// Position keyframes
			if (ret->v.major > 1 || (ret->v.major == 2 && ret->v.minor >= 5)) {
				// Versions 1.5 and up
				loader_read(&currentNode->poskey_count, sizeof(int), 1, loader);
			}
			else {
				currentNode->poskey_count = 0;
			}

			if (currentNode->poskey_count > 0) {
				currentNode->poskeys = (struct RORsmPosKeyframe*)_xalloc(sizeof(struct RORsmPosKeyframe) * currentNode->poskey_count);
				loader_read(currentNode->poskeys, sizeof(struct RORsmPosKeyframe), currentNode->poskey_count, loader);
			}
			else {
				currentNode->poskeys = NULL;
			}
			// Rotation keyframes
			loader_read(&currentNode->rotkey_count, sizeof(int), 1, loader);
			if (currentNode->rotkey_count > 0) {
				struct RORsmRotKeyframe* x;
				int rotkeyframe_size = sizeof(struct RORsmRotKeyframe) * currentNode->rotkey_count;
				x = _xalloc(rotkeyframe_size);
				currentNode->rotkeys = x;
				loader_read(currentNode->rotkeys, sizeof(struct RORsmRotKeyframe), currentNode->rotkey_count, loader);
			}
			else {
				currentNode->rotkeys = NULL;
			}
		}
	}
	else {
		ret->nodes = NULL;
	}

	if (loader_error(loader)) {
		// data was missing
		fprintf(stderr, "RSM is incomplete or invalid\n");
		rsm_unload(ret);
		ret = NULL;
	}
	loader_free(loader);

	return(ret);
}

struct RORsm *rsm_loadFromGrf(struct ROGrfFile *file) {
	struct RORsm *ret;
	if (file->data == NULL) {
		grf_getdata(file);
		if (file->data != NULL) {
			ret = rsm_load(file->data, file->uncompressedLength);
		}
		_xfree(file->data);
		file->data = NULL;
	}
	else {
		ret = rsm_load(file->data, file->uncompressedLength);
	}

	return(ret);
}

void rsm_nodedelete(struct RORsmNode *node) {
	if (node->textures != NULL) {
		_xfree(node->textures);
		node->textures = NULL;
	}

	if (node->vertices != NULL) {
		_xfree(node->vertices);
		node->vertices = NULL;
	}

	if (node->texv != NULL) {
		_xfree(node->texv);
		node->texv = NULL;
	}

	if (node->faces != NULL) {
		_xfree(node->faces);
		node->faces = NULL;
	}

	if (node->poskeys != NULL) {
		_xfree(node->poskeys);
		node->poskeys = NULL;
	}
	
	if (node->rotkeys != NULL) {
		_xfree(node->rotkeys);
		node->rotkeys = NULL;
	}
}

void rsm_unload(struct RORsm* rsm) {
	if (rsm == NULL)
		return;

	if (rsm->textures != NULL) {
		int i;
		for (i = 0; i < rsm->texture_count; i++)
			_xfree(rsm->textures[i]);

		_xfree(rsm->textures);
	}

	if (rsm->nodes != NULL) {
		int i;
		for (i = 0; i < rsm->node_count; i++)
			rsm_nodedelete(&rsm->nodes[i]);
		
		_xfree(rsm->nodes);
	}

	_xfree(rsm);
}