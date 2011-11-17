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
#ifndef __ROINT_STR_H
#define __ROINT_STR_H

#ifndef WITHOUT_ROINT_CONFIG
#	include "roint/config.h"
#endif
struct ROGrfFile; // forward declaration

#ifdef __cplusplus
extern "C" {
#endif 


#pragma pack(push,1)
/// Animation Key Frame.
///
/// Vertices:
///   (u,v),(u2,v2)
///  /
/// 0---1 - (u+us,v),(u2+us2,v2)
/// |   |
/// 3---2 - (u+us,v+vs),(u2+us2,v2+vs2)
///  \
///   (u,v+vs),(u2,v2+vs2)
///
/// Coordinates:
///  __x   __u
/// |     |
/// y     v
///
/// A normal keyframe provides raw data of a frame.
/// A morph keyframe:
/// - only appears next to a normal keyframe with the same framenum
/// - affects frames after framenum and before the next keyframe
/// - adds x,y,u,v,us,vs,u2,v2,us2,vs2,ax,ay,rz,crR,crG,crB,crA per frame
/// - modifies aniframe according to anitype
struct ROStrKeyFrame {
	unsigned int framenum; //< Frame number.
	unsigned int type; //< Frame type. 0 - normal keyframe; 1 - morpth keyframe
	float x;
	float y;
	float u;
	float v;
	float us;
	float vs;
	float u2;
	float v2;
	float us2;
	float vs2;
	float ax[4]; //< vertice coordinates
	float ay[4]; //< vertice coordinates
	float aniframe; //< textureId
	/// Texture animation type.
	/// Determines how aniframe morphs per frame.
	///  0 - no change
	///  1 - ]-inf,inf[
	///    aniframe += keyframe.aniframe; // add aniframe
	///  2 - ]-inf,ROStrLayer.texturecount[
	///    aniframe += keyframe.anidelta; // add anidelta
	///    if (aniframe >= ROStrLayer.texturecount) // stop on limit
	///      aniframe = ROStrLayer.texturecount - 1.0f;
	///  3 - [0.0,ROStrLayer.texturecount[
	///    aniframe += keyframe.anidelta; // add anidelta
	///    if (aniframe >= ROStrLayer.texturecount) // loop when over
	///      aniframe -= (float)(int)(aniframe / ROStrLayer.texturecount) * ROStrLayer.texturecount;
	///  4 - [0.0,ROStrLayer.texturecount[  WARNING broken on 2004 client
	///    aniframe -= keyframe.anidelta; // subtract anidelta
	///    if (aniframe < 0.0f) { // loop when under
	///      aniframe -= (float)(int)(aniframe / ROStrLayer.texturecount) * ROStrLayer.texturecount;
	///      if (aniframe < 0.0f)
	///        aniframe += ROStrLayer.texturecount;
	///    }
	///  5 - [0,ROStrLayer.texturecount - 1]  randomize with anidelta seed???
	///    int value = (int)((curframe - keyframe.framenum) * keyframe.anidelta + aniframe);
	///    int lasttex = ROStrLayer.texturecount - 1;
	///    int n = value / lasttex;
	///    if (n & 1)
	///      aniframe = lasttex * (n + 1) - value;
	///    else
	///      aniframe = value - lasttex * n;
	unsigned int anitype;
	float anidelta;
	float rz; //< Rotation [0,1024[ is equivalent to [0,360[ degrees
	float crR; //< Red color component of vertices [0,255]
	float crG; //< Green color component of vertices [0,255]
	float crB; //< Blue color component of vertices [0,255]
	float crA; //< Alpha color component of vertices [0,255]
	unsigned int srcalpha; //< Source blend mode (D3DBLEND_*)
	unsigned int destalpha; //< Destination blend mode (D3DBLEND_*)
	/// Multi-texture mode.
	///  1 - TODO
	///  2 - TODO
	///  3 - TODO
	///  4 - TODO
	///  5 - TODO
	///  6 - TODO
	///  7 - TODO
	///  8 - TODO
	///  9 - TODO
	///  10 - TODO
	///  11 - TODO
	///  12 - TODO
	///  13 - TODO
	///  14 - TODO
	///  15 - TODO
	unsigned int mtpreset;
};

/// Texture pathname. (defaults to "1.bmp")
struct ROStrTexture {
	char name[128]; //< NUL-terminated string
};

/// Animation layer/track. Contains a sequence keyframes and a list of textures being used.
struct ROStrLayer {
	unsigned int texturecount;
	struct ROStrTexture *textures;
	unsigned int keyframecount;
	struct ROStrKeyFrame *keyframes;
};

/// Effect Animation.
/// Official client supports 60 tracks, 110 textures per track.
struct ROStr {
	unsigned int version;
	unsigned int framecount; //< number of frames
	unsigned int fps; //< frames per second
	unsigned int layercount;
	unsigned char reserved[16];
	struct ROStrLayer *layers; //< animation layers/tracks
};
#pragma pack(pop)


/// Inspects the str data and returns the first compatible version. (0 if invalid)
ROINT_DLLAPI unsigned int str_inspect(const struct ROStr *str);
/// Loads the str from a data buffer. (NULL on error)
ROINT_DLLAPI struct ROStr *str_loadFromData(const unsigned char *data, unsigned long len);
/// Loads the str from a system file. (NULL on error)
ROINT_DLLAPI struct ROStr *str_loadFromFile(const char *fn);
/// Loads the str from a ROGrf file. (NULL on error)
ROINT_DLLAPI struct ROStr *str_loadFromGrf(struct ROGrfFile*);
/// Saves the str to a data buffer. (0 on success)
/// WARNING : the 'data_out' data has to be released with the roint free function
ROINT_DLLAPI int str_saveToData(const struct ROStr *str, unsigned char **data_out, unsigned long *size_out);
/// Saves the str to a system file. (0 on success)
ROINT_DLLAPI int str_saveToFile(const struct ROStr *str, const char *fn);
/// Frees everything inside the ROStr structure allocated by us (including the str itself!)
ROINT_DLLAPI void str_unload(struct ROStr *str);


#ifdef __cplusplus
}
#endif 

#endif /* __ROINT_STR_H */
