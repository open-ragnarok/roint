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


//#define SKIP_PRINT_LIGHTMAP
//#define SKIP_PRINT_SURFACE
//#define SKIP_PRINT_CELL


int gnd_equal(struct ROGnd *gnd, struct ROGnd *gnd2) {
	unsigned int cellcount;
	unsigned int i;
	if (gnd2->vermajor != gnd->vermajor ||
		gnd2->verminor != gnd->verminor ||
		gnd2->width != gnd->width ||
		gnd2->height != gnd->height ||
		gnd2->zoom != gnd->zoom ||
		gnd2->texturecount != gnd->texturecount ||
		gnd2->lightmapcount != gnd->lightmapcount ||
		gnd2->surfacecount != gnd->surfacecount)
		return(0);
	for (i = 0; i < gnd->texturecount; i++) {
		char *texture = gnd->textures[i];
		char *texture2 = gnd2->textures[i];
		if (strcmp(texture, texture2) != 0)
			return(0);
	}
	for (i = 0; i < gnd->lightmapcount; i++) {
		struct ROGndLightmap *lightmap = &gnd->lightmaps[i];
		struct ROGndLightmap *lightmap2 = &gnd2->lightmaps[i];
		if (memcmp(lightmap, lightmap2, sizeof(struct ROGndLightmap)) != 0)
			return(0);
	}
	for (i = 0; i < gnd->surfacecount; i++) {
		struct ROGndSurface *surface = &gnd->surfaces[i];
		struct ROGndSurface *surface2 = &gnd2->surfaces[i];
		if (memcmp(surface, surface2, sizeof(struct ROGndSurface)) != 0)
			return(0);
	}
	cellcount = gnd->width * gnd->height;
	for (i = 0; i < cellcount; i++) {
		struct ROGndCell *cell = &gnd->cells[i];
		struct ROGndCell *cell2 = &gnd2->cells[i];
		if (memcmp(cell, cell2, sizeof(struct ROGndCell)) != 0)
			return(0);
	}
	return(1);
}


int main(int argc, char **argv)
{
	const char *fn;
	struct ROGnd *gnd;
	unsigned int cellcount;
	unsigned int i;
	int ret;

	if (argc != 2) {
		const char *exe = argv[0];
		printf("Usage:\n  %s file.gnd\n", exe);
		return(EXIT_FAILURE);
	}

	fn = argv[1];

	gnd = gnd_loadFromFile(fn);
	if (gnd == NULL) {
		printf("error : failed to load file '%s'\n", fn);
		return(EXIT_FAILURE);
	}
	ret = EXIT_SUCCESS;
	printf("Inspect: 0x%X\n", gnd_inspect(gnd));
	printf("Version: v%u.%u\n", gnd->vermajor, gnd->verminor);
	if (gnd->vermajor == 1 && (gnd->verminor == 6 || gnd->verminor == 7))
		;// supported (v1.6 or v1.7)
	else {
		printf("error : unknown version\n");
		ret = EXIT_FAILURE;
	}
	printf("Zoom: %f\n", gnd->zoom);
	printf("Textures: %u (%p)\n", gnd->texturecount, gnd->textures);
	for (i = 0; i < gnd->texturecount; i++) {
		char *texture = gnd->textures[i];
		printf("[%u] \"%s\"\n", i, texture, texture);
	}
	printf("Lightmaps: %u (%p)\n", gnd->lightmapcount, gnd->lightmaps);
	for (i = 0; i < gnd->lightmapcount; i++) {
		struct ROGndLightmap *lightmap = &gnd->lightmaps[i];
#ifndef SKIP_PRINT_LIGHTMAP
		unsigned int j;
		for (j = 0; j < 8 * 8; j++)
			printf("[%u][%u] brightness=%u intensity.r=%u intensity.g=%u intensity.b=%u\n", i, j,
				lightmap->brightness[j], lightmap->intensity[j].r, lightmap->intensity[j].g, lightmap->intensity[j].b);
#endif
	}
	printf("Surfaces: %u (%p)\n", gnd->surfacecount, gnd->surfaces);
	for (i = 0; i < gnd->surfacecount; i++) {
		struct ROGndSurface *surface = &gnd->surfaces[i];
#ifndef SKIP_PRINT_SURFACE
		printf("[%u] u={%f,%f,%f,%f}\n", i, surface->u[0], surface->u[1], surface->u[2], surface->u[3]);
		printf("[%u] v={%f,%f,%f,%f}\n", i, surface->v[0], surface->v[1], surface->v[2], surface->v[3]);
		printf("[%u] textureId=%d\n", i, surface->textureId);
		printf("[%u] lightmapId=%d\n", i, surface->lightmapId);
		printf("[%u] color={b=%u,g=%u,r=%u,a=%u}\n", i, surface->color.b, surface->color.g, surface->color.r, surface->color.a);
#endif
	}
	cellcount = gnd->width * gnd->height;
	printf("Cells: %u (%u x %u) (%p)\n", cellcount, gnd->width, gnd->height, gnd->cells);
	for (i = 0; i < cellcount; i++) {
		struct ROGndCell *cell = &gnd->cells[i];
#ifndef SKIP_PRINT_CELL
		printf("[%u] height={%f,%f,%f,%f}\n", i, cell->height[0], cell->height[1], cell->height[2], cell->height[3], cell->height[4]);
		printf("[%u] topSurfaceId=%d\n", i, cell->topSurfaceId);
		printf("[%u] frontSurfaceId=%d\n", i, cell->frontSurfaceId);
		printf("[%u] rightSurfaceId=%d\n", i, cell->rightSurfaceId);
#endif
	}
	if (cellcount == 0 && gnd->cells != NULL) {
		printf("error : should have NULL cells\n");
		ret = EXIT_FAILURE;
	}

	{// test save to data
		unsigned char *data = NULL;
		unsigned long length = 0;
		struct ROGnd *gnd2;
		if (gnd->verminor == 6)
			gnd->verminor = 7; // v1.6 is load-only (data converted to v1.7)
		printf("Save: %d\n", gnd_saveToData(gnd, &data, &length));
		printf("Data: %p %u\n", data, length);
		gnd2 = gnd_loadFromData(data,length);
		if (data == NULL) {
			printf("error : saving produced NULL data\n");
			ret = EXIT_FAILURE;
		}
		else if (gnd2 == NULL) {
			printf("error : saving produced invalid data\n");
			ret = EXIT_FAILURE;
		}
		else {
			printf("Inspect: 0x%x\n", gnd_inspect(gnd2));
			if (!gnd_equal(gnd, gnd2)) {
				printf("error : saving produced different data\n");
				ret = EXIT_FAILURE;
			}
		}
		if (data != NULL)
			get_roint_free_func()(data);
		gnd_unload(gnd2);
	}

	gnd_unload(gnd);
	if (ret == EXIT_SUCCESS)
		printf("OK\n");
	return(ret);
}
