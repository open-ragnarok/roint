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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <roint.h>


//#define SKIP_PRINT_TEXTURE
//#define SKIP_PRINT_KEYFRAME


int str_equal(struct ROStr *str, struct ROStr *str2) {
	unsigned int layerId, textureId;
	if (str2->version != str->version ||
		str2->framecount != str->framecount ||
		str2->fps != str->fps ||
		str2->layercount != str->layercount ||
		memcmp(str2->reserved, str->reserved, sizeof(str->reserved)) != 0)
		return(0);
	for (layerId = 0; layerId < str->layercount; layerId++) {
		struct ROStrLayer *layer = &str->layers[layerId];
		struct ROStrLayer *layer2 = &str2->layers[layerId];
		if (layer2->texturecount != layer->texturecount ||
			layer2->keyframecount != layer->keyframecount)
			return(0);
		for (textureId = 0; textureId < layer->texturecount; textureId++) {
			struct ROStrTexture *texture = &layer->textures[textureId];
			struct ROStrTexture *texture2 = &layer2->textures[textureId];
			if (strcmp(texture2->name, texture->name) != 0)
				return(0);
		}
		if (layer->keyframecount > 0) {
			if (memcmp(layer2->keyframes, layer->keyframes, sizeof(struct ROStrKeyFrame) * layer->keyframecount) != 0)
				return(0);
		}
	}
	return(1);
}


int main(int argc, char **argv)
{
	const char *fn;
	struct ROStr *str;
	unsigned int layerId, textureId, keyframeId;
	int ret;

	if (argc != 2) {
		const char *exe = argv[0];
		printf("Usage:\n  %s file.str\n", exe);
		return(EXIT_FAILURE);
	}

	fn = argv[1];

	str = str_loadFromFile(fn);
	if (str == NULL) {
		printf("error : failed to load file '%s'\n", fn);
		return(EXIT_FAILURE);
	}
	ret = EXIT_SUCCESS;
	printf("Inspect: %u\n", str_inspect(str));
	printf("Version: v%u\n", str->version);
	if (str->version == 148)
		;// supported
	else {
		printf("error : unknown version\n");
		ret = EXIT_FAILURE;
	}
	printf("Frame count: %u\n", str->framecount);
	printf("Frames Per Second: %u\n", str->fps);
	printf("Reserved: x%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X (\"%-16s\")\n",
		str->reserved[0], str->reserved[1], str->reserved[2], str->reserved[3], str->reserved[4], str->reserved[5], str->reserved[6], str->reserved[7],
		str->reserved[8], str->reserved[9], str->reserved[10], str->reserved[11], str->reserved[12], str->reserved[13], str->reserved[14], str->reserved[15],
		str->reserved);
	printf("Layers: %u (%p)\n", str->layercount, str->layers);
	if (str->layercount > 0 && str->layers == NULL) {
		printf("error : should have non-NULL layers\n");
		ret = EXIT_FAILURE;
	}
	if (str->layercount == 0 && str->layers != NULL) {
		printf("error : should have NULL layers\n");
		ret = EXIT_FAILURE;
	}
	for (layerId = 0; layerId < str->layercount; layerId++) {
		struct ROStrLayer *layer = &str->layers[layerId];
		printf("[%u] Textures: %u (%p)\n", layerId, layer->texturecount, layer->textures);
		if (layer->texturecount > 0 && layer->textures == NULL) {
			printf("error : should have non-NULL textures\n");
			ret = EXIT_FAILURE;
		}
		if (layer->texturecount == 0 && layer->textures != NULL) {
			printf("error : should have NULL textures\n");
			ret = EXIT_FAILURE;
		}
		for (textureId = 0; textureId < layer->texturecount; textureId++) {
#ifndef SKIP_PRINT_TEXTURE
			printf("[%u][%u] \"%s\"\n", layerId, textureId, layer->textures[textureId].name);
#endif
		}
		printf("[%u] Key Frames: %u (%p)\n", layerId, layer->keyframecount, layer->keyframes);
		if (layer->keyframecount > 0 && layer->keyframes == NULL) {
			printf("error : should have non-NULL keyframes\n");
			ret = EXIT_FAILURE;
		}
		if (layer->keyframecount == 0 && layer->keyframes != NULL) {
			printf("error : should have NULL keyframes\n");
			ret = EXIT_FAILURE;
		}
		for (keyframeId = 0; keyframeId < layer->keyframecount; keyframeId++) {
			struct ROStrKeyFrame *keyframe = &layer->keyframes[keyframeId];
#ifndef SKIP_PRINT_KEYFRAME
			printf("[%u][%u] framenum=%u type=%u\n", layerId, keyframeId, keyframe->framenum, keyframe->type);
			printf("[%u][%u] x=%f y=%f\n", layerId, keyframeId, keyframe->x, keyframe->y);
			printf("[%u][%u] u=%f v=%f us=%f vs=%f\n", layerId, keyframeId, keyframe->u, keyframe->v, keyframe->us, keyframe->vs);
			printf("[%u][%u] u2=%f v2=%f us2=%f vs2=%f\n", layerId, keyframeId, keyframe->u2, keyframe->v2, keyframe->us2, keyframe->vs2);
			printf("[%u][%u] ax={%f,%f,%f,%f}\n", layerId, keyframeId, keyframe->ax[0], keyframe->ax[1], keyframe->ax[2], keyframe->ax[3]);
			printf("[%u][%u] ay={%f,%f,%f,%f}\n", layerId, keyframeId, keyframe->ay[0], keyframe->ay[1], keyframe->ay[2], keyframe->ay[3]);
			printf("[%u][%u] aniframe=%f anitype=%u anidelta=%f rz=%f\n", layerId, keyframeId, keyframe->aniframe, keyframe->anitype, keyframe->anidelta, keyframe->rz);
			printf("[%u][%u] crR=%f crG=%f crB=%f crA=%f\n", layerId, keyframeId, keyframe->crR, keyframe->crG, keyframe->crB, keyframe->crA);
			printf("[%u][%u] srcalpha=%u destalpha=%u mtpreset=%u\n", layerId, keyframeId, keyframe->srcalpha, keyframe->destalpha, keyframe->mtpreset);
#endif
		}
	}

	{// test save to data
		unsigned char *data = NULL;
		unsigned long length = 0;
		struct ROStr *str2;
		printf("Save: %d\n", str_saveToData(str, &data, &length));
		printf("Data: %p %u\n", data, length);
		str2 = str_loadFromData(data,length);
		if (data == NULL) {
			printf("error : saving produced NULL data\n");
			ret = EXIT_FAILURE;
		}
		else if (str2 == NULL) {
			printf("error : saving produced invalid data\n");
			ret = EXIT_FAILURE;
		}
		else {
			printf("Inspect: %u\n", str_inspect(str2));
			if (!str_equal(str, str2)) {
				printf("error : saving produced different data\n");
				ret = EXIT_FAILURE;
			}
		}
		if (data != NULL)
			get_roint_free_func()(data);
		str_unload(str2);
	}

	str_unload(str);
	if (ret == EXIT_SUCCESS)
		printf("OK\n");
	return(ret);
}
