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


int spr_equal(struct ROSpr *spr, struct ROSpr *spr2) {
	unsigned int i;

	if (spr2->version != spr->version ||
		spr2->palimagecount != spr->palimagecount ||
		spr2->rgbaimagecount != spr->rgbaimagecount)
		return(0);
	for (i = 0; i < spr->palimagecount; i++) {
		struct ROSprPalImage *image = &spr->palimages[i];
		struct ROSprPalImage *image2 = &spr->palimages[i];
		if (image2->width != image->width ||
			image2->height != image->height ||
			memcmp(image2->data, image->data, sizeof(unsigned char) * image->width * image->height) != 0)
			return(0);
	}
	for (i = 0; i < spr->rgbaimagecount; i++) {
		struct ROSprRgbaImage *image = &spr->rgbaimages[i];
		struct ROSprRgbaImage *image2 = &spr->rgbaimages[i];
		if (image2->width != image->width ||
			image2->height != image->height ||
			memcmp(image2->data, image->data, sizeof(struct ROSprColor) * image->width * image->height) != 0)
			return(0);
	}
	if (spr2->pal == NULL && spr->pal == NULL)
		return(1);
	if (spr2->pal == NULL ||
		spr->pal == NULL ||
		memcmp(spr2->pal, spr->pal, sizeof(struct ROPal)) != 0)
		return(0);
	return(1);
}


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
	
	ret = EXIT_SUCCESS;
	spr = spr_loadFromFile(fn);
	if (spr == NULL) {
		printf("error : failed to load file '%s'\n", fn);
		return(EXIT_FAILURE);
	}
	printf("Inspect: 0x%x\n", spr_inspect(spr));
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

	{// test save to data
		unsigned char *data = NULL;
		unsigned long length = 0;
		struct ROSpr *spr2;
		printf("Save: %d\n", spr_saveToData(spr, &data, &length));
		printf("Data: %p %u\n", data, length);
		spr2 = spr_loadFromData(data,length);
		if (data == NULL) {
			printf("error : saving produced NULL data\n");
			ret = EXIT_FAILURE;
		}
		else if (spr2 == NULL) {
			printf("error : saving produced invalid data\n");
			ret = EXIT_FAILURE;
		}
		else {
			printf("Inspect: 0x%x\n", spr_inspect(spr2));
			if (!spr_equal(spr, spr2)) {
				printf("error : saving produced different data\n");
				ret = EXIT_FAILURE;
			}
		}
		if (data != NULL)
			get_roint_free_func()(data);
		spr_unload(spr2);
	}

	if (ret == EXIT_SUCCESS)
		printf("OK\n");
	return(ret);
}
