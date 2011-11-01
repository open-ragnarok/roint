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


//#define SKIP_PRINT_CELL


int gat_equal(struct ROGat *gat, struct ROGat *gat2) {
	unsigned int cellcount;
	unsigned int i;
	if (gat2->vermajor != gat->vermajor ||
		gat2->verminor != gat->verminor ||
		gat2->width != gat->width ||
		gat2->height != gat->height)
		return(0);
	cellcount = gat->width * gat->height;
	for (i = 0; i < cellcount; i++) {
		struct ROGatCell *cell = &gat->cells[i];
		struct ROGatCell *cell2 = &gat2->cells[i];
		if (cell2->height[0] != cell->height[0] ||
			cell2->height[1] != cell->height[1] ||
			cell2->height[2] != cell->height[2] ||
			cell2->height[3] != cell->height[3] ||
			cell2->type != cell->type)
			return(0);
	}
	return(1);
}


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
	printf("Inspect: 0x%x\n", gat_inspect(gat));
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
#ifndef SKIP_PRINT_CELL
		printf("[%u] height={%f,%f,%f,%f} type=%d\n", i,
			cell->height[0], cell->height[1], cell->height[2], cell->height[3], cell->height[4],
			cell->type);
#endif
	}
	if (cellcount == 0 && gat->cells != NULL) {
		printf("error : should have NULL cells\n");
		ret = EXIT_FAILURE;
	}

	{// test save to data
		unsigned char *data = NULL;
		unsigned long length = 0;
		struct ROGat *gat2;
		printf("Save: %d\n", gat_saveToData(gat, &data, &length));
		printf("Data: %p %u\n", data, length);
		gat2 = gat_loadFromData(data,length);
		if (data == NULL) {
			printf("error : saving produced NULL data\n");
			ret = EXIT_FAILURE;
		}
		else if (gat2 == NULL) {
			printf("error : saving produced invalid data\n");
			ret = EXIT_FAILURE;
		}
		else {
			printf("Inspect: 0x%x\n", gat_inspect(gat2));
			if (!gat_equal(gat, gat2)) {
				printf("error : saving produced different data\n");
				ret = EXIT_FAILURE;
			}
		}
		if (data != NULL)
			get_roint_free_func()(data);
		gat_unload(gat2);
	}

	{// test save to file
		const char *fn = "test_save.gat";
		struct ROGat *gat2;
		printf("Save: %d\n", gat_saveToFile(gat, fn));
		gat2 = gat_loadFromFile(fn);
		if (gat2 == NULL) {
			printf("error : saving produced invalid data or no file\n");
			ret = EXIT_FAILURE;
		}
		else {
			printf("Inspect: 0x%x\n", gat_inspect(gat2));
			if (!gat_equal(gat, gat2)) {
				printf("error : saving produced different data\n");
				ret = EXIT_FAILURE;
			}
		}
		gat_unload(gat2);
	}

	gat_unload(gat);
	if (ret == EXIT_SUCCESS)
		printf("OK\n");
	return(ret);
}
