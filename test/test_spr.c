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
	struct ROSpr *spr;
	unsigned int i;
	int ret;

	if (argc != 2) {
		const char *exe = argv[0];
		printf("Usage:\n  %s sprite.spr\n", exe);
		return(EXIT_FAILURE);
	}

	fn = argv[1];
	
	spr = spr_loadFromFile(fn);
	if (spr == NULL) {
		printf("error : failed to load file '%s'\n", fn);
		return(EXIT_FAILURE);
	}
	ret = EXIT_SUCCESS;
	printf("Version: 0x%x (v%u.%u)\n", spr->version, (spr->version >> 8) & 0xFF, spr->version & 0xFF);
	if (spr->version != 0x100 && spr->version != 0x101 && spr->version != 0x200 && spr->version != 0x201) {
		printf("error : unknown version\n");
		ret = EXIT_FAILURE;
	}
	printf("Pal Images: %u\n", spr->palimagecount);
	for (i = 0; i < spr->palimagecount; i++) {
		struct ROSprPalImage *image = &spr->palimages[i];
		unsigned int pixels = image->width * image->height;
		printf("[%u] %u x %u  %p\n", i, image->width, image->height, image->data);
		if (pixels > 0 && image->data == NULL) {
			printf("error : should have non-NULL data\n");
			ret = EXIT_FAILURE;
		}
		if (pixels == 0 && image->data != NULL) {
			printf("error : should have NULL data\n");
			ret = EXIT_FAILURE;
		}
	}
	printf("Rgba Images: %u\n", spr->rgbaimagecount);
	if (spr->version < 0x200 && spr->rgbaimagecount > 0) {
		printf("error : should not have Rgba Images\n");
		ret = EXIT_FAILURE;
	}
	for (i = 0; i < spr->rgbaimagecount; i++) {
		struct ROSprRgbaImage *image = &spr->rgbaimages[i];
		unsigned int pixels = image->width * image->height;
		printf("[%u] %u x %u  %p\n", i, image->width, image->height, image->data);
		if (pixels > 0 && image->data == NULL) {
			printf("error : should have non-NULL data\n");
			ret = EXIT_FAILURE;
		}
		if (pixels == 0 && image->data != NULL) {
			printf("error : should have NULL data\n");
			ret = EXIT_FAILURE;
		}
	}
	printf("Palette: %p\n", spr->pal);
	if (spr->version >= 0x101 && spr->pal == NULL) {
		printf("error : palette should be non-NULL\n");
		ret = EXIT_FAILURE;
	}
	if (spr->version < 0x101 && spr->pal != NULL) {
		printf("error : palette should be NULL\n");
		ret = EXIT_FAILURE;
	}

	if (ret == EXIT_SUCCESS)
		printf("OK\n");
	return(ret);
}
