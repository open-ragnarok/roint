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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


const char SPR_MAGIC[] = {'S','P'};


unsigned short spr_inspect(const struct ROSpr *spr) {
	unsigned int i;
	unsigned int pixels;

	if (spr == NULL) {
		_xlog("spr.inspect : invalid argument (spr=%p)\n", spr);
		return(0);
	}

	if (spr->pal == NULL) {
		if (spr->rgbaimages > 0) {
			_xlog("spr.inspect : rgba images are not supported (no palette, so only v1.0 is valid)");
			return(0); // invalid, rgba images are not supported
		}
		return(0x100); // v1.0
	}
	for (i = 0; i < spr->palimagecount; i++) {
		const struct ROSprPalImage *image = &spr->palimages[i];
		if (_mul_over_limit(image->width, image->height, 0xFFFFFFFF)) {
			_xlog("spr.inspect : [%u] pal image dimensions are too big (%ux%u)\n", i, image->width, image->height);
			return(0); // invalid, image is too big
		}
		pixels = image->width * image->height;
		if (pixels > 0 && image->data == NULL) {
			_xlog("spr.inspect : [%u] expected non-NULL data in pal image\n", i);
			return(0);
		}
		if (pixels == 0 && image->data != NULL) {
			_xlog("spr.inspect : [%u] expected NULL data in pal image\n", i);
			return(0);
		}
	}
	if (spr->rgbaimages > 0) {
		for (i = 0; i < spr->rgbaimagecount; i++) {
			const struct ROSprRgbaImage *image = &spr->rgbaimages[i];
			if (_mul_over_limit(image->width, image->height, 0xFFFFFFFF)) {
				_xlog("spr.inspect : [%u] rgba image dimensions are too big (%ux%u)\n", i, image->width, image->height);
				return(0); // invalid, image is too big
			}
			pixels = image->width * image->height;
			if (pixels > 0 && image->data == NULL) {
				_xlog("spr.inspect : [%u] expected non-NULL data in rgba image\n", i);
				return(0);
			}
			if (pixels == 0 && image->data != NULL) {
				_xlog("spr.inspect : [%u] expected NULL data in rgba image\n", i);
				return(0);
			}
		}
		return(0x200); // v2.0 or v2.1
	}
	return(0x101); // v1.1 or v2.0 or v2.1
}


struct ROSpr *spr_load(struct _reader *reader) {
	struct ROSpr *ret;
	unsigned int i;
	char magic[2];
	unsigned int pixels;

	if (reader == NULL || reader->error) {
		_xlog("spr.load : invalid argument (reader=%p reader.error=%d)\n", reader, reader->error);
		return(NULL);
	}

	ret = (struct ROSpr*)_xalloc(sizeof(struct ROSpr));
	memset(ret, 0, sizeof(struct ROSpr));

	reader->read(&magic, 2, 1, reader);
	if (memcmp(SPR_MAGIC, magic, 2) != 0) {
		_xlog("spr.load : invalid header x%02X%02X (\"%-2s\")\n", magic[0], magic[1], magic);
		spr_unload(ret);
		return(NULL);
	}

	reader->read(&ret->version, 2, 1, reader);
	//_xlog("SPR Version: %u.%u\n", (ret->version >> 8) & 0xFF, ret->version & 0xFF);
	switch (ret->version) {
		default:
			_xlog("spr.load : unknown version 0x%X (v%u.%u)\n", ret->version, (ret->version >> 8) & 0xFF, ret->version & 0xFF);
			spr_unload(ret);
			return(NULL);
		case 0x100:
		case 0x101:
		case 0x200:
		case 0x201:
			break;// supported
	}

	reader->read(&ret->palimagecount, 2, 1, reader);
	if (ret->version >= 0x200)
		reader->read(&ret->rgbaimagecount, 2, 1, reader);

	if (ret->palimagecount > 0) {
		ret->palimages = (struct ROSprPalImage*)_xalloc(sizeof(struct ROSprPalImage) * ret->palimagecount);
		memset(ret->palimages, 0, sizeof(struct ROSprPalImage) * ret->palimagecount);
		for (i = 0; i < ret->palimagecount; i++) {
			struct ROSprPalImage *image = &ret->palimages[i];
			reader->read(&image->width, 2, 1, reader);
			reader->read(&image->height, 2, 1, reader);
			if (_mul_over_limit(image->width, image->height, 0xFFFFFFFF)) {
				_xlog("spr.load : [%u] pal image too big (width=%u height=%u)\n", i, image->width, image->height);
				spr_unload(ret);
				return(NULL);
			}
			pixels = image->width * image->height;
			if (pixels > 0) {
				image->data = (unsigned char*)_xalloc(sizeof(unsigned char) * pixels);
				if (ret->version >= 0x201) {
					unsigned int next = 0;
					unsigned short encoded;
					reader->read(&encoded, 2, 1, reader);
					while (next < pixels && encoded > 0) {
						unsigned char c;
						reader->read(&c, 1, 1, reader);
						encoded--;
						if (c == 0) {// index 0 is rle-encoded (invisible/background palette index)
							unsigned char len;
							reader->read(&len, 1, 1, reader);
							encoded--;
							if (len == 0)
								len = 1;
							if (next + len > pixels) {
								_xlog("spr.load : [%u] too much encoded data for pal image (next=%u, len=%u, pixels=%u remaining_data=%u)\n", i, next, len, pixels, encoded);
								spr_unload(ret);
								return(NULL);
							}
							memset(image->data + next, 0, len);
							next += len;
						}
						else
							image->data[next++] = c;
					}
					if (next != pixels || encoded > 0) {
						_xlog("spr.load : [%u] bad encoded pal image (width=%u, height=%u, pixels_left=%u, remaining_data=%u)\n", i, image->width, image->height, pixels - next, encoded);
						spr_unload(ret);
						return(NULL);
					}
				}
				else
					reader->read(image->data, 1, pixels, reader);
			}
		}
	}

	if (ret->rgbaimagecount > 0) {
		ret->rgbaimages = (struct ROSprRgbaImage*)_xalloc(sizeof(struct ROSprRgbaImage) * ret->rgbaimagecount);
		memset(ret->rgbaimages, 0, sizeof(struct ROSprRgbaImage) * ret->rgbaimagecount);
		for (i = 0; i < ret->rgbaimagecount; i++) {
			struct ROSprRgbaImage *image = &ret->rgbaimages[i];
			reader->read(&image->width, 2, 1, reader);
			reader->read(&image->height, 2, 1, reader);
			pixels = image->width * image->height;
			if (pixels > 0) {
				image->data = (struct ROSprColor*)_xalloc(sizeof(struct ROSprColor) * pixels);
				reader->read(image->data, sizeof(struct ROSprColor), pixels, reader);
			}
		}
	}

	if (reader->error) {
		_xlog("spr.load : read error\n");
		spr_unload(ret);
		return(NULL);
	}

	if (ret->version >= 0x101) {
		ret->pal = pal_load(reader);
		if (ret->pal == NULL) {
			_xlog("spr.load : failed to read palette\n");
			spr_unload(ret);
			return(NULL);
		}
	}

	return(ret);
}


struct ROSpr *spr_loadFromData(const unsigned char *data, unsigned int length) {
	struct ROSpr *ret;
	struct _reader *reader;

	reader = memreader_init(data, length);
	ret = spr_load(reader);
	reader->destroy(reader);

	return(ret);
}


struct ROSpr *spr_loadFromFile(const char *fn) {
	struct ROSpr *ret;
	struct _reader *reader;

	reader = filereader_init(fn);
	ret = spr_load(reader);
	reader->destroy(reader);

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


int spr_save(const struct ROSpr *spr, struct _writer *writer) {
	unsigned int i,j;
	unsigned int pixels;
	unsigned short minimumver;
	unsigned short encodedlen;
	unsigned char encodeddata[0xFFFF];

	if (spr == NULL || writer == NULL || writer->error) {
		_xlog("spr.save : invalid argument (spr=%p writer=%p writer.error=%d)\n", spr, writer, writer->error);
		return(1);
	}

	minimumver = spr_inspect(spr);
	if (minimumver == 0) {
		_xlog("spr.save : invalid\n");
		return(1);
	}
	switch (spr->version) {
		default:
			_xlog("spr.save : unknown version 0x%X (v%u.%u)\n", spr->version, (spr->version >> 8) & 0xFF, spr->version & 0xFF);
			return(1);
		case 0x100:
		case 0x101:
		case 0x200:
		case 0x201:
			break;// supported
	}
	if (spr->version < minimumver) {
		_xlog("spr.save : incompatible version (must be at least v%u.%u)\n", (minimumver >> 8) & 0xFF, minimumver & 0xFF);
		return(1);
	}

	writer->write(SPR_MAGIC, 2, 1, writer);
	writer->write(&spr->version, 2, 1, writer);
	writer->write(&spr->palimagecount, 2, 1, writer);
	if (spr->version >= 0x200)
		writer->write(&spr->rgbaimagecount, 2, 1, writer);

	for (i = 0; i < spr->palimagecount; i++) {
		const struct ROSprPalImage *image = &spr->palimages[i];
		writer->write(&image->width, 2, 1, writer);
		writer->write(&image->height, 2, 1, writer);
		pixels = image->width * image->height;
		if (spr->version >= 0x201) {
			encodedlen = 0;
			j = 0;
			while (j < pixels) {
				unsigned char c = image->data[j];
				j++;

				if (_add_over_limit(encodedlen, (c == 0)? 2: 1, 0xFFFF)) {
					_xlog("spr.save : [%u] failed to encode pal image data, use version v2.0 instead\n");
					return(1);
				}
				encodeddata[encodedlen] = c;
				encodedlen++;
				if (c == 0) {
					unsigned char len = 1;
					while (j < pixels && len < 255 && image->data[j] == 0) {
						j++;
						len++;
					}
					encodeddata[encodedlen] = len;
					encodedlen++;
				}
			}
			writer->write(&encodedlen, 2, 1, writer);
			if (encodedlen > 0)
				writer->write(encodeddata, 1, encodedlen, writer);
		}
		else if (pixels > 0)
			writer->write(image->data, 1, pixels, writer);
	}

	if (spr->version >= 0x200) {
		for (i = 0; i < spr->rgbaimagecount; i++) {
			const struct ROSprRgbaImage *image = &spr->rgbaimages[i];
			writer->write(&image->width, 2, 1, writer);
			writer->write(&image->height, 2, 1, writer);
			pixels = image->width * image->height;
			if (pixels > 0)
				writer->write(image->data, sizeof(struct ROSprColor), pixels, writer);
		}
	}
	
	if (writer->error) {
		_xlog("spr.save : write error\n");
		return(1);
	}

	if (spr->version >= 0x101)
		return(pal_save(spr->pal, writer));
	return(0);
}


int spr_saveToData(const struct ROSpr *spr, unsigned char **data_out, unsigned long *size_out) {
	int ret;
	struct _writer *writer;

	writer = memwriter_init(data_out, size_out);
	ret = spr_save(spr, writer);
	writer->destroy(writer);

	return(ret);
}


int spr_saveToFile(const struct ROSpr *spr, const char *fn) {
	int ret;
	struct _writer *writer;

	writer = filewriter_init(fn);
	ret = spr_save(spr, writer);
	writer->destroy(writer);

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
