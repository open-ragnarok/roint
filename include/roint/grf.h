#ifndef __ROINT_GRF_H
#define __ROINT_GRF_H

#ifdef __cplusplus
extern "C" {
#endif

struct ROGrf;
struct ROGrfFile;
    
struct ROGrf *grf_open(const char *fn);
void grf_close(struct ROGrf *grf);
unsigned int grf_filecount(const struct ROGrf* grf);
    
/**
  * Retrieves data from the GRF file and stores in the data pointer.
  * Returns 0 on success.
  */
int grf_getdata(struct ROGrfFile *file);
void grf_freedata(struct ROGrfFile *file);

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
};

#ifdef __cplusplus
}
#endif 

#endif /* __ROINT_GRF_H */
