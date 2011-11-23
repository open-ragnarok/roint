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
/// \file constant.c
/// Constants utility.
#include "internal.h"

#include <string.h> // strcmp

/// Constant.
struct _roint_constant {
	const char *name; ///< Constant name.
	const char *value; ///< Constant value.
	const char *description; ///< Constant description. 
};

/// List of constants.
const struct _roint_constant constants[] = {
	// paths related to file formats
	{"path.file.ebm", "_tmpEmblem\\", "path to ebm files"},
	{"path.file.gat", "data\\", "path to gat files"},
	{"path.file.gnd", "data\\", "path to gnd files"},
	{"path.file.rsw", "data\\", "path to rsw files"},
	{"path.file.imf", "data\\imf\\", "path to imf files"},
	{"path.file.rma", "data\\model\\", "path to rma files"},
	{"path.file.rsm", "data\\model\\", "path to rsm files"},
	{"path.file.rsx", "data\\model\\", "path to rsx files"},
	{"path.file.gr2", "data\\model\\3dmob\\", "path to gr2 files"},
	{"path.file.pal", "data\\palette\\", "path to pal files"},
	{"path.file.act", "data\\sprite\\", "path to act files"},
	{"path.file.spr", "data\\sprite\\", "path to spr files"},
	{"path.file.bmp", "data\\texture\\", "path to bmp files"},
	{"path.file.jpg", "data\\texture\\", "path to jpg files"},
	{"path.file.tga", "data\\texture\\", "path to tga files"},
	{"path.file.str", "data\\texture\\effect\\", "path to str files"},
	{"path.file.str.texture", "effect\\", "subpath of the str textures"},
	{"path.file.wav", "data\\wav\\", "path to wav files"},

//---------------//
// old constants //
//---------------//
	{"user_interface", "\xc0\xaf\xc0\xfa\xc0\xce\xc5\xcd\xc6\xe4\xc0\xcc\xbd\xba", "TODO old constant"},

// BODY PARTS
	{"body", "\xb8\xf6\xc5\xeb", "TODO old constant"},
	{"head", "\xb8\xd3\xb8\xae\xc5\xeb", "TODO old constant"},
	{"head2", "\xb8\xd3\xb8\xae", "TODO old constant"}, // Used by palette
	{"weapons", "\xb8\xf9\xc5\xa9", "TODO old constant"},

//CHARACTER TYPE
	{"humans", "\xc0\xce\xb0\xa3\xc1\xb7", "TODO old constant"}, // Human family
	{"monsters", "\xb8\xf3\xbd\xba\xc5\xcd", "TODO old constant"},

//
	{"shields", "\xb9\xe6\xc6\xd0", "TODO old constant"},
	{"items", "\xbe\xc6\xc0\xcc\xc5\xdb", "TODO old constant"},
	{"accessories", "\xbe\xc7\xbc\xbc\xbb\xe7\xb8\xae", "TODO old constant"}, // Head accessories (Helm)
	{"effects", "\xc0\xcc\xc6\xd1\xc6\xae", "TODO old constant"},

	{"water", "\xbf\xf6\xc5\xcd", "TODO old constant"},

// === SEX
	{"female", "\xbf\xa9", "TODO old constant"},
	{"male", "\xb3\xb2", "TODO old constant"},

// Weapons
	{"sword", "\xb0\xcb", "TODO old constant"},
	{"dagger", "\xb4\xdc\xb0\xcb", "TODO old constant"},
	{"axe", "\xb5\xb5\xb3\xa2", "TODO old constant"},
	{"staff", "\xb7\xd4\xb5\xe5", "TODO old constant"},
	{"mace", "\xc5\xac\xb7\xb4", "TODO old constant"},
	{"spear", "\xc3\xa2\x0d\x0a", "TODO old constant"},
	{"claw", "\xb3\xca\xc5\xac", "TODO old constant"},
	{"book", "\xc3\xa5", "TODO old constant"},
	{"katar", "\xc4\xab\xc5\xb8\xb8\xa3", "TODO old constant"},
	{"bow", "\xc8\xb0", "TODO old constant"},
	{"instrument", "\xbe\xc7\xb1\xe2", "TODO old constant"},
	{"whip", "\xc3\xa4\xc2\xef", "TODO old constant"},

	{"afterimage", "\xb0\xcb\xb1\xa4", "TODO old constant"}, // Weapons's after image

//	{"arrow", "\x\x\x\x", "TODO old constant"},

// classes [kR105]
	{"novice", "\xC3\xCA\xBA\xB8\xC0\xDA", "TODO old constant"},
	{"swordsman", "\xB0\xCB\xBB\xE7", "TODO old constant"},
	{"magician", "\xB8\xB6\xB9\xFD\xBB\xE7", "TODO old constant"},
	{"archer", "\xB1\xC3\xBC\xF6", "TODO old constant"},
	{"acolyte", "\xBC\xBA\xC1\xF7\x9F\xE0", "TODO old constant"},
	{"merchant", "\xBB\xF3\xC0\xCE", "TODO old constant"},
	{"thief", "\xB5\xB5\xB5\xCF", "TODO old constant"},
	{"knight", "\xB1\xE2\xBB\xE7", "TODO old constant"},
	{"priest", "\xC7\xC1\xB8\xAE\xBD\xBA\xC6\xAE", "TODO old constant"},
	{"wizard", "\xC0\xA7\xC0\xFA\xB5\xE5", "TODO old constant"},
	{"blacksmith", "\xC1\xA6\xC3\xB6", "TODO old constant"},
	{"hunter", "\xC7\xE5\xC5\xCD", "TODO old constant"},
	{"assassin", "\xBE\xEE\xBC\xBC\xBD\xC5", "TODO old constant"},
	{"knight_mounted", "\xC6\xE4\xC4\xDA\xC6\xE4\xC4\xDA\x5F\xB1\xE2\xBB\xE7", "TODO old constant"},
	{"crusader", "\xC5\xA9\xB7\xE7\xBC\xBC\xC0\xCC\xB4\xF5", "TODO old constant"},
	{"monk", "\xB8\xF9\xC5\xA9", "TODO old constant"},
	{"sage", "\xBC\xBC\xC0\xCC\xC1\xF6", "TODO old constant"},
	{"rogue", "\xB7\xCE\xB1\xD7", "TODO old constant"},
	{"alchemist", "\xBF\xAC\xB1\xDD\xBC\xFA\xBB\xE7", "TODO old constant"},
	{"bard", "\xB9\xD9\xB5\xE5", "TODO old constant"},
	{"dancer", "\xB9\xAB\xC8\xF1", "TODO old constant"},
	{"crusader_mounted", "\xBD\xC5\xC6\xE4\xC4\xDA\xC5\xA9\xB7\xE7\xBC\xBC\xC0\xCC\xB4\xF5", "TODO old constant"},
	{"gm", "\x9D\xC4\xBF\xB5\xC0\xDA", "TODO old constant"},
	{"mercenary", "\xBF\xEB\xBA\xB4", "TODO old constant"}

};
/// Number of constants.
#define CONSTANT_COUNT sizeof(constants)/sizeof(constants[0])

unsigned int roint_constant_count(void) {
	return(CONSTANT_COUNT);
}

const char *roint_constant_name(unsigned int idx) {
	if (idx < CONSTANT_COUNT)
		return(constants[idx].name);
	return(NULL);
}

const char *roint_constant_value(unsigned int idx) {
	if (idx < CONSTANT_COUNT)
		return(constants[idx].value);
	return(NULL);
}

const char *roint_constant_description(unsigned int idx) {
	if (idx < CONSTANT_COUNT)
		return(constants[idx].description);
	return(NULL);
}

const char *roint_constant_name2value(const char *name) {
	unsigned int i;
	if (name == NULL)
		return(NULL);
	for (i = 0; i < CONSTANT_COUNT; ++i) {
		if (strcmp(constants[i].name, name) == 0)
			return(constants[i].value);
	}
	return(NULL);
}

const char *roint_constant_name2description(const char *name) {
	unsigned int i;
	if (name == NULL)
		return(NULL);
	for (i = 0; i < CONSTANT_COUNT; ++i) {
		if (strcmp(constants[i].name, name) == 0)
			return(constants[i].description);
	}
	return(NULL);
}
