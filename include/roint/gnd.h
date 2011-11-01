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
#ifndef __ROINT_GND_H
#define __ROINT_GND_H

#ifndef WITHOUT_ROINT_CONFIG
#	include "roint/config.h"
#endif
struct ROGrfFile; // forward declaration

#ifdef __cplusplus
extern "C" {
#endif 


#pragma pack(push,1)
/// Ground Cell.
struct ROGndCell {
	/// Height of the cell corners.
	///  ordering : west->east, south->north
	///  value : lower numbers mean higher ground
	///  zoom : 5 units is equivalent to the side of the cell
	float height[4];
	int topSurfaceId; //< -1 for none
	int frontSurfaceId; //< -1 for none
	int rightSurfaceId; //< -1 for none
};

struct ROGndColor {
	unsigned char b;
	unsigned char g;
	unsigned char r;
	unsigned char a; //< ignored?
};

/// Ground Surface.
struct ROGndSurface {
	float u[4]; //< west->east, south->north
	float v[4]; //< west->east, south->north
	short textureId; //< -1 for none
	short lightmapId; //< -1 for none
	struct ROGndColor color;
};

struct ROGndIntensity {
	unsigned char r;
	unsigned char g;
	unsigned char b;
};

/// Lightmap, represents the shadow projected on the cell.
struct ROGndLightmap {
	unsigned char brightness[8 * 8];
	struct ROGndIntensity intensity[8 * 8];
};

/// Ground File.
/// Supported versions:
///  v1.6 - base version (no save, load-only)
///  v1.7 - lightmap data, 4-byte surfaceIds
struct ROGnd {
	unsigned char vermajor; //< major version
	unsigned char verminor; //< minor version
	unsigned int width;
	unsigned int height;
	float zoom; //< size of the cell side
	unsigned int texturecount;
	char **textures; //< NUL-terminated strings
	unsigned int lightmapcount;
	struct ROGndLightmap *lightmaps;
	unsigned int surfacecount;
	struct ROGndSurface *surfaces;
	struct ROGndCell *cells;
};
#pragma pack(pop)


/// Inspects the gnd data and returns the first compatible version. (0 if invalid)
ROINT_DLLAPI unsigned short gnd_inspect(const struct ROGnd *gnd);
/// Loads the gnd from a data buffer. (NULL on error)
ROINT_DLLAPI struct ROGnd *gnd_loadFromData(const unsigned char *data, unsigned long len);
/// Loads the gnd from a system file. (NULL on error)
ROINT_DLLAPI struct ROGnd *gnd_loadFromFile(const char *fn);
/// Loads the gnd from a ROGrf file. (NULL on error)
ROINT_DLLAPI struct ROGnd *gnd_loadFromGrf(struct ROGrfFile*);
/// Saves the gnd to a data buffer. Discards incompatible information. (0 on success)
/// WARNING : the 'data_out' data has to be released with the roint free function
ROINT_DLLAPI int gnd_saveToData(const struct ROGnd *gnd, unsigned char **data_out, unsigned long *size_out);
/// Saves the gnd to a system file. Discards incompatible information. (0 on success)
ROINT_DLLAPI int gnd_saveToFile(const struct ROGnd *gnd, const char *fn);
/// Frees everything inside the ROGnd structure allocated by us (including the gnd itself!)
ROINT_DLLAPI void gnd_unload(struct ROGnd *gnd);


#ifdef __cplusplus
}
#endif 

#endif /* __ROINT_GND_H */
