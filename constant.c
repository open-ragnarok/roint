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

#include <string.h> // strcmp

struct s_roint_constant {
	const char *name;
	const char *value;
};

const struct s_roint_constant constants[] = {
	{"user_interface", "\xc0\xaf\xc0\xfa\xc0\xce\xc5\xcd\xc6\xe4\xc0\xcc\xbd\xba"},

// BODY PARTS
	{"body", "\xb8\xf6\xc5\xeb"},
	{"head", "\xb8\xd3\xb8\xae\xc5\xeb"},
	{"head2", "\xb8\xd3\xb8\xae"}, // Used by palette
	{"weapons", "\xb8\xf9\xc5\xa9"},

//CHARACTER TYPE
	{"humans", "\xc0\xce\xb0\xa3\xc1\xb7"}, // Human family
	{"monsters", "\xb8\xf3\xbd\xba\xc5\xcd"},

//
	{"shields", "\xb9\xe6\xc6\xd0"},
	{"items", "\xbe\xc6\xc0\xcc\xc5\xdb"},
	{"accessories", "\xbe\xc7\xbc\xbc\xbb\xe7\xb8\xae"}, // Head accessories (Helm)
	{"effects", "\xc0\xcc\xc6\xd1\xc6\xae"},

	{"water", "\xbf\xf6\xc5\xcd"},

// === SEX
	{"female", "\xbf\xa9"},
	{"male", "\xb3\xb2"},

// Weapons
	{"sword", "\xb0\xcb"},
	{"dagger", "\xb4\xdc\xb0\xcb"},
	{"axe", "\xb5\xb5\xb3\xa2"},
	{"staff", "\xb7\xd4\xb5\xe5"},
	{"mace", "\xc5\xac\xb7\xb4"},
	{"spear", "\xc3\xa2\x0d\x0a"},
	{"claw", "\xb3\xca\xc5\xac"},
	{"book", "\xc3\xa5"},
	{"katar", "\xc4\xab\xc5\xb8\xb8\xa3"},
	{"bow", "\xc8\xb0"},
	{"instrument", "\xbe\xc7\xb1\xe2"},
	{"whip", "\xc3\xa4\xc2\xef"},

	{"afterimage", "\xb0\xcb\xb1\xa4"}, // Weapons's after image

//	{"arrow", "\x\x\x\x"},

// classes [kR105]
	{"novice", "\xC3\xCA\xBA\xB8\xC0\xDA"},
	{"swordsman", "\xB0\xCB\xBB\xE7"},
	{"magician", "\xB8\xB6\xB9\xFD\xBB\xE7"},
	{"archer", "\xB1\xC3\xBC\xF6"},
	{"acolyte", "\xBC\xBA\xC1\xF7\x9F\xE0"},
	{"merchant", "\xBB\xF3\xC0\xCE"},
	{"thief", "\xB5\xB5\xB5\xCF"},
	{"knight", "\xB1\xE2\xBB\xE7"},
	{"priest", "\xC7\xC1\xB8\xAE\xBD\xBA\xC6\xAE"},
	{"wizard", "\xC0\xA7\xC0\xFA\xB5\xE5"},
	{"blacksmith", "\xC1\xA6\xC3\xB6"},
	{"hunter", "\xC7\xE5\xC5\xCD"},
	{"assassin", "\xBE\xEE\xBC\xBC\xBD\xC5"},
	{"knight_mounted", "\xC6\xE4\xC4\xDA\xC6\xE4\xC4\xDA\x5F\xB1\xE2\xBB\xE7"},
	{"crusader", "\xC5\xA9\xB7\xE7\xBC\xBC\xC0\xCC\xB4\xF5"},
	{"monk", "\xB8\xF9\xC5\xA9"},
	{"sage", "\xBC\xBC\xC0\xCC\xC1\xF6"},
	{"rogue", "\xB7\xCE\xB1\xD7"},
	{"alchemist", "\xBF\xAC\xB1\xDD\xBC\xFA\xBB\xE7"},
	{"bard", "\xB9\xD9\xB5\xE5"},
	{"dancer", "\xB9\xAB\xC8\xF1"},
	{"crusader_mounted", "\xBD\xC5\xC6\xE4\xC4\xDA\xC5\xA9\xB7\xE7\xBC\xBC\xC0\xCC\xB4\xF5"},
	{"gm", "\x9D\xC4\xBF\xB5\xC0\xDA"},
	{"mercenary", "\xBF\xEB\xBA\xB4"}

};
#define CONSTANT_COUNT sizeof(constants)/sizeof(constants[0])

/// number of constants
size_t roint_constant_count(void) {
	return(CONSTANT_COUNT);
}

/// get constant name by index (NULL on error)
const char *roint_constant_name(size_t idx) {
	if (idx < CONSTANT_COUNT)
		return(constants[idx].name);
	return(NULL);
}

/// get constant value by index (NULL on error)
const char *roint_constant_value(size_t idx) {
	if (idx < CONSTANT_COUNT)
		return(constants[idx].value);
	return(NULL);
}

/// get constant value by name (NULL on error)
const char *roint_constant_name2value(const char *name) {
	size_t i;
	if (name == NULL)
		return(NULL);
	for (i = 0; i < CONSTANT_COUNT; ++i) {
		if (strcmp(constants[i].name, name) == 0)
			return(constants[i].value);
	}
	return(NULL);
}
