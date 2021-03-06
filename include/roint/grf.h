#ifndef __ROINT_GRF_H
#define __ROINT_GRF_H

#ifdef ROINT_INTERNAL
#	include "config.h"
#elif !defined(WITHOUT_ROINT_CONFIG)
#	include "roint/config.h"
#endif

#ifndef ROINT_DLLAPI
#	define ROINT_DLLAPI
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct ROGrf;
struct ROGrfFile;

typedef void (*t_grf_walk_function_ptr)(const struct ROGrfFile*, void* aux);

ROINT_DLLAPI struct ROGrf *grf_open(const char *fn);
ROINT_DLLAPI void grf_close(struct ROGrf *grf);
ROINT_DLLAPI unsigned int grf_filecount(const struct ROGrf* grf);

ROINT_DLLAPI struct ROGrfFile *grf_getfileinfo(const struct ROGrf* grf, unsigned int idx);
ROINT_DLLAPI struct ROGrfFile *grf_getfileinfobyname(const struct ROGrf* grf, const char* fn);

ROINT_DLLAPI void grf_walk(const struct ROGrf* grf, t_grf_walk_function_ptr fptr, void *aux);

/**
  * Retrieves data from the GRF file and stores in the data pointer.
  * Returns 0 on success.
  */
ROINT_DLLAPI int grf_getdata(struct ROGrfFile *file);
ROINT_DLLAPI void grf_freedata(struct ROGrfFile *file);

#ifdef __cplusplus
}
#endif 

#include <stdio.h> // FILE

#ifdef __cplusplus
extern "C" {
#endif 


struct ROGrfFile {
    char *fileName;
    int compressedLength;
    int compressedLengthAligned;
    int uncompressedLength;
    
    char flags;
    int offset;
    int cycle; // for DES Decoding purposes

	struct ROGrf *grf;

	unsigned char *data;
};

struct ROGrf {
	struct {
	    char signature[16];
		unsigned char allowencryption[14];
	    unsigned int filetableoffset;
		unsigned int number1, number2;
		unsigned int version;
	} header;

	FILE *fp;
	struct ROGrfFile *files;
    struct BTree *btree;
};

#ifdef __cplusplus
}
#endif 

#endif /* __ROINT_GRF_H */
