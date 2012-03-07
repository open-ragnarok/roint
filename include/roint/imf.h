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
#ifndef __ROINT_IMF_H
#define __ROINT_IMF_H
/// \file
/// \ingroup FileFormatHeaders
/// IMF file format. Player sprite interaction.

#ifdef ROINT_INTERNAL
#	include "config.h"
#elif !defined(WITHOUT_ROINT_CONFIG)
#	include "roint/config.h"
#endif

#ifndef ROINT_DLLAPI
#	define ROINT_DLLAPI
#endif
struct ROGrfFile; // forward declaration

#ifdef __cplusplus
extern "C" {
#endif 


#pragma pack(push,1)
/// Imf Motion.
struct ROImfMotion {
	int priority; ///< Priority.
	int cx; ///< X offset.
	int cy; ///< Y offset.
};

/// Imf Action.
struct ROImfAction {
	unsigned int motioncount; ///< Number of motions.
	struct ROImfMotion* motions; ///< Motions.
};

/// Imf Layer.
struct ROImfLayer {
	unsigned int actioncount; ///< Number of actions.
	/// Actions.
	/// \note original client supports 104 actions
	struct ROImfAction* actions;
};

/// Sprite Interaction.
///
/// Determines the offset of the player character body and head sprites in the selection screen.\n
/// Determines the drawing order between the weapon and shield or between weapons of a player character.
/// \note body is layer 0, head is layer 1
struct ROImf {
	/// Supported versions:
	/// - 1.01 - base version
	float version;
	/// Checksum of content.
	/// \note ignored by original client
	unsigned int checksum;
	int lastlayer; ///< Index of the last layer. (-1 if empty)
	/// Layers. (lastlayer + 1 layers)
	/// \note original client supports 15 layers
	struct ROImfLayer* layers;
};
#pragma pack(pop)


/// Inspects the imf data and returns the first compatible version. (0.0f if invalid)
ROINT_DLLAPI float imf_inspect(const struct ROImf *imf);
/// Loads the str from a data buffer. (NULL on error)
ROINT_DLLAPI struct ROImf *imf_loadFromData(const unsigned char *data, unsigned long len);
/// Loads the str from a system file. (NULL on error)
ROINT_DLLAPI struct ROImf *imf_loadFromFile(const char *fn);
/// Loads the str from a ROGrf file. (NULL on error)
ROINT_DLLAPI struct ROImf *imf_loadFromGrf(struct ROGrfFile*);
/// Saves the imf to a data buffer. (0 on success)
/// \warning the 'data_out' data has to be released with the roint free function
ROINT_DLLAPI int imf_saveToData(const struct ROImf *imf, unsigned char **data_out, unsigned long *size_out);
/// Saves the imf to a system file. (0 on success)
ROINT_DLLAPI int imf_saveToFile(const struct ROImf *imf, const char *fn);
/// Frees everything inside the ROImf structure allocated by us (including the imf itself!)
ROINT_DLLAPI void imf_unload(struct ROImf *imf);


#ifdef __cplusplus
}
#endif 

#endif /* __ROINT_IMF_H */
