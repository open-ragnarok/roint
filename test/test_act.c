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


int act_equal(struct ROAct *act, struct ROAct *act2) {
	unsigned int actionId, motionId, sprclipId, attachpointId, eventId;

	if (act2->version != act->version ||
		act2->actioncount != act->actioncount ||
		act2->eventcount != act->eventcount ||
		memcmp(act2->reserved, act->reserved, sizeof(act->reserved)) != 0)
		return(0);
	for (actionId = 0; actionId < act->actioncount; actionId++) {
		struct ROActAction *action = &act->actions[actionId];
		struct ROActAction *action2 = &act->actions[actionId];
		if (action2->motioncount != action->motioncount)
			return(0);
		for (motionId = 0; motionId < action->motioncount; motionId++) {
			struct ROActMotion *motion = &action->motions[motionId];
			struct ROActMotion *motion2 = &action2->motions[motionId];
			if (motion2->sprclipcount != motion->sprclipcount ||
				motion2->attachpointcount != motion->attachpointcount ||
				motion2->eventId != motion->eventId ||
				memcmp(motion2->range1, motion->range1, sizeof(motion->range1)) != 0 ||
				memcmp(motion2->range2, motion->range2, sizeof(motion->range2)) != 0)
				return(0);
			for (sprclipId = 0; sprclipId < motion->sprclipcount; sprclipId++) {
				struct ROActSprClip *sprclip = &motion->sprclips[sprclipId];
				struct ROActSprClip *sprclip2 = &motion2->sprclips[sprclipId];
				if (sprclip2->x != sprclip->x ||
					sprclip2->y != sprclip->y ||
					sprclip2->sprNo != sprclip->sprNo ||
					sprclip2->mirrorOn != sprclip->mirrorOn ||
					sprclip2->color != sprclip->color ||
					sprclip2->xZoom != sprclip->xZoom ||
					sprclip2->yZoom != sprclip->yZoom ||
					sprclip2->angle != sprclip->angle ||
					sprclip2->sprType != sprclip->sprType ||
					sprclip2->width != sprclip->width ||
					sprclip2->height != sprclip->height)
					return(0);
			}
			for (attachpointId = 0; attachpointId < motion->attachpointcount; attachpointId++) {
				struct ROActAttachPoint *attachpoint = &motion->attachpoints[attachpointId];
				struct ROActAttachPoint *attachpoint2 = &motion2->attachpoints[attachpointId];
				if (attachpoint2->x != attachpoint->x ||
					attachpoint2->y != attachpoint->y ||
					attachpoint2->attr != attachpoint->attr)
					return(0);
			}
		}
	}
	for (eventId = 0; eventId < act->eventcount; eventId++) {
		struct ROActEvent *evt = &act->events[eventId];
		struct ROActEvent *evt2 = &act->events[eventId];
		if (memcmp(evt2->name, evt->name, sizeof(evt->name)) != 0)
			return(0);
	}
	if (act->version >= 0x202) {
		if (memcmp(act2->delays, act->delays, 4 * act->actioncount) != 0)
			return(0);
	}
	return(1);
}


int main(int argc, char **argv)
{
	const char *fn;
	struct ROAct *act;
	unsigned int actionId, motionId, sprclipId, attachpointId, eventId;
	int ret;

	if (argc != 2) {
		const char *exe = argv[0];
		printf("Usage:\n  %s file.act\n", exe);
		return(EXIT_FAILURE);
	}

	fn = argv[1];
	
	ret = EXIT_SUCCESS;
	act = act_loadFromFile(fn);
	if (act == NULL) {
		printf("error : failed to load file '%s'\n", fn);
		return(EXIT_FAILURE);
	}
	printf("Inspect: 0x%x\n", act_inspect(act));
	printf("Version: 0x%x (v%u.%u)\n", act->version, (act->version >> 8) & 0xFF, act->version & 0xFF);
	switch (act->version) {
		default:
			if (act->version >= 0x100 && act->version <= 0x1FF)
				break;// base version v1.?
			printf("error : unknown version\n");
			ret = EXIT_FAILURE;
			break;
		case 0x200:
		case 0x201:
		case 0x202:
		case 0x203:
		case 0x204:
		case 0x205:
			break;// supported
	}

	printf("Reserved: 0x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x (\"%-10s\")\n",
		act->reserved[0], act->reserved[1], act->reserved[2], act->reserved[3], act->reserved[4],
		act->reserved[5], act->reserved[6], act->reserved[7], act->reserved[8], act->reserved[9],
		act->reserved);

	printf("Actions: %u (%p)\n", act->actioncount, act->actions);
	for (actionId = 0; actionId < act->actioncount; actionId++) {
		struct ROActAction *action = &act->actions[actionId];
		printf("[%u] Motions: %u (%p)\n", actionId, action->motioncount, action->motions);
		for (motionId = 0; motionId < action->motioncount; motionId++) {
			struct ROActMotion *motion = &action->motions[motionId];
			printf("[%u][%u] range1={%d,%d,%d,%d} range2={%d,%d,%d,%d} eventId=%d\n", actionId, motionId,
				motion->range1[0], motion->range1[1], motion->range1[2], motion->range1[3],
				motion->range2[0], motion->range2[1], motion->range2[2], motion->range2[3],
				motion->eventId);

			printf("[%u][%u] Spr Clips: %u (%p)\n", actionId, motionId, motion->sprclipcount, motion->sprclips);
			if (motion->sprclipcount > 1000) {
				printf("error : should not have more than 1000 sprclips\n");
				ret = EXIT_FAILURE;
			}
			for (sprclipId = 0; sprclipId < motion->sprclipcount; sprclipId++) {
				struct ROActSprClip *sprclip = &motion->sprclips[sprclipId];
				printf("[%u][%u][%u] x=%d y=%d sprNo=%d mirrorOn=%u color=0x%08x xZoom=%f yZoom=%f angle=%d sprType=%d width=%d height=%d\n", actionId, motionId, sprclipId,
					sprclip->x, sprclip->y, sprclip->sprNo, sprclip->mirrorOn, sprclip->color,
					sprclip->xZoom, sprclip->yZoom, sprclip->angle, sprclip->sprType, sprclip->width, sprclip->height);
				if (act->version < 0x205 && (sprclip->width != 0 || sprclip->height != 0)) {
					printf("error : should not have width or height (< v2.5)\n");
					ret = EXIT_FAILURE;
				}
			}
			if (motion->sprclipcount == 0 && motion->sprclips != NULL) {
				printf("error : should have NULL sprclips\n");
				ret = EXIT_FAILURE;
			}

			printf("[%u][%u] Attach Points: %u (%p)\n", actionId, motionId, motion->attachpointcount, motion->attachpoints);
			if (act->version < 0x203 && motion->attachpoints != NULL) {
				printf("error : should have NULL attachpoints (< v2.3)\n");
				ret = EXIT_FAILURE;
			}
			for (attachpointId = 0; attachpointId < motion->attachpointcount; attachpointId++) {
				struct ROActAttachPoint *attachpoint = &motion->attachpoints[attachpointId];
				printf("[%u][%u][%u] x=%d y=%d attr=%d\n", actionId, motionId, attachpointId,
					attachpoint->x, attachpoint->y, attachpoint->attr);
			}
			if (motion->attachpointcount == 0 && motion->attachpoints != NULL) {
				printf("error : should have NULL attachpoints\n");
				ret = EXIT_FAILURE;
			}
		}
		if (action->motioncount == 0 && action->motions != NULL) {
			printf("error : should have NULL motions\n");
			ret = EXIT_FAILURE;
		}
	}
	if (act->actioncount == 0 && act->actions != NULL) {
		printf("error : should have NULL actions\n");
		ret = EXIT_FAILURE;
	}

	printf("Events: %u (%p)\n", act->eventcount, act->events);
	if (act->version < 0x201 && act->events != NULL) {
		printf("error : should have NULL events (< v2.1)\n");
		ret = EXIT_FAILURE;
	}
	for (eventId = 0; eventId < act->eventcount; eventId++) {
		struct ROActEvent *evt = &act->events[eventId];
		printf("[%u] name=\"%s\"\n", eventId, evt->name);
	}
	if (act->eventcount == 0 && act->events != NULL) {
		printf("error : should have NULL events\n");
		ret = EXIT_FAILURE;
	}

	printf("Delays: (%p)\n", act->delays);
	if (act->version < 0x202 && act->delays != NULL) {
		printf("error : should have NULL delays (< v2.2)\n");
		ret = EXIT_FAILURE;
	}
	if (act->version >= 0x202 ) {
		if (act->actioncount > 0 && act->delays == NULL) {
			printf("error : should have non-NULL delays (v2.2+)\n");
			ret = EXIT_FAILURE;
		}
		for (actionId = 0; actionId < act->actioncount; actionId++)
			printf("[%u] delay=%f\n", actionId, act->delays[actionId]);
		if (act->actioncount == 0 && act->delays != NULL) {
			printf("error : should have NULL delays\n");
			ret = EXIT_FAILURE;
		}
	}

	{// test save to data
		unsigned char *data = NULL;
		unsigned long length = 0;
		struct ROAct *act2;
		printf("Save: %d\n", act_saveToData(act, &data, &length));
		printf("Data: %p %u\n", data, length);
		act2 = act_loadFromData(data,length);
		if (data == NULL) {
			printf("error : saving produced NULL data\n");
			ret = EXIT_FAILURE;
		}
		else if (act2 == NULL) {
			printf("error : saving produced invalid data\n");
			ret = EXIT_FAILURE;
		}
		else {
			printf("Inspect: 0x%x\n", act_inspect(act2));
			if (!act_equal(act, act2)) {
				printf("error : saving produced different data\n");
				ret = EXIT_FAILURE;
			}
		}
		if (data != NULL)
			get_roint_free_func()(data);
		act_unload(act2);
	}

	if (ret == EXIT_SUCCESS)
		printf("OK\n");
	return(ret);
}
