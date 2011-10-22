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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <roint.h>


int main(int argc, char **argv)
{
	const char *fn;
	struct ROGat *gat;
	unsigned int cellcount;
	unsigned int i;
	int ret;

	if (argc != 2) {
		const char *exe = argv[0];
		printf("Usage:\n  %s file.gat\n", exe);
		return(EXIT_FAILURE);
	}

	fn = argv[1];

	gat = gat_loadFromFile(fn);
	if (gat == NULL) {
		printf("error : failed to load file '%s'\n", fn);
		return(EXIT_FAILURE);
	}
	ret = EXIT_SUCCESS;
	printf("Version: v%u.%u\n", gat->vermajor, gat->verminor);
	if (gat->vermajor == 1 && gat->verminor == 2)
		;// supported (v1.2)
	else {
		printf("error : unknown version\n");
		ret = EXIT_FAILURE;
	}

	cellcount = gat->width * gat->height;
	printf("Cells: %u (%u x %u) (%p)\n", cellcount, gat->width, gat->height, gat->cells);
	for (i = 0; i < cellcount; i++) {
		struct ROGatCell *cell = &gat->cells[i];
		printf("[%u] height={%f,%f,%f,%f} type=%d\n", i,
			cell->height[0], cell->height[1], cell->height[2], cell->height[3], cell->height[4],
			cell->type);
	}
	if (cellcount == 0 && gat->cells != NULL) {
		printf("error : should have NULL cells\n");
		ret = EXIT_FAILURE;
	}

	if (ret == EXIT_SUCCESS)
		printf("OK\n");
	return(ret);
}
