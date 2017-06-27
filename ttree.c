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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _ALL_IN_ONE
#include "defines.h"
#include "slib.h"
#include "ttree.h"
#endif // _ALL_IN_ONE

// init tree
void ttreeinit(ttree_t *ptree)
{
	ptree->firstnode = NULL;
	ptree->lastnode = NULL;
	ptree->firstbranch = NULL;
	ptree->lastbranch = NULL;
}

// free tree memory
void ttreefree(ttree_t *ptree)
{
	ttreenode_t *pnode, *pnodenext;
	ttreebranch_t *pbranch, *pbranchnext;

	pnode = ptree->firstnode;
	while (pnode) {
		pnodenext = pnode->next;
		free(pnode);
		pnode = pnodenext;
	}

	pbranch = ptree->firstbranch;
	while (pbranch) {
		pbranchnext = pbranch->next;
		free(pbranch);
		pbranch = pbranchnext;
	}
}

// add a new node (function) to tree
int ttreeaddnode(ttree_t *ptree, char *funname, char *filename)
{
	int iErr = 0;
	ttreenode_t *pnode;

	if (ttreefindnode(ptree, funname, filename) != NULL)
		return 0;

	pnode = calloc(1, sizeof(ttreenode_t));
	if (!pnode) {
		printf("\nMemory allocation error\n");
		return -1;
	}

	pnode->funname = strdup(funname);
	if (!pnode->funname)
		goto cleanup_pnode;

	if (filename) {
		pnode->filename = strdup(filename);
		if (!pnode->filename)
			goto cleanup_funname;
	}

	if (ptree->lastnode) {
		ptree->lastnode->next = pnode;
		ptree->lastnode = pnode;
	} else {
		ptree->firstnode = pnode;
		ptree->lastnode = pnode;
	}

	return 0;

cleanup_funname:
	free(pnode->funname);

cleanup_pnode:
	free(pnode);

	return -1;
}

// add a new branch (caller function node to callee function node connection)
int ttreeaddbranch(ttree_t *ptree, ttreenode_t *caller, ttreenode_t *callee,
		   char *filename)
{
	int iErr = 0;
	ttreebranch_t *pbranch;

	if (caller == NULL || callee == NULL) {
		printf("\nTrying to make a branch with NULL nodes\n");
		return -1;
	}

	if (ttreefindbranch(ptree, caller, callee, filename, NULL) != NULL)
		return 0;

	// only if branch does not exist yet
	pbranch = calloc(1, sizeof(ttreebranch_t));
	if (!pbranch) {
		printf("\nMemory allocation error\n");
		return -1;
	}

	// initialize all branch data bytes to 0
	pbranch->parent = caller;
	pbranch->child = callee;
	pbranch->filename = strdup(filename);
	if (!pbranch->filename) {
		iErr = -1;
		goto cleanup_pbranch;
	}

	if (ptree->lastbranch) {
		ptree->lastbranch->next = pbranch;
		ptree->lastbranch = pbranch;
	} else {
		ptree->firstbranch = pbranch;
		ptree->lastbranch = pbranch;
	}

	return iErr;

cleanup_pbranch:
	free(pbranch);

	return iErr;
}

// find a node with specified function name and file name and return its pointer
// or NULL if not found
ttreenode_t *ttreefindnode(ttree_t *ptree, char *funname, char *filename)
{
	ttreenode_t *pnode;

	if (!funname)
		return NULL;

	pnode = ptree->firstnode;
	while (pnode != NULL) {
		if (strcmp(pnode->funname, funname) == 0 &&
		    (filename == NULL ||
		     strcmp(pnode->filename, filename) == 0))
			break;
		pnode = pnode->next;
	}

	return pnode;
}

// find a branch with specified caller, callee and file name and return its
// pointer or NULL if not found;
// if pstart == NULL search will be performed over all branches, otherwise it
// will start from the specified branch
ttreebranch_t *ttreefindbranch(ttree_t *ptree, ttreenode_t *caller,
			       ttreenode_t *callee, char *filename,
			       ttreebranch_t *pstart)
{
	ttreebranch_t *pbranch;

	if (pstart == NULL)
		pbranch = ptree->firstbranch;
	else
		pbranch = pstart;

	if (caller == NULL && callee == NULL)
		pbranch = NULL;

	while (pbranch != NULL) {
		if ((caller == NULL || pbranch->parent == caller) &&
		    (callee == NULL || pbranch->child == callee) &&
		    (filename == NULL ||
		     strcmp(pbranch->filename, filename) == 0))
			break;
		pbranch = pbranch->next;
	}

	return pbranch;
}
