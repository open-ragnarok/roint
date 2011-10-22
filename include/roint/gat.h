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
#ifndef __ROINT_GAT_H
#define __ROINT_GAT_H

#ifndef WITHOUT_ROINT_CONFIG
#	include "roint/config.h"
#endif
struct ROGrfFile; // forward declaration

#ifdef __cplusplus
extern "C" {
#endif 


#pragma pack(push,1)
/// Ground Cell.
struct ROGatCell {
	float height[4]; //< west->east, south->north ordering; the lower the number, the higher the ground
	/// Type of cell.
	///  0 - walkable block
	///  1 - non-walkable block
	///  2 - non-walkable water (not snipable)
	///  3 - walkable water
	///  4 - non-walkable water (snipable)
	///  5 - cliff (snipable)
	///  6 - cliff (not snipable)
	///  other - unknown
	int type;
};

/// Ground Altitude File.
/// Supported versions: v1.2
struct ROGat {
	unsigned char vermajor; //< major version
	unsigned char verminor; //< minor version
	unsigned int width;
	unsigned int height;
	struct ROGatCell *cells; //< west->east, south->north ordering
};
#pragma pack(pop)


/// Loads the gat from a data buffer. (NULL on error)
ROINT_DLLAPI struct ROGat *gat_loadFromData(const unsigned char *data, unsigned int len);
/// Loads the gat from a system file. (NULL on error)
ROINT_DLLAPI struct ROGat *gat_loadFromFile(const char *fn);
/// Loads the gat from a ROGrf file. (NULL on error)
ROINT_DLLAPI struct ROGat *gat_loadFromGrf(struct ROGrfFile*);
/// Frees everything inside the ROGat structure allocated by us (including the gat itself!)
ROINT_DLLAPI void gat_unload(struct ROGat*);


#ifdef __cplusplus
}
#endif 

#endif /* __ROINT_GAT_H */
