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
#ifndef __ROINT_ACT_H
#define __ROINT_ACT_H

#ifndef WITHOUT_ROINT_CONFIG
#	include "roint/config.h"
#endif
struct ROGrfFile; // forward declaration

#ifdef __cplusplus
extern "C" {
#endif 


#pragma pack(push,1)
/// Sprite Clip.
struct ROActSprClip {
	int x; //< offset in frame (center is 0)
	int y; //< offset in frame (center is 0)
	int sprNo; //< number of the image in the spr file (-1 for none)
	unsigned int mirrorOn; //< mirror image along the vertical axis if non-zero
	unsigned int color; //< (uchar r,g,b,a;)
	float xZoom; //< scale of X axis
	float yZoom; //< scale of Y axis
	int angle; //< angle/rotation (degrees)
	int sprType; //< 0=palette image,1=rgba image
	int width; //< (v2.5+)
	int height; //< (v2.5+)
};

struct ROActAttachPoint {
	int x;
	int y;
	int attr;
};

/// Motion, represents a frame in an action.
/// Holds a collection of sprite clips and a collection of attach points.
struct ROActMotion {
	int range1[4]; //< left,top,right,bottom
	int range2[4]; //< left,top,right,bottom
	int eventId; //< -1 for none
	unsigned int sprclipcount; //< (max 1000)
	struct ROActSprClip *sprclips;
	unsigned int attachpointcount;
	struct ROActAttachPoint *attachpoints; //< (v2.3+)
};

/// Action. Holds a collection of motions.
struct ROActAction {
	unsigned int motioncount;
	struct ROActMotion *motions;
};

 struct ROActEvent {
	char name[40]; //< event name (ex: "atk") or path of sound file
};

/// Holds information on how to correctly draw sprites to create walking characters, item animation and such.
/// Supported versions:
///  v1.? - base version
///  v2.0 - color/zoom/angle/sprType in sprclip, eventId in motion
///  v2.1 - array of events
///  v2.2 - array of delays
///  v2.3 - array of attach points in motion
///  v2.4 - separate x/y zoom in sprclip
///  v2.5 - width/height in sprclip
struct ROAct {
	unsigned short version; //< 0x1?? or 0x200 or 0x201 or 0x202 or 0x203 or 0x205
	unsigned char reserved[10];
	unsigned short actioncount;
	struct ROActAction *actions;
	unsigned int eventcount;
	struct ROActEvent *events; //< (v2.1+)
	float *delays; //< 1 delay per action (default 4.0, v2.2+)
};
#pragma pack(pop)


/// Loads the act from a data buffer. (NULL on error)
ROINT_DLLAPI struct ROAct *act_loadFromData(const unsigned char *data, unsigned int len);
/// Loads the act from a system file. (NULL on error)
ROINT_DLLAPI struct ROAct *act_loadFromFile(const char *fn);
/// Loads the act from a ROGrf file. (NULL on error)
ROINT_DLLAPI struct ROAct *act_loadFromGrf(struct ROGrfFile*);
/// Frees everything inside the ROAct structure allocated by us (including the act itself!)
ROINT_DLLAPI void act_unload(struct ROAct*);


#ifdef __cplusplus
}
#endif 

#endif /* __ROINT_ACT_H */
