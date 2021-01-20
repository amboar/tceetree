/*
 * This source code is released for free distribution under the terms of the MIT
 * License (MIT):
 *
 * Copyright (c) 2014, Fabio Visona'
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef _TTREE_H
#define _TTREE_H

#include <ccan/list/list.h>
#include <ccan/strmap/strmap.h>

typedef struct ttreenode_st *ttreenode_tp;
typedef struct ttreebranch_st *ttreebranch_tp;

typedef struct ttreenode_st {
	char *funname;      // function name
	char *filename;     // filename where the function definition is
	int outdone;	// = 1 when node output is done
	int subtreeoutdone; // = 1 when node subtree output is done
	int isroot;	 // = 1 when node is one of the roots
	int icolor;	 // color for node (0 = default)
	ttreenode_tp next;  // Next node for linear list access
} ttreenode_t;

typedef struct ttreeparent_st {
	char *filename;      // filename where the call is
	ttreenode_t *node; // parent node (calling function)
	struct list_node elem;
} ttreeparent_t;

typedef struct ttreechild_st {
	ttreenode_t *node;  // child node (called function)
	struct list_node elem;
} ttreechild_t;

typedef struct ttreebranch_st {
	ttreeparent_t parent;
	ttreechild_t child;
	int outdone;	 // = 1 when branch output is done
	int icolor;	  // color for branch (0 = default)
	ttreebranch_tp next; // Next branch for linear list access
} ttreebranch_t;

typedef struct ttreebranchsource_st {
	STRMAP(ttreebranch_t *) targets;
} ttreebranchsource_t;

typedef struct ttreebranchfile_st {
	union {
		STRMAP(ttreebranchsource_t *) sources;
		STRMAP(struct list_head *) targets;
	};
} ttreebranchfile_t;

typedef struct ttreefile_st {
	STRMAP(ttreenode_t *) nodes;
} ttreefile_t;

typedef STRMAP(ttreefile_t *) strmap_treefile_p;
typedef STRMAP(ttreebranchfile_t *) strmap_treebranchfile_p;

typedef struct ttree_st {
	ttreenode_t *firstnode;     // first node of linear list
	ttreenode_t *lastnode;
	STRMAP(ttreefile_t *) node_files;
	STRMAP(ttreenode_t *) node_funcs;

	ttreebranch_t *firstbranch; // first branch of linear list
	ttreebranch_t *lastbranch;
	STRMAP(ttreebranchfile_t *) branch_exact;
	STRMAP(ttreebranchfile_t *) branch_callers;
	STRMAP(struct list_head *) branch_callees;
	ttreebranch_t *lbranch;
} ttree_t;

ttree_t *ttreeinit(void);
void ttreedestroy(ttree_t *);
ttreenode_t *ttreeaddnode(ttree_t *ptree, char *funname, char *filename);
int ttreeaddbranch(ttree_t *ptree, ttreenode_t *caller, ttreenode_t *callee,
		   char *filename);
ttreenode_t *ttreefindnode(ttree_t *ptree, char *funname, char *filename);
ttreebranch_t *ttreefindbranch(ttree_t *ptree, ttreenode_t *caller,
			       ttreenode_t *callee, char *filename,
			       ttreebranch_t *pstart);
int ttreegetextendednodename(char *sout, int isize, ttreenode_t *pnode);

#endif // #ifndef _TTREE_H
