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
#ifndef __ROINT_RSM_H
#define __ROINT_RSM_H

#ifndef WITHOUT_ROINT_CONFIG
#	include "roint/config.h"
#endif
struct ROGrfFile; // forward declaration

#ifdef __cplusplus
extern "C" {
#endif 

struct RORsmVertex {
	union {
		float v[3];
		struct { float x, y, z; };
	};
};

struct RORsmTexture {
	unsigned int color;
	float u, v;
};

struct RORsmFace {
	unsigned short vertidx[3];	// Indexes for the three triangle vertices
	unsigned short tvertidx[3]; // Indexes for the texture coordinate vertices
	unsigned short texid;		// The index of the texture to use
	unsigned short __padding;
	int twoSide;
	int smoothGroup;
};

struct RORsmPosKeyframe {
	int frame;
	float px, py, pz;
};

struct RORsmRotKeyframe {
	int frame;
	union {
		float q[4];
		struct { float qx, qy, qz, qw; };	// TODO: not sure about the order of the quaternion elements
	};
};

struct RORsmNode {
	char name[40];
	char parent[40];
	int texture_count;
	int *textures;

	float offsetMT[9]; // 3x3 matrix that identifies the axis and origin of this node
	struct RORsmVertex pos_; // Why in the world do we have 2 positions!?
	struct RORsmVertex pos;
	float rot_angle;
	struct RORsmVertex rot_axis;
	struct RORsmVertex scale;
	// Vertices -- All the vertices used in this node
	int vertice_count;
	struct RORsmVertex *vertices;
	// Texture vertices -- All the texture vertices used in this node
	int texv_count;
	struct RORsmTexture *texv;
	// Faces
	int face_count;
	struct RORsmFace *faces;
	// Position keyframes
	int poskey_count;
	struct RORsmPosKeyframe *poskeys;
	// Rotation keyframes
	int rotkey_count;
	struct RORsmRotKeyframe *rotkeys;
};

struct RORsm {
	// Common header
	char magic[4];
	union {
		unsigned short version;
		struct {
			unsigned char major;
			unsigned char minor;
		} v;
	};
	// RSM Stuff
	int anim_length;
	int shade_type;
	unsigned char alpha;
	unsigned char reserved[16];

	int texture_count;
	char **textures;

	char main_node[40];
	int node_count;

	struct RORsmNode *nodes;
};
    
// Loads the rsm from a given memory pointer using up to the given length
ROINT_DLLAPI struct RORsm *rsm_load(const unsigned char *data, unsigned int len);
// Loads the rsm from the ROGrf structure file. -- This is only a wrapper to the rsm_load() function
ROINT_DLLAPI struct RORsm *rsm_loadFromGrf(struct ROGrfFile*);
// Frees everything inside the RORsm structure allocated by us (including the rsm itself!)
ROINT_DLLAPI void rsm_unload(struct RORsm*);

#ifdef __cplusplus
}
#endif 

#endif /* __ROINT_RSM_H */
