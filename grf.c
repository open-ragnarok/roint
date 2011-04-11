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
#include "grf.h"
#include "des.h"

// Using this file in something that is NOT Open-Ragnarok?
// Don't worry. If you don't have OPENRO_INTERNAL defined, this file will automagically use the standard C malloc() and free() functions. You'll only need grf.{c,h} and des.{c.h} files.
#ifdef OPENRO_INTERNAL
#	include "internal.h"
#else
#	define _xalloc malloc
#	define _xfree free
#endif

#include <zlib.h>
#include <stdlib.h>
#include <string.h>

unsigned int grf_filecount(const struct ROGrf* grf) {
	unsigned int ret;

	if (grf == NULL)
		return(0);

	ret = grf->header.number2 - grf->header.number1 - 7;

	return(ret);
}

struct ROGrf *grf_open(const char *fn) {
	unsigned int compressedLength, uncompressedLength;
	FILE *fp;
	struct ROGrf *ret;
	unsigned char *headerCompressedBody, *headerBody;
	unsigned int i, offset;
	unsigned long ul;
	unsigned int filecount;

	fp = fopen(fn, "rb");
	if (fp == NULL) {
		fprintf(stderr, "Cannot open file %s\n", fn);
		return(NULL);
	}

	ret = (struct ROGrf*)_xalloc(sizeof(struct ROGrf));

	ret->fp = fp;

	// Read header
	fread(ret->header.signature, 16, 1, ret->fp);
	fread(ret->header.allowencryption, 14, 1, ret->fp);
	fread(&ret->header.filetableoffset, sizeof(unsigned int), 1, ret->fp);
	fread(&ret->header.number1, sizeof(unsigned int), 1, ret->fp);
	fread(&ret->header.number2, sizeof(unsigned int), 1, ret->fp);
	fread(&ret->header.version, sizeof(unsigned int), 1, ret->fp);

	// Start to read the index
	fseek(fp, ret->header.filetableoffset, SEEK_CUR);

	// File table header
	fread(&compressedLength, sizeof(unsigned int), 1, ret->fp);
	fread(&uncompressedLength, sizeof(unsigned int), 1, ret->fp);
	ul = uncompressedLength;

	headerCompressedBody = (unsigned char*)_xalloc(compressedLength);
	headerBody = (unsigned char*)_xalloc(uncompressedLength);

	fread(headerCompressedBody, compressedLength, 1, ret->fp);
	uncompress(headerBody, &ul, headerCompressedBody, compressedLength);
	
	if (ul == 0) {
		fprintf(stderr, "Cannot uncompress FileTableHeader\n");
		grf_close(ret);
		return(NULL);
	}

	_xfree(headerCompressedBody);
	
	// Alloc file array
	filecount = grf_filecount(ret);
	ret->files = (struct ROGrfFile*)_xalloc(sizeof(struct ROGrfFile) * filecount);
	memset(ret->files, 0, sizeof(struct ROGrfFile) * filecount);
	
	// Load files from array...
	offset = 0;
	for (i = 0; i < filecount; i++) {
		char c;
		unsigned int pos;
		char aux[512];

		pos = 0;
		memset(aux, 0, 512);

		c = (char)headerBody[offset++];
		while (c != 0) {
			aux[pos++] = c;
			c = (char)headerBody[offset++];
		}

		// Use string as-is
		ret->files[i].fileName = (char*)_xalloc(sizeof(char) * (pos + 1));
		strcpy(ret->files[i].fileName, aux);

		// Load the rest of the file information
		memcpy(&ret->files[i].compressedLength, headerBody+offset, sizeof(int));
		offset += sizeof(int);
		memcpy(&ret->files[i].compressedLengthAligned, headerBody+offset, sizeof(int));
		offset += sizeof(int);
		memcpy(&ret->files[i].uncompressedLength, headerBody+offset, sizeof(int));
		offset += sizeof(int);
		memcpy(&ret->files[i].flags, headerBody+offset, sizeof(char));
		offset += sizeof(char);
		memcpy(&ret->files[i].offset, headerBody+offset, sizeof(int));
		offset += sizeof(int);

		// Setup cycle for des decrypting purposes
		if (ret->files[i].flags == 3) {
			int lop;
			int srccount;
			int srclen = ret->files[i].compressedLength;
			for (lop = 10, srccount = 1; srclen >= lop; lop = lop * 10, srccount++);
			ret->files[i].cycle = srccount;
		}

		// Setup GRF pointer
		ret->files[i].grf = ret;
	}

	_xfree(headerBody);

	return(ret);
}

void grf_close(struct ROGrf *grf) {
	unsigned int i;

	if (grf == NULL)
		return;

	if (grf->files != NULL) {
		for (i = 0; i < grf_filecount(grf); i++) {
			if (grf->files[i].fileName != NULL)
				_xfree(grf->files[i].fileName);
			if (grf->files[i].data != NULL)
				_xfree(grf->files[i].data);
		}
		_xfree(grf->files);
	}

	if (grf->fp != NULL)
		fclose(grf->fp);

	_xfree(grf);
}

int grf_getdata(struct ROGrfFile *file) {
	unsigned char *body;
	unsigned char *uncompressed;
	unsigned long uncompressedLength;
	int r;

	if (file == NULL)
		return(1);

	if (file->grf == NULL)
		return(1);

	if (file->data != NULL) {
		_xfree(file->data);
		file->data = NULL;
	}

	body = _xalloc(file->compressedLength);
	uncompressed = _xalloc(file->uncompressedLength);

	fseek(file->grf->fp, 46 + file->offset, SEEK_SET);
	fread(body, file->compressedLengthAligned, 1, file->grf->fp);

	if ((file->flags == 3) || (file->flags == 5)) {
		// Decode DES
		des_decode(body, file->compressedLengthAligned, file->cycle);
	}

	uncompressedLength = file->uncompressedLength;

	r = uncompress(uncompressed, &uncompressedLength, body, file->compressedLengthAligned);
	if (r != Z_OK) {
		_xfree(body);
		_xfree(uncompressed);

		switch(r) {
			case Z_MEM_ERROR:
				fprintf(stderr, "Error uncompressing data Z_MEM_ERROR");
				break;
			case Z_BUF_ERROR:
				fprintf(stderr, "Error uncompressing data Z_BUF_ERROR");
				break;
			case Z_STREAM_ERROR:
				fprintf(stderr, "Error uncompressing data Z_STREAM_ERROR");
				break;
			case Z_DATA_ERROR:
				fprintf(stderr, "Error uncompressing data Z_DATA_ERROR");
				break;
			default:
				fprintf(stderr, "Unknown error when uncompressing data: %d", r);
		}
		return(1);
	}

	file->data = uncompressed;
	_xfree(body);

	return(0);
}

void grf_freedata(struct ROGrfFile *file) {
	if (file->data != NULL)
		_xfree(file->data);

	file->data = NULL;
}