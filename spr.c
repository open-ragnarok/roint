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
#include "memloader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct ROSpr *spr_loadFromData(const unsigned char *data, unsigned int length) {
	struct ROSpr *ret;
	struct _memloader *loader;
	unsigned int i;
	char magic[2];
	unsigned int pixels;

	ret = (struct ROSpr*)_xalloc(sizeof(struct ROSpr));
	memset(ret, 0, sizeof(struct ROSpr));
	loader = loader_init(data, length);

	loader_read(&magic, 2, 1, loader);
	if (strncmp("SP", magic, 2) != 0) {
		_xlog("Invalid SPR header: '%c%c'\n", magic[0], magic[1]);
		spr_unload(ret);
		loader_free(loader);
		return(NULL);
	}

	loader_read(&ret->version, 2, 1, loader);
	_xlog("SPR Version: %u.%u\n", (ret->version >> 8) & 0xFF, ret->version & 0xFF);
	switch (ret->version) {
		default:
			_xlog("Unsupported SPR version\n");
			spr_unload(ret);
			loader_free(loader);
			return(NULL);
		case 0x100:
		case 0x101:
		case 0x200:
		case 0x201:
			break;// supported
	}

	loader_read(&ret->palimagecount, 2, 1, loader);
	if (ret->version >= 0x200)
		loader_read(&ret->rgbaimagecount, 2, 1, loader);

	if (ret->palimagecount > 0) {
		ret->palimages = (struct ROSprPalImage*)_xalloc(sizeof(struct ROSprPalImage) * ret->palimagecount);
		memset(ret->palimages, 0, sizeof(struct ROSprPalImage) * ret->palimagecount);
		for (i = 0; i < ret->palimagecount; i++) {
			struct ROSprPalImage *image = &ret->palimages[i];
			loader_read(&image->width, 2, 1, loader);
			loader_read(&image->height, 2, 1, loader);
			pixels = image->width * image->height;
			if (pixels > 0) {
				image->data = (unsigned char*)_xalloc(sizeof(unsigned char) * pixels);
				if (ret->version >= 0x201) {
					unsigned int next = 0;
					unsigned short encoded;
					loader_read(&encoded, 2, 1, loader);
					while (next < pixels && encoded > 0) {
						unsigned char c;
						loader_read(&c, 1, 1, loader);
						encoded--;
						if (c == 0) {// index 0 is rle-encoded (invisible/background palette index)
							unsigned char len;
							loader_read(&len, 1, 1, loader);
							encoded--;
							if (len == 0)
								len = 1;
							if (next + len > pixels) {
								_xlog("too much rle-encoded data (next=%u, len=%u, pixels=%u)\n", next, len, pixels);
								break;// no space
							}
							memset(image->data + next, 0, len);
							next += len;
						}
						else
							image->data[next++] = c;
					}
					if (next != pixels || encoded > 0) {
						_xlog("SPR has bad rle-encoded pal image (index=%u, width=%u, height=%u, pixels left=%u, extra data=%u)\n", i, image->width, image->height, pixels - next, encoded);
						spr_unload(ret);
						loader_free(loader);
						return(NULL);
					}
				}
				else
					loader_read(image->data, 1, pixels, loader);
			}
		}
	}

	if (ret->rgbaimagecount > 0) {
		ret->rgbaimages = (struct ROSprRgbaImage*)_xalloc(sizeof(struct ROSprRgbaImage) * ret->rgbaimagecount);
		memset(ret->rgbaimages, 0, sizeof(struct ROSprRgbaImage) * ret->rgbaimagecount);
		for (i = 0; i < ret->rgbaimagecount; i++) {
			struct ROSprRgbaImage *image = &ret->rgbaimages[i];
			loader_read(&image->width, 2, 1, loader);
			loader_read(&image->height, 2, 1, loader);
			pixels = image->width * image->height;
			if (pixels > 0) {
				image->data = (struct ROSprColor*)_xalloc(sizeof(struct ROSprColor) * pixels);
				loader_read(image->data, sizeof(struct ROSprColor), pixels, loader);
			}
		}
	}

	if (ret->version >= 0x101) {
		unsigned long offset = loader_tell(loader);
		ret->pal = pal_loadFromData(data + offset, length - offset);
	}

	if (loader_error(loader) || ret->pal == NULL) {
		// data was missing
		_xlog("SPR is incomplete or invalid\n");
		spr_unload(ret);
		loader_free(loader);
		return(NULL);
	}
	loader_free(loader);

	return(ret);
}


struct ROSpr *spr_loadFromFile(const char *fn) {
	FILE *fp;
	unsigned char *data;
	long length;
	struct ROSpr *ret;

	fp = fopen(fn, "rb");
	if (fp == NULL) {
		_xlog("Cannot open file %s\n", fn);
		return(NULL);
	}

	fseek(fp, 0, SEEK_END);
	length = ftell(fp);
	if (length == -1) {
		_xlog("%s : %s\n", fn, strerror(errno));
		fclose(fp);
		return(NULL);
	}

	data = (unsigned char*)_xalloc((unsigned int)length);
	fseek(fp, 0, SEEK_SET);
	clearerr(fp);
	fread(data, (unsigned int)length, 1, fp);
	if (ferror(fp)) {
		_xlog("%s : %s\n", fn, strerror(errno));
		_xfree(data);
		fclose(fp);
		return(NULL);
	}

	ret = spr_loadFromData(data, (unsigned int)length);
	_xfree(data);
	fclose(fp);

	return(ret);
}


struct ROSpr *spr_loadFromGrf(struct ROGrfFile *file) {
	struct ROSpr *ret = NULL;
	if (file->data == NULL) {
		grf_getdata(file);
		if (file->data != NULL) {
			ret = spr_loadFromData(file->data, file->uncompressedLength);
		}
		_xfree(file->data);
		file->data = NULL;
	}
	else {
		ret = spr_loadFromData(file->data, file->uncompressedLength);
	}

	return(ret);
}


void spr_unload(struct ROSpr* spr) {
	unsigned int i;

	if (spr == NULL)
		return;

	if (spr->palimages != NULL) {
		for (i = 0; i < spr->palimagecount; i++)
			if (spr->palimages[i].data != NULL)
				_xfree(spr->palimages[i].data);
		_xfree(spr->palimages);
	}

	if (spr->rgbaimages != NULL) {
		for (i = 0; i < spr->rgbaimagecount; i++)
			if (spr->rgbaimages[i].data != NULL)
				_xfree(spr->rgbaimages[i].data);
		_xfree(spr->rgbaimages);
	}

	if (spr->pal != NULL)
		pal_unload(spr->pal);

	_xfree(spr);
}
