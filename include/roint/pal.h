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
#ifndef __ROINT_PAL_H
#define __ROINT_PAL_H

#ifndef WITHOUT_ROINT_CONFIG
#	include "roint/config.h"
#endif
struct ROGrfFile; // forward declaration

#ifdef __cplusplus
extern "C" {
#endif 


#pragma pack(push,1)
/// Palette entry.
/// Represents a color with red, green and blue components.
struct ROPalColor {
	unsigned char r, g, b, reserved;
};

/// Palette.
/// Contains an array of 256 palette entries.
/// The first index is sometimes considered invisible.
struct ROPal {
	struct ROPalColor pal[256];
};
#pragma pack(pop)


/// Loads the pal from a data buffer. (NULL on error)
ROINT_DLLAPI struct ROPal *pal_loadFromData(const unsigned char *data, unsigned long len);
/// Loads the pal from a system file. (NULL on error)
ROINT_DLLAPI struct ROPal *pal_loadFromFile(const char *fn);
/// Loads the pal from a ROGrf file. (NULL on error)
ROINT_DLLAPI struct ROPal *pal_loadFromGrf(struct ROGrfFile*);
/// Saves the pal to a data buffer. (0 on success)
/// WARNING : the 'data_out' data has to be released with the roint free function
ROINT_DLLAPI int pal_saveToData(struct ROPal *pal, unsigned char **data, unsigned long *len);
/// Saves the pal to a system file. (0 on success)
ROINT_DLLAPI int pal_saveToFile(struct ROPal *pal, const char *fn);
/// Frees everything inside the ROPal structure allocated by us (including the pal itself!)
ROINT_DLLAPI void pal_unload(struct ROPal*);


#ifdef __cplusplus
}
#endif 

#endif /* __ROINT_PAL_H */
