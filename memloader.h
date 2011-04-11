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
#ifndef __ROINT_MEMLOADER_H
#define __ROINT_MEMLOADER_H

// For ROInt internal use only

struct _memloader;

struct _memloader *loader_init(const unsigned char *ptr, unsigned long size);
void loader_free(struct _memloader *loader);

void loader_read(void *dest, unsigned int size, unsigned int count, struct _memloader *loader);
unsigned long loader_tell(const struct _memloader *loader);
int loader_seek(struct _memloader *loader, long pos, int origin);


#endif /* __ROINT_MEMLOADER_H */