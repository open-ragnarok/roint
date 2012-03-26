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
#include "avl.h"

#include <stdio.h>

unsigned int __btree_tree_depth(const struct BTree *tree, unsigned int node) {
	unsigned int ret, a;

	ret = 1;

	if (tree->nodes[node].left != -1) {
		a = __btree_tree_depth(tree, tree->nodes[node].left) + 1;
		if (ret < a)
			ret = a;
	}

	if (tree->nodes[node].right != -1) {
		a = __btree_tree_depth(tree, tree->nodes[node].right) + 1;
		if (ret < a)
			ret = a;
	}

	return(ret);
}

int __btree_node_balance(const struct BTree *tree, unsigned int node) {
	int ret, r, l;

	r = 0;
	l = 0;
	
	if (tree->nodes[node].left != -1) {
		l = __btree_tree_depth(tree, tree->nodes[node].left);
	}

	if (tree->nodes[node].right != -1) {
		r = __btree_tree_depth(tree, tree->nodes[node].right);
	}

	ret = r - l;

	return(ret);
}

void __btree_rotate_right(struct BTree* tree, unsigned int* path, unsigned int depth) {
	struct BTreeNode *oldroot;
	struct BTreeNode *newroot;
	struct BTreeNode *parent;

	int oldroot_id;
	int newroot_id;

	//printf("Rotate Right\n");

	// Setup stuff
	parent = NULL;
	if (depth > 0) {
		parent = &tree->nodes[path[depth-1]];
	}

	oldroot_id = path[depth];
	newroot_id = tree->nodes[path[depth]].left;

	oldroot = &tree->nodes[path[depth]];
	newroot = &tree->nodes[oldroot->left];

	// Change stuff
	oldroot->left = newroot->right;
	newroot->right = oldroot_id;

	// Fix root
	if (parent != NULL) {
		if (parent->right == oldroot_id)
			parent->right = newroot_id;
		else if (parent->left == oldroot_id)
			parent->left = newroot_id;
		else
			printf("ERROR: the path is WRONG!");
	}
	else {
		tree->root = newroot_id;
	}

	// Fix Path
	path[depth] = newroot_id;
}

void __btree_rotate_left(struct BTree* tree, unsigned int* path, unsigned int depth) {
	struct BTreeNode *oldroot;
	struct BTreeNode *newroot;
	struct BTreeNode *parent;

	int oldroot_id;
	int newroot_id;

	//printf("Rotate Left\n");

	// Setup stuff
	parent = NULL;
	if (depth > 0) {
		parent = &tree->nodes[path[depth-1]];
	}

	oldroot_id = path[depth];
	newroot_id = tree->nodes[path[depth]].right;

	oldroot = &tree->nodes[path[depth]];
	newroot = &tree->nodes[oldroot->right];

	// Change stuff
	oldroot->right = newroot->left;
	newroot->left = oldroot_id;

	// Fix root
	if (parent != NULL) {
		if (parent->right == oldroot_id)
			parent->right = newroot_id;
		else if (parent->left == oldroot_id)
			parent->left = newroot_id;
		else
			printf("ERROR: the path is WRONG!");
	}
	else {
		tree->root = newroot_id;
	}

	// Fix Path
	path[depth] = newroot_id;
}


void __btree_add(struct BTree* tree, unsigned int node, unsigned int *path) {
	unsigned int j, k;
	int i;

	// Start with root
	k = tree->root;
	j = 0;
	while (1) {
		path[j++] = k;
		if (tree->compareFunc(tree->_internalData, node, k) < 0) {	// We go left
			if (tree->nodes[k].left == -1) {
				tree->nodes[k].left = node;
				break;
			}

			k = tree->nodes[k].left;
		}
		else {						// We go right
			if (tree->nodes[k].right == -1) {
				tree->nodes[k].right = node;
				break;
			}

			k = tree->nodes[k].right;
		}
	}

	//print_tree(tree, nodes);
		
	// AVL Balance
	for (i = j-1; i >= 0; i--) {
		int balance, childbalance;

		balance = __btree_node_balance(tree, path[i]);
		if (balance == +2) {				// Right-Right or Right-Left
			childbalance = __btree_node_balance(tree, tree->nodes[path[i]].right);
			if (childbalance == -1) {		// Right Rotation of right child.
				path[i+1] = tree->nodes[path[i]].right;
				__btree_rotate_right(tree, path, i+1);
			}
			__btree_rotate_left(tree, path, i);
			break;
		}
		else if (balance == -2) {			// Left-Left or Left Right
			childbalance = __btree_node_balance(tree, tree->nodes[path[i]].left);
			if (childbalance == +1) {		// Left Rotation of left child.
				path[i+1] = tree->nodes[path[i]].left;
				__btree_rotate_left(tree, path, i+1);
			}
			__btree_rotate_right(tree, path, i);
			break;
		}
	}

	//print_tree(tree, nodes);
}

int __btree_find(struct BTree* tree, const void *what) {
    int k = tree->root;
    int r;
    
    while (k != -1) {
		r = tree->findFunc(tree->_internalData, k, what);
        
        if (r == 0) {
            return(k);
        }
        else if (r > 0) {
            k = tree->nodes[k].left;
        }
        else {
            k = tree->nodes[k].right;
        }
    }
    
    return(-1);
}
