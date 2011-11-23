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


//#define SKIP_PRINT_MOTION


int imf_equal(struct ROImf *imf, struct ROImf *imf2) {
	unsigned int layerId, actionId;
	if (imf2->version != imf->version ||
		imf2->checksum != imf->checksum ||
		imf2->lastlayer != imf->lastlayer)
		return(0);
	if (imf->lastlayer >= 0) {
		unsigned int layercount = (unsigned int)imf->lastlayer + 1;
		for (layerId = 0; layerId < layercount; layerId++) {
			struct ROImfLayer *layer = &imf->layers[layerId];
			struct ROImfLayer *layer2 = &imf2->layers[layerId];
			if (layer2->actioncount != layer->actioncount)
				return(0);
			for (actionId = 0; actionId < layer->actioncount; actionId++) {
				struct ROImfAction *action = &layer->actions[actionId];
				struct ROImfAction *action2 = &layer2->actions[actionId];
				if (action2->motioncount != action->motioncount)
					return(0);
				if (action->motioncount > 0) {
					if (memcmp(action2->motions, action->motions, sizeof(struct ROImfMotion) * action->motioncount) != 0)
						return(0);
				}
			}
		}
	}
	return(1);
}


int main(int argc, char **argv)
{
	const char *fn;
	struct ROImf *imf;
	unsigned int layerId, actionId, motionId;
	unsigned int layercount;
	int ret;

	if (argc != 2) {
		const char *exe = argv[0];
		printf("Usage:\n  %s file.imf\n", exe);
		return(EXIT_FAILURE);
	}

	fn = argv[1];

	imf = imf_loadFromFile(fn);
	if (imf == NULL) {
		printf("error : failed to load file '%s'\n", fn);
		return(EXIT_FAILURE);
	}
	ret = EXIT_SUCCESS;
	printf("Inspect: %f\n", imf_inspect(imf));
	printf("Version: v%f\n", imf->version);
	if (imf->version == 1.01f)
		;// supported
	else {
		printf("error : unknown version\n");
		ret = EXIT_FAILURE;
	}
	printf("Checksum: %d\n", imf->checksum);
	printf("Layers: %d + 1 (%p)\n", imf->lastlayer, imf->layers);
	if (imf->lastlayer < -1) {
		printf("error : invalid lastlayer (%d)\n", imf->lastlayer);
		ret = EXIT_FAILURE;
	}
	layercount = (unsigned int)(imf->lastlayer + 1);
	if (layercount > 0 && imf->layers == NULL) {
		printf("error : should have non-NULL layers\n");
		ret = EXIT_FAILURE;
	}
	if (layercount == 0 && imf->layers != NULL) {
		printf("error : should have NULL layers\n");
		ret = EXIT_FAILURE;
	}
	for (layerId = 0; layerId < layercount; layerId++) {
		struct ROImfLayer *layer = &imf->layers[layerId];
		printf("[%u] Actions: %u (%p)\n", layerId, layer->actioncount, layer->actions);
		if (layer->actioncount > 0 && layer->actions == NULL) {
			printf("error : [%u] should have non-NULL actions\n", layerId);
			ret = EXIT_FAILURE;
		}
		if (layer->actioncount == 0 && layer->actions != NULL) {
			printf("error : [%u] should have NULL actions\n", layerId);
			ret = EXIT_FAILURE;
		}
		for (actionId = 0; actionId < layer->actioncount; actionId++) {
			struct ROImfAction *action = &layer->actions[actionId];
			printf("[%u][%u] Motions: %u (%p)\n", layerId, actionId, action->motioncount, action->motions);
			if (action->motioncount > 0 && action->motions == NULL) {
				printf("error : [%u][%u] should have non-NULL motions\n", layerId, actionId);
				ret = EXIT_FAILURE;
			}
			if (action->motioncount == 0 && action->motions != NULL) {
				printf("error : [%u][%u] should have NULL motions\n", layerId, actionId);
				ret = EXIT_FAILURE;
			}
			for (motionId = 0; motionId < action->motioncount; motionId++) {
				struct ROImfMotion *motion = &action->motions[motionId];
	#ifndef SKIP_PRINT_MOTION
				printf("[%u][%u][%u] priority=%d cx=%d cy=%d\n", layerId, actionId, motionId, motion->priority, motion->cx, motion->cy);
	#endif
			}
		}
	}

	{// test save to data
		unsigned char *data = NULL;
		unsigned long length = 0;
		struct ROImf *imf2;
		printf("Save: %d\n", imf_saveToData(imf, &data, &length));
		printf("Data: %p %u\n", data, length);
		imf2 = imf_loadFromData(data,length);
		if (data == NULL) {
			printf("error : saving produced NULL data\n");
			ret = EXIT_FAILURE;
		}
		else if (imf2 == NULL) {
			printf("error : saving produced invalid data\n");
			ret = EXIT_FAILURE;
		}
		else {
			printf("Inspect: %f\n", imf_inspect(imf2));
			if (!imf_equal(imf, imf2)) {
				printf("error : saving produced different data\n");
				ret = EXIT_FAILURE;
			}
		}
		if (data != NULL)
			get_roint_free_func()(data);
		imf_unload(imf2);
	}

	imf_unload(imf);
	if (ret == EXIT_SUCCESS)
		printf("OK\n");
	return(ret);
}
