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
#ifndef __ROINT_CONFIG_H
#define __ROINT_CONFIG_H

// dynamic library
#ifndef _MSC_VER
#cmakedefine ROINT_DLL
#endif


#if defined(_MSC_VER) && defined(ROINT_DLL)
#	ifdef ROINT_INTERNAL
#		define ROINT_DLLAPI  __declspec( dllexport )
#	else
#		define ROINT_DLLAPI  __declspec( dllimport )
#	endif
#else
#	define ROINT_DLLAPI
#endif


// output log messages to the console
#define ROINT_ENABLE_CONSOLE_LOG_FUNC


#endif /* __ROINT_CONFIG_H */
