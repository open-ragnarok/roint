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


const char ACT_MAGIC[] = {'A','C'};


unsigned short act_inspect(const struct ROAct *act) {
	unsigned int actionId, motionId, sprclipId, eventId;
	unsigned short minver = 0x100;

	if (act == NULL) {
		_xlog("act.inspect : invalid argument (act=%p)\n", act);
		return(0);
	}
	
	if (act->actioncount > 0 && act->actions == NULL) {
		_xlog("act.inspect : expected non-NULL actions\n");
		return(0);
	}
	if (act->actioncount == 0 && act->actions != NULL) {
		_xlog("act.inspect : expected NULL actions\n");
		return(0);
	}
	for (actionId = 0; actionId < act->actioncount; actionId++) {
		const struct ROActAction *action = &act->actions[actionId];
		if (action->motioncount > 0 && action->motions == NULL) {
			_xlog("act.inspect : [%u] expected non-NULL motions\n", actionId);
			return(0);
		}
		if (action->motioncount == 0 && action->motions != NULL) {
			_xlog("act.inspect : [%u] expected NULL motions\n", actionId);
			return(0);
		}
		for (motionId = 0; motionId < action->motioncount; motionId++) {
			const struct ROActMotion *motion = &action->motions[motionId];
			if (minver < 0x200 && (motion->eventId != -1))
				minver = 0x200;
			if (motion->sprclipcount > 0 && motion->sprclips == NULL) {
				_xlog("act.inspect : [%u][%u] expected non-NULL sprclips\n", actionId, motionId);
				return(0);
			}
			if (motion->sprclipcount == 0 && motion->sprclips != NULL) {
				_xlog("act.inspect : [%u][%u] expected NULL sprclips\n", actionId, motionId);
				return(0);
			}
			if (motion->attachpointcount > 0 && motion->attachpoints == NULL) {
				_xlog("act.inspect : [%u][%u] expected non-NULL attachpoints\n", actionId, motionId);
				return(0);
			}
			if (motion->attachpointcount == 0 && motion->attachpoints != NULL) {
				_xlog("act.inspect : [%u][%u] expected NULL attachpoints\n", actionId, motionId);
				return(0);
			}
			for (sprclipId = 0; sprclipId < motion->sprclipcount; sprclipId++) {
				const struct ROActSprClip *sprclip = &motion->sprclips[motionId];
				if (minver < 0x205 && (sprclip->width != 0 || sprclip->height != 0))
					minver = 0x205;
				if (minver < 0x204 && sprclip->xZoom != sprclip->yZoom)
					minver = 0x204;
				if (minver < 0x200 && (sprclip->sprType != 0 || sprclip->angle != 0 || sprclip->color != 0xFFFFFFFF || sprclip->xZoom != 1.0f || sprclip->yZoom != 1.0f))
					minver = 0x200;
			}
			if (minver < 0x203 && motion->attachpointcount > 0)
				minver = 0x203;
		}
	}
	if (act->eventcount > 0 && act->events == NULL) {
		_xlog("act.inspect : expected non-NULL events\n");
		return(0);
	}
	if (act->eventcount == 0 && act->events != NULL) {
		_xlog("act.inspect : expected NULL events\n");
		return(0);
	}
	if (minver >= 0x202 && act->delays == NULL) {
		_xlog("act.inspect : expected non-NULL delays for version 0x%X (v%u.%u)\n", minver, (minver >> 8) & 0xFF, minver & 0xFF);
		return(0);
	}
	for (eventId = 0; eventId < act->eventcount; eventId++) {
		const struct ROActEvent *evt = &act->events[eventId];
		if (memchr(evt->name, 0, sizeof(evt->name)) == NULL) {
			_xlog("act.inspect : [%u] event name is not NUL-terminated\n", eventId);
			return(0);
		}
	}
	if (minver < 0x202 && act->delays != NULL)
		minver = 0x202;
	if (minver < 0x201 && act->eventcount > 0)
		minver = 0x201;
	return(minver);
}


struct ROAct *act_load(struct _reader *reader) {
	struct ROAct *ret;
	unsigned int actionId, motionId, sprclipId, attachpointId, eventId;
	char magic[2];

	if (reader == NULL || reader->error) {
		_xlog("act.load : invalid argument (reader=%p reader.error=%d)\n", reader, reader->error);
		return(NULL);
	}

	ret = (struct ROAct*)_xalloc(sizeof(struct ROAct));
	memset(ret, 0, sizeof(struct ROAct));

	reader->read(&magic, 2, 1, reader);
	if (memcmp(ACT_MAGIC, magic, 2) != 0) {
		_xlog("act.load : invalid header x%02X%02X (\"%-2s\")\n", magic[0], magic[1], magic);
		act_unload(ret);
		return(NULL);
	}

	reader->read(&ret->version, 2, 1, reader);
	//_xlog("ACT Version: %u.%u\n", (ret->version >> 8) & 0xFF, ret->version & 0xFF);
	switch (ret->version) {
		default:
			if (ret->version >= 0x100 && ret->version <= 0x1FF) {
				_xlog("act.load : WARNING assuming base version 0x%X (v%u.%u)\n", ret->version, (ret->version >> 8) & 0xFF, ret->version & 0xFF);
				break;// supported? not sure what's the base version... probably 0x100 or 0x101
			}
			_xlog("act.load : unknown version 0x%X (v%u.%u)\n", ret->version, (ret->version >> 8) & 0xFF, ret->version & 0xFF);
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
		_xlog("act.load : read error\n");
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


int act_save(const struct ROAct *act, struct _writer *writer) {
	unsigned int actionId, motionId, sprclipId, attachpointId, eventId;
	unsigned short minimumver;

	if (act == NULL || writer == NULL || writer->error) {
		_xlog("act.save : invalid argument (act=%o writer=%p writer.error=%d)\n", act, writer, writer->error);
		return(1);
	}
	
	minimumver = act_inspect(act);
	if (minimumver == 0) {
		_xlog("act.save : invalid\n");
		return(1);
	}
	switch (act->version) {
		default:
			if (act->version >= 0x100 && act->version <= 0x1FF) {
				_xlog("act.save : WARNING assuming base version 0x%X (v%u.%u)\n", act->version, (act->version >> 8) & 0xFF, act->version & 0xFF);
				break;// supported? not sure what's the base version... probably 0x100 or 0x101
			}
			_xlog("act.save : unknown version 0x%X (v%u.%u)\n", act->version, (act->version >> 8) & 0xFF, act->version & 0xFF);
			return(1);
		case 0x200:
		case 0x201:
		case 0x202:
		case 0x203:
		case 0x204:
		case 0x205:
			break;// supported
	}
	if (act->version < minimumver) {
		_xlog("act.save : incompatible version (must be at least v%u.%u)\n", (minimumver >> 8) & 0xFF, minimumver & 0xFF);
		return(1);
	}

	writer->write(ACT_MAGIC, 2, 1, writer);
	writer->write(&act->version, 2, 1, writer);
	// write actions
	writer->write(&act->actioncount, 2, 1, writer);
	writer->write(&act->reserved, 10, 1, writer);
	for (actionId = 0; actionId < act->actioncount; actionId++) {
		const struct ROActAction *action = &act->actions[actionId];
		// write motions
		writer->write(&action->motioncount, 4, 1, writer);
		for (motionId = 0; motionId < action->motioncount; motionId++) {
			struct ROActMotion *motion = &action->motions[motionId];
			writer->write(&motion->range1, 4, 4, writer);
			writer->write(&motion->range2, 4, 4, writer);
			// write sprclips
			writer->write(&motion->sprclipcount, 4, 1, writer);
			for (sprclipId = 0; sprclipId < motion->sprclipcount; sprclipId++) {
				const struct ROActSprClip *sprclip = &motion->sprclips[sprclipId];
				writer->write(&sprclip->x, 4, 1, writer);
				writer->write(&sprclip->y, 4, 1, writer);
				writer->write(&sprclip->sprNo, 4, 1, writer);
				writer->write(&sprclip->mirrorOn, 4, 1, writer);
				if (act->version >= 0x200) {
					writer->write(&sprclip->color, 4, 1, writer);
					if (act->version >= 0x204) {
						writer->write(&sprclip->xZoom, 4, 1, writer);
						writer->write(&sprclip->yZoom, 4, 1, writer);
					}
					else
						writer->write(&sprclip->xZoom, 4, 1, writer);
					writer->write(&sprclip->angle, 4, 1, writer);
					writer->write(&sprclip->sprType, 4, 1, writer);
					if (act->version >= 0x205) {
						writer->write(&sprclip->width, 4, 1, writer);
						writer->write(&sprclip->height, 4, 1, writer);
					}
				}
			}
			// write eventId
			if (act->version >= 0x200)
				writer->write(&motion->eventId, 4, 1, writer);
			// write attach points
			if (act->version >= 0x203) {
				writer->write(&motion->attachpointcount, 4, 1, writer);
				for (attachpointId = 0; attachpointId < motion->attachpointcount; attachpointId++) {
					const struct ROActAttachPoint *attachpoint = &motion->attachpoints[attachpointId];
					int ignored = 0;
					writer->write(&ignored, 4, 1, writer);
					writer->write(attachpoint, sizeof(struct ROActAttachPoint), 1, writer);
				}
			}
		}
	}
	// write events
	if (act->version >= 0x201) {
		writer->write(&act->eventcount, 4, 1, writer);
		for (eventId = 0; eventId < act->eventcount; eventId++) {
			struct ROActEvent *evt = &act->events[eventId];
			writer->write(evt, sizeof(struct ROActEvent), 1, writer);
		}
	}
	// write delays
	if (act->version >= 0x202) {
		if (act->actioncount > 0)
			writer->write(act->delays, 4, act->actioncount, writer);
	}

	if (writer->error) {
		_xlog("act.save : write error\n");
		return(1);
	}

	return(0);
}


int act_saveToData(const struct ROAct *act, unsigned char **data_out, unsigned long *size_out) {
	int ret;
	struct _writer *writer;

	writer = memwriter_init(data_out, size_out);
	ret = act_save(act, writer);
	writer->destroy(writer);

	return(ret);
}


int act_saveToFile(const struct ROAct *act, const char *fn) {
	int ret;
	struct _writer *writer;

	writer = filewriter_init(fn);
	ret = act_save(act, writer);
	writer->destroy(writer);

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
