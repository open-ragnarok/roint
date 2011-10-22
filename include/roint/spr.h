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
#ifndef __ROINT_SPR_H
#define __ROINT_SPR_H

#ifndef WITHOUT_ROINT_CONFIG
#	include "roint/config.h"
#endif
struct ROGrfFile; // forward declaration
struct ROPal; // forward declaration

#ifdef __cplusplus
extern "C" {
#endif 


#pragma pack(push,1)
/// RGBA pixel.
struct ROSprColor {
	unsigned char a;
	unsigned char b;
	unsigned char g;
	unsigned char r;
};

/// Palette Image.
struct ROSprPalImage {
	unsigned short width;
	unsigned short height;
	unsigned char *data; //< palette indexes; left to right, top to bottom ordering
};

/// RGBA Image.
struct ROSprRgbaImage {
	unsigned short width;
	unsigned short height;
	struct ROSprColor *data; //< color pixels; left to right, bottom to top ordering
};

/// Sprite.
/// Contains palette images, RGBA images (v2.0+) and a palette (v1.1+).
/// Supported versions: v1.0 v1.1 v2.0 v2.1
struct ROSpr {
	unsigned short version;//< 0x100 or 0x101 or 0x200 or 0x201
	unsigned short palimagecount;
	struct ROSprPalImage *palimages;
	unsigned short rgbaimagecount;
	struct ROSprRgbaImage *rgbaimages;
	struct ROPal *pal;
};
#pragma pack(pop)


/// Loads the spr from a data buffer. (NULL on error)
ROINT_DLLAPI struct ROSpr *spr_loadFromData(const unsigned char *data, unsigned int len);
/// Loads the spr from the ROGrf structure file. (NULL on error)
ROINT_DLLAPI struct ROSpr *spr_loadFromGrf(struct ROGrfFile*);
/// Frees everything inside the ROSpr structure allocated by us (including the spr itself!)
ROINT_DLLAPI void spr_unload(struct ROSpr*);


#ifdef __cplusplus
}
#endif 

#endif /* __ROINT_SPR_H */
