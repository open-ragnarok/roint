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
#ifndef __ROINT_LOG_H
#define __ROINT_LOG_H

#ifndef WITHOUT_ROINT_CONFIG
#	include "roint/config.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*roint_log_func)(const char *fmt, ...);

/// Set a custom log function to use troughout roint.
/// Pass NULL to return to the default function.
ROINT_DLLAPI void roint_set_log_func(roint_log_func);
/// Get log function.
ROINT_DLLAPI roint_log_func roint_get_log_func(void);

#ifdef __cplusplus
}
#endif 

#endif /* __ROINT_LOG_H */
