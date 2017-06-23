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

typedef struct ttreebranch_st {
	char *filename;      // filename where the call is
	ttreenode_t *parent; // parent node (calling function)
	ttreenode_t *child;  // child node (called function)
	int outdone;	 // = 1 when branch output is done
	int icolor;	  // color for branch (0 = default)
	ttreebranch_tp next; // Next branch for linear list access
} ttreebranch_t;

typedef struct ttree_st {
	ttreenode_t *firstnode;     // first node of linear list
	ttreenode_t *lastnode;      // last node of linear list
	ttreebranch_t *firstbranch; // first branch of linear list
	ttreebranch_t *lastbranch;  // last branch of linear list
} ttree_t;

void ttreeinit(ttree_t *ptree);
void ttreefree(ttree_t *ptree);
int ttreeaddnode(ttree_t *ptree, char *funname, char *filename);
int ttreeaddbranch(ttree_t *ptree, ttreenode_t *caller, ttreenode_t *callee,
		   char *filename);
ttreenode_t *ttreefindnode(ttree_t *ptree, char *funname, char *filename);
ttreebranch_t *ttreefindbranch(ttree_t *ptree, ttreenode_t *caller,
			       ttreenode_t *callee, char *filename,
			       ttreebranch_t *pstart);

#endif // #ifndef _TTREE_H
