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

#include <stdarg.h>
#include <stdio.h>


void _console_log_func(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
}


void _null_log_func(const char *fmt, ...) {
}


#ifdef ENABLE_CONSOLE_LOG_FUNC
#	define DEFAULT_LOG_FUNC &_console_log_func
#else
#	define DEFAULT_LOG_FUNC &_null_log_func
#endif
roint_log_func _xlog = DEFAULT_LOG_FUNC;


void roint_set_log_func(roint_log_func x) {
	if (x == NULL)
		_xlog = DEFAULT_LOG_FUNC;
	else
		_xlog = x;
}


roint_log_func roint_get_log_func() {
	return(_xlog);
}
