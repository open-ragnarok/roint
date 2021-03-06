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
#ifndef __ROINT_CONSTANT_H
#define __ROINT_CONSTANT_H
/// \file roint/constant.h
/// \ingroup UtilityHeaders
/// RO related constants.

#ifdef ROINT_INTERNAL
#	include "config.h"
#elif !defined(WITHOUT_ROINT_CONFIG)
#	include "roint/config.h"
#endif

#ifndef ROINT_DLLAPI
#	define ROINT_DLLAPI
#endif

#ifdef __cplusplus
extern "C" {
#endif

/// Get number of constants.
ROINT_DLLAPI unsigned int roint_constant_count(void);

/// Get constant name by index. (NULL on error)
ROINT_DLLAPI const char *roint_constant_name(unsigned int idx);

/// Get constant value by index. (NULL on error)
ROINT_DLLAPI const char *roint_constant_value(unsigned int idx);

/// Get constant description by index. (NULL on error)
ROINT_DLLAPI const char *roint_constant_description(unsigned int idx);

/// Get constant value by name. (NULL on error)
ROINT_DLLAPI const char *roint_constant_name2value(const char *name);

/// Get constant description by name. (NULL on error)
ROINT_DLLAPI const char *roint_constant_name2description(const char *name);

#ifdef __cplusplus
}
#endif 

#endif /* __ROINT_CONSTANT_H */
