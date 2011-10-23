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


struct ROAct *act_load(struct _reader *reader) {
	struct ROAct *ret;
	unsigned int actionId, motionId, sprclipId, attachpointId, eventId;
	char magic[2];

	ret = (struct ROAct*)_xalloc(sizeof(struct ROAct));
	memset(ret, 0, sizeof(struct ROAct));

	reader->read(&magic, 2, 1, reader);
	if (strncmp("AC", magic, 2) != 0) {
		_xlog("Invalid ACT header: '%c%c'\n", magic[0], magic[1]);
		act_unload(ret);
		return(NULL);
	}

	reader->read(&ret->version, 2, 1, reader);
	//_xlog("ACT Version: %u.%u\n", (ret->version >> 8) & 0xFF, ret->version & 0xFF);
	switch (ret->version) {
		default:
			if (ret->version >= 0x100 && ret->version <= 0x1FF) {
				_xlog("WARNING assuming this is the base version\n");
				break;// supported? not sure what's the base version... probably 0x100 or 0x101
			}
			_xlog("Unsupported ACT version\n");
			act_unload(ret);
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
	reader->read(&ret->actioncount, 2, 1, reader);
	reader->read(&ret->reserved, 10, 1, reader);
	if (ret->actioncount > 0) {
		ret->actions = (struct ROActAction*)_xalloc(sizeof(struct ROActAction) * ret->actioncount);
		memset(ret->actions, 0, sizeof(struct ROActAction) * ret->actioncount);
		for (actionId = 0; actionId < ret->actioncount; actionId++) {
			struct ROActAction *action = &ret->actions[actionId];
			// read motions
			reader->read(&action->motioncount, 4, 1, reader);
			if (action->motioncount > 0) {
				action->motions = (struct ROActMotion*)_xalloc(sizeof(struct ROActMotion) * action->motioncount);
				memset(action->motions, 0, sizeof(struct ROActMotion) * action->motioncount);
				for (motionId = 0; motionId < action->motioncount; motionId++) {
					struct ROActMotion *motion = &action->motions[motionId];
					reader->read(&motion->range1, 4, 4, reader);
					reader->read(&motion->range2, 4, 4, reader);
					// read sprclips
					reader->read(&motion->sprclipcount, 4, 1, reader);
					if (motion->sprclipcount > 0) {
						motion->sprclips = (struct ROActSprClip*)_xalloc(sizeof(struct ROActSprClip) * motion->sprclipcount);
						memset(motion->sprclips, 0, sizeof(struct ROActSprClip) * motion->sprclipcount);
						for (sprclipId = 0; sprclipId < motion->sprclipcount; sprclipId++) {
							struct ROActSprClip *sprclip = &motion->sprclips[sprclipId];
							reader->read(&sprclip->x, 4, 1, reader);
							reader->read(&sprclip->y, 4, 1, reader);
							reader->read(&sprclip->sprNo, 4, 1, reader);
							reader->read(&sprclip->mirrorOn, 4, 1, reader);
							sprclip->color = 0xFFFFFFFF;
							sprclip->xZoom = sprclip->yZoom = 1.0f;
							sprclip->angle = 0;
							sprclip->sprType = 0;
							sprclip->width = sprclip->height = 0;
							if (ret->version >= 0x200) {
								reader->read(&sprclip->color, 4, 1, reader);
								if (ret->version >= 0x204) {
									reader->read(&sprclip->xZoom, 4, 1, reader);
									reader->read(&sprclip->yZoom, 4, 1, reader);
								}
								else {
									float zoom;
									reader->read(&zoom, 4, 1, reader);
									sprclip->xZoom = sprclip->yZoom = zoom;
								}
								reader->read(&sprclip->angle, 4, 1, reader);
								reader->read(&sprclip->sprType, 4, 1, reader);
								if (ret->version >= 0x205) {
									reader->read(&sprclip->width, 4, 1, reader);
									reader->read(&sprclip->height, 4, 1, reader);
								}
							}
						}
					}
					// read eventId
					motion->eventId = -1;
					if (ret->version >= 0x200) {
						reader->read(&motion->eventId, 4, 1, reader);
						if (ret->version == 0x200)
							motion->eventId = -1;// no array of events in this version
					}
					// read attach points
					if (ret->version >= 0x203) {
						reader->read(&motion->attachpointcount, 4, 1, reader);
						if (motion->attachpointcount > 0) {
							motion->attachpoints = (struct ROActAttachPoint*)_xalloc(sizeof(struct ROActAttachPoint) * motion->attachpointcount);
							memset(motion->attachpoints, 0, sizeof(struct ROActAttachPoint) * motion->attachpointcount);
							for (attachpointId = 0; attachpointId < motion->attachpointcount; attachpointId++) {
								struct ROActAttachPoint *attachpoint = &motion->attachpoints[attachpointId];
								int ignored;
								reader->read(&ignored, 4, 1, reader);
								reader->read(attachpoint, sizeof(struct ROActAttachPoint), 1, reader);
							}
						}
					}
				}
			}
		}
	}
	// read events
	if (ret->version >= 0x201) {
		reader->read(&ret->eventcount, 4, 1, reader);
		if (ret->eventcount > 0) {
			ret->events = (struct ROActEvent*)_xalloc(sizeof(struct ROActEvent) * ret->eventcount);
			memset(ret->events, 0, sizeof(struct ROActEvent) * ret->eventcount);
			for (eventId = 0; eventId < ret->eventcount; eventId++) {
				struct ROActEvent *evt = &ret->events[eventId];
				reader->read(evt, sizeof(struct ROActEvent), 1, reader);
				evt->name[39] = 0;
			}
		}
	}
	// read delays
	if (ret->version >= 0x202) {
		if (ret->actioncount > 0) {
			ret->delays = (float*)_xalloc(sizeof(float) * ret->actioncount);
			reader->read(ret->delays, 4, ret->actioncount, reader);
		}
	}

	if (reader->error) {
		// data was missing
		_xlog("ACT is incomplete or invalid\n");
		act_unload(ret);
		return(NULL);
	}

	return(ret);
}


struct ROAct *act_loadFromData(const unsigned char *data, unsigned int length) {
	struct ROAct *ret;
	struct _reader *reader;

	reader = memreader_init(data, length);
	ret = act_load(reader);
	reader->destroy(reader);

	return(ret);
}


struct ROAct *act_loadFromFile(const char *fn) {
	struct ROAct *ret;
	struct _reader *reader;

	reader = filereader_init(fn);
	ret = act_load(reader);
	reader->destroy(reader);

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
