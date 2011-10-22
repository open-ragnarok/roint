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
#include "memloader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct ROAct *act_loadFromData(const unsigned char *data, unsigned int length) {
	struct ROAct *ret;
	struct _memloader *loader;
	unsigned int actionId, motionId, sprclipId, attachpointId, eventId;
	char magic[2];

	ret = (struct ROAct*)_xalloc(sizeof(struct ROAct));
	memset(ret, 0, sizeof(struct ROAct));
	loader = loader_init(data, length);

	loader_read(&magic, 2, 1, loader);
	if (strncmp("AC", magic, 2) != 0) {
		_xlog("Invalid ACT header: '%c%c'\n", magic[0], magic[1]);
		act_unload(ret);
		loader_free(loader);
		return(NULL);
	}

	loader_read(&ret->version, 2, 1, loader);
	_xlog("ACT Version: %u.%u\n", (ret->version >> 8) & 0xFF, ret->version & 0xFF);
	switch (ret->version) {
		default:
			if (ret->version >= 0x100 && ret->version <= 0x1FF) {
				_xlog("WARNING assuming this is the base version\n");
				break;// supported? not sure what's the base version... probably 0x100 or 0x101
			}
			_xlog("Unsupported ACT version\n");
			act_unload(ret);
			loader_free(loader);
			return(NULL);
		case 0x200:
		case 0x201:
		case 0x202:
		case 0x203:
		case 0x204:
		case 0x205:
			break;// supported
	}

	// read actions
	loader_read(&ret->actioncount, 2, 1, loader);
	loader_read(&ret->reserved, 10, 1, loader);
	if (ret->actioncount > 0) {
		ret->actions = (struct ROActAction*)_xalloc(sizeof(struct ROActAction) * ret->actioncount);
		memset(ret->actions, 0, sizeof(struct ROActAction) * ret->actioncount);
		for (actionId = 0; actionId < ret->actioncount; actionId++) {
			struct ROActAction *action = &ret->actions[actionId];
			// read motions
			loader_read(&action->motioncount, 4, 1, loader);
			if (action->motioncount > 0) {
				action->motions = (struct ROActMotion*)_xalloc(sizeof(struct ROActMotion) * action->motioncount);
				memset(action->motions, 0, sizeof(struct ROActMotion) * action->motioncount);
				for (motionId = 0; motionId < action->motioncount; motionId++) {
					struct ROActMotion *motion = &action->motions[motionId];
					loader_read(&motion->range1, 4, 4, loader);
					loader_read(&motion->range2, 4, 4, loader);
					// read sprclips
					loader_read(&motion->sprclipcount, 4, 1, loader);
					if (motion->sprclipcount > 0) {
						motion->sprclips = (struct ROActSprClip*)_xalloc(sizeof(struct ROActSprClip) * motion->sprclipcount);
						memset(motion->sprclips, 0, sizeof(struct ROActSprClip) * motion->sprclipcount);
						for (sprclipId = 0; sprclipId < motion->sprclipcount; sprclipId++) {
							struct ROActSprClip *sprclip = &motion->sprclips[sprclipId];
							loader_read(&sprclip->x, 4, 1, loader);
							loader_read(&sprclip->y, 4, 1, loader);
							loader_read(&sprclip->sprNo, 4, 1, loader);
							loader_read(&sprclip->mirrorOn, 4, 1, loader);
							sprclip->color = 0xFFFFFFFF;
							sprclip->xZoom = sprclip->yZoom = 1.0f;
							sprclip->angle = 0;
							sprclip->sprType = 0;
							sprclip->width = sprclip->height = 0;
							if (ret->version >= 0x200) {
								loader_read(&sprclip->color, 4, 1, loader);
								if (ret->version >= 0x204) {
									loader_read(&sprclip->xZoom, 4, 1, loader);
									loader_read(&sprclip->yZoom, 4, 1, loader);
								}
								else {
									float zoom;
									loader_read(&zoom, 4, 1, loader);
									sprclip->xZoom = sprclip->yZoom = zoom;
								}
								loader_read(&sprclip->angle, 4, 1, loader);
								loader_read(&sprclip->sprType, 4, 1, loader);
								if (ret->version >= 0x205) {
									loader_read(&sprclip->width, 4, 1, loader);
									loader_read(&sprclip->height, 4, 1, loader);
								}
							}
						}
					}
					// read eventId
					motion->eventId = -1;
					if (ret->version >= 0x200) {
						loader_read(&motion->eventId, 4, 1, loader);
						if (ret->version == 0x200)
							motion->eventId = -1;// no array of events in this version
					}
					// read attach points
					if (ret->version >= 0x203) {
						loader_read(&motion->attachpointcount, 4, 1, loader);
						if (motion->attachpointcount > 0) {
							motion->attachpoints = (struct ROActAttachPoint*)_xalloc(sizeof(struct ROActAttachPoint) * motion->attachpointcount);
							memset(motion->attachpoints, 0, sizeof(struct ROActAttachPoint) * motion->attachpointcount);
							for (attachpointId = 0; attachpointId < motion->attachpointcount; attachpointId++) {
								struct ROActAttachPoint *attachpoint = &motion->attachpoints[attachpointId];
								int ignored;
								loader_read(&ignored, 4, 1, loader);
								loader_read(attachpoint, sizeof(struct ROActAttachPoint), 1, loader);
							}
						}
					}
				}
			}
		}
	}
	// read events
	if (ret->version >= 0x201) {
		loader_read(&ret->eventcount, 4, 1, loader);
		if (ret->eventcount > 0) {
			ret->events = (struct ROActEvent*)_xalloc(sizeof(struct ROActEvent) * ret->eventcount);
			memset(ret->events, 0, sizeof(struct ROActEvent) * ret->eventcount);
			for (eventId = 0; eventId < ret->eventcount; eventId++) {
				struct ROActEvent *evt = &ret->events[eventId];
				loader_read(evt, sizeof(struct ROActEvent), 1, loader);
				evt->name[39] = 0;
			}
		}
	}
	// read delays
	if (ret->version >= 0x202) {
		if (ret->actioncount > 0) {
			ret->delays = (float*)_xalloc(sizeof(float) * ret->actioncount);
			loader_read(ret->delays, 4, ret->actioncount, loader);
		}
	}

	if (loader_error(loader)) {
		// data was missing
		_xlog("SPR is incomplete or invalid\n");
		act_unload(ret);
		loader_free(loader);
		return(NULL);
	}
	loader_free(loader);

	return(ret);
}


struct ROAct *act_loadFromGrf(struct ROGrfFile *file) {
	struct ROAct *ret = NULL;
	if (file->data == NULL) {
		grf_getdata(file);
		if (file->data != NULL) {
			ret = act_loadFromData(file->data, file->uncompressedLength);
		}
		_xfree(file->data);
		file->data = NULL;
	}
	else {
		ret = act_loadFromData(file->data, file->uncompressedLength);
	}

	return(ret);
}


void act_unload(struct ROAct* act) {
	unsigned int actionId, motionId;

	if (act == NULL)
		return;

	if (act->actions != NULL) {
		for (actionId = 0; actionId < act->actioncount; actionId++) {
			struct ROActAction *action = &act->actions[actionId];
			if (action->motions != NULL) {
				for (motionId = 0; motionId < action->motioncount; motionId++) {
					struct ROActMotion *motion = &action->motions[motionId];
					if (motion->sprclips != NULL)
						_xfree(motion->sprclips);

					if (motion->attachpoints != NULL)
						_xfree(motion->attachpoints);
				}
				_xfree(action->motions);
			}
		}
		_xfree(act->actions);
	}

	if (act->events != NULL)
		_xfree(act->events);

	if (act->delays != NULL)
		_xfree(act->delays);

	_xfree(act);
}
