/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of The Open Ragnarok Project
    Copyright 2007 - 2012 The Open Ragnarok Team
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
#ifndef __AVL_H
#define __AVL_H

// AVL tree structure for improved file search

// Function prototype to compare object at index A with index B, using data from BTree->_internalData
typedef int (*t__btree_compare)(const void*, unsigned int, unsigned int);

struct BTreeNode {
    int left, right;
};
struct BTree {
	int root;
	const void* _internalData;
	t__btree_compare compareFunc;

	struct BTreeNode *nodes;
};

// Returns the depth of the given node
unsigned int __btree_tree_depth(const struct BTree *tree, unsigned int node);

// Returns the balance value of the given node (on balanced AVL trees, the returned value will always be -1, 0 or 1).
int __btree_node_balance(const struct BTree *tree, unsigned int node);

// Rotates right the node at the "depth" index on the "path" vector. Note that the "path" variable should be the path trailed until the last inserted element.
void __btree_rotate_right(struct BTree* tree, unsigned int* path, unsigned int depth);

// Rotates left the node at the "depth" index on the "path" vector. Note that the "path" variable should be the path trailed until the last inserted element.
void __btree_rotate_left(struct BTree* tree, unsigned int* path, unsigned int depth);

// Adds a value on the given tree. The "tree->_internalData" value will be used with the "tree->CompareFunc" to compare the file at "node" index with the elements of the tree. The "path" variable should contains enough free elements based on the number of files included inside the array of filenames.
void __btree_add(struct BTree* tree, unsigned int node, unsigned int *path);


#endif /* __AVL_H */
