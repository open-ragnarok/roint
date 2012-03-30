#ifndef __ROINT_RSW_H
#define __ROINT_RSW_H

#ifdef ROINT_INTERNAL
#	include "config.h"
#elif !defined(WITHOUT_ROINT_CONFIG)
#	include "roint/config.h"
#endif

#ifndef ROINT_DLLAPI
#	define ROINT_DLLAPI
#endif
struct ROGrfFile; // forward declaration

#ifdef __cplusplus
extern "C" {
#endif

#define RORSW_OBJECT_MODEL 1
#define RORSW_OBJECT_LIGHT 2
#define RORSW_OBJECT_SOUND 3
#define RORSW_OBJECT_EFFECT 4

struct RORswQuadTreeNode {
	float max[3];
	float min[3];
	float halfSize[3];
	float center[3];
	int child[4]; //< index of child node (generated), 0 for no child
};
    
struct s_RORswObjModel {
    char name[40];
    int animType;
    float animSpeed;
    int blockType;
    char modelName[80]; //< RSM filename
    char nodeName[80];
    float pos[3];
    float rot[3];
    float scale[3];
};

struct s_RORswObjLight {
    char name[80];
    float pos[3];
    union {
        struct {
            int red;
            int green;
            int blue;
        };
        int color[3];
    };
    float range;
};
    
struct s_RORswObjSound {
    char name[80];
    char waveName[80];
    float pos[3];
    float vol;
    int width;
    int height;
    float range;
    float cycle;		
};
    
struct s_RORswObjEffect {
   char name[80];
   float pos[3];
   int type;
   float emitSpeed;
   float param[4];
};

struct RORswObject {
	int type;
	union {
		struct s_RORswObjModel model;
        struct s_RORswObjLight light;
        struct s_RORswObjSound sound;
		struct s_RORswObjEffect effect;
	};
};

struct RORsw {
	/*
		Unnamed union to represent the file version. Note that the BYTE order on the file is MAJOR.MINOR.
		Here is represented in reverse so that the unsigned short variable "version" is represented
		correcly on little-endian representation.
	*/
	union {
		struct {
			unsigned char verminor; //< minor version
			unsigned char vermajor; //< major version
		};
		unsigned short version;
	};

	char m_iniFile[40];
	char m_gndFile[40];
	char m_gatFile[40];
	char m_scrFile[40];

	struct s_water {
		float level;
		int type;
		float waveHeight;
		float waveSpeed;
		float wavePitch;
		int animSpeed;
	} water;

	struct s_light {
		int longitude; //< degrees
		int latitude; //< degrees
		float diffuse[3]; //< color
		float ambient[3]; //< color
		float ignored; //< ignored, float?
	} light;

	struct s_ground {
		union {
			struct {
				int top;
				int bottom;
				int left;
				int right;
			};
			int gnd[4];
		};
	} ground;

	int obj_count;
	struct RORswObject *objects;
	struct RORswQuadTreeNode *quadtree;
};

ROINT_DLLAPI struct RORsw *rsw_loadFromData(const unsigned char *data, unsigned long length);
ROINT_DLLAPI struct RORsw *rsw_loadFromFile(const char *fn);
ROINT_DLLAPI struct RORsw *rsw_loadFromGrf(struct ROGrfFile *file);
ROINT_DLLAPI void rsw_unload(struct RORsw *rsw);

#ifdef __cplusplus
}
#endif

#endif /* __ROINT_RSW_H */
