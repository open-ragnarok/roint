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

Welcome to the OpenRagnarok pure-C interface library.

  The goal of this library is to provide comprehensible access to all files and 
resources provided by Gravity in its awesome game.
  This project is part of a larger project to try to implement an fully-working client
of the game, as well as some browsers, analyzers and viewers.

  As of this writing, the library has read-only capabilities, but the goal is to be
a full-features library, so it can be used on auto-patching as well.

  To get the code working, you could make use of CMake to create the appropriate build
environment.

  In a pinch, you can create an dynamic library build with the following command:

$ cmake -G "Unix Makefiles" <path-to-roint>

  Or, if you want to use Apple's XCode:

$ cmake -G Xcode <path-to-roint>

