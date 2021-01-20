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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _ALL_IN_ONE
#include "defines.h"
#include "slib.h"
#include "ttree.h"
#endif // _ALL_IN_ONE

#include <ccan/container_of/container_of.h>
#include <ccan/likely/likely.h>
#include <ccan/list/list.h>
#include <ccan/str/str.h>
#include <ccan/tal/tal.h>
#include <ccan/tal/str/str.h>

// init tree
ttree_t *ttreeinit(void)
{
	ttree_t *ptree = talz(NULL, ttree_t);
	if (!ptree)
		return NULL;

	strmap_init(&ptree->node_files);
	strmap_init(&ptree->node_funcs);
	strmap_init(&ptree->branch_exact);
	strmap_init(&ptree->branch_callers);
	strmap_init(&ptree->branch_callees);

	return ptree;
}

void ttreedestroy(ttree_t *ptree)
{
	/* FIXME: Memory leak: Iterate to clear nested map */
	strmap_clear(&ptree->node_files);
	strmap_clear(&ptree->node_funcs);

	/* FIXME: Memory leak: Iterate to clear nested map */
	strmap_clear(&ptree->branch_exact);
	/* FIXME: Memory leak: Iterate to clear nested map */
	strmap_clear(&ptree->branch_callers);
	strmap_clear(&ptree->branch_callees);
	tal_free(ptree);
}

// add a new node (function) to tree
ttreenode_t *ttreeaddnode(ttree_t *ptree, char *funname, char *filename)
{
	ttreenode_t *pnode;
	ttreefile_t *pfile;

	if ((pnode = ttreefindnode(ptree, funname, filename)))
		return pnode;

	pnode = talz(ptree, ttreenode_t);
	if (!pnode) {
		printf("\nMemory allocation error\n");
		return NULL;
	}

	pnode->funname = tal_strdup(pnode, funname);
	if (!pnode->funname)
		goto cleanup_pnode;

	if (filename) {
		pnode->filename = tal_strdup(pnode, filename);
		if (!pnode->filename)
			goto cleanup_pnode;

		pfile = strmap_get(&ptree->node_files, pnode->filename);
		if (!pfile) {
			pfile = talz(ptree, ttreefile_t);
			if (!pfile)
				goto cleanup_pnode;

			strmap_init(&pfile->nodes);
			if (!strmap_add(&ptree->node_files, pnode->filename, pfile))
				goto cleanup_pfile;
		}

		if (!strmap_add(&pfile->nodes, pnode->funname, pnode))
			goto cleanup_pnode;
	}

	if (!strmap_get(&ptree->node_funcs, pnode->funname))
		if (!strmap_add(&ptree->node_funcs, pnode->funname, pnode))
			goto cleanup_pnode;

	if (unlikely(!ptree->firstnode))
		ptree->firstnode = pnode;

	if (likely(ptree->lastnode))
		ptree->lastnode->next = pnode;

	ptree->lastnode = pnode;

	return pnode;

cleanup_pfile:
	tal_free(pfile);

cleanup_pnode:
	tal_free(pnode);

	return NULL;
}

// add a new branch (caller function node to callee function node connection)
int ttreeaddbranch(ttree_t *ptree, ttreenode_t *caller, ttreenode_t *callee,
		   char *filename)
{
	int iErr = 0;
	ttreebranch_t *pbranch;
	ttreeparent_t *parent;
	ttreechild_t *child;

	if (caller == NULL || callee == NULL) {
		printf("\nTrying to make a branch with NULL nodes\n");
		return -1;
	}

	if (ttreefindbranch(ptree, caller, callee, filename, NULL) != NULL)
		return 0;

	// only if branch does not exist yet
	pbranch = talz(ptree, ttreebranch_t);
	if (!pbranch) {
		printf("\nMemory allocation error\n");
		return -1;
	}

	parent = &pbranch->parent;
	child = &pbranch->child;

	pbranch->parent.filename = tal_strdup(pbranch, filename);
	if (!pbranch->parent.filename) {
		iErr = -1;
		goto cleanup_pbranch;
	}

	parent->node = caller;
	child->node = callee;

	if (filename && caller && callee) {
		ttreebranchfile_t *pfile;
		ttreebranchsource_t *psrc;

		pfile = strmap_get(&ptree->branch_exact, parent->filename);
		if (!pfile) {
			pfile = talz(ptree, ttreebranchfile_t);
			if (!pfile)
				goto cleanup_pbranch; /* FIXME */

			strmap_init(&pfile->sources);

			if (!strmap_add(&ptree->branch_exact, parent->filename,
						pfile))
				goto cleanup_pbranch; /* FIXME */
		}

		psrc = strmap_get(&pfile->sources, caller->funname);
		if (!psrc) {
			psrc = talz(pfile, ttreebranchsource_t);
			if (!psrc)
				goto cleanup_pbranch; /* FIXME */

			strmap_init(&psrc->targets);

			if (!strmap_add(&pfile->sources, caller->funname, psrc))
				goto cleanup_pbranch; /* FIXME */
		}

		/* Just add it: we know it's not present as the find() failed */
		if (!strmap_add(&psrc->targets, callee->funname, pbranch))
			goto cleanup_pbranch; /* FIXME */
	}

	if (caller) {
		/*
		 * Illegal filename for key if file is NULL, that way we can
		 * still enter the caller in the branch_callers map
		 */
		const char *key = filename ? parent->filename : "/";
		struct list_head *head = NULL;
		ttreebranchfile_t *pfile;

		pfile = strmap_get(&ptree->branch_callers, key);
		if (!pfile) {
			pfile = talz(ptree, ttreebranchfile_t);
			if (!pfile)
				goto cleanup_pbranch; /* FIXME */

			strmap_init(&pfile->targets);

			if (!strmap_add(&ptree->branch_callers, key, pfile))
				goto cleanup_pbranch; /* FIXME */

		}

		head = strmap_get(&pfile->targets, caller->funname);
		if (!head) {
			head = talz(ptree, struct list_head);
			if (!head)
				goto cleanup_pbranch; /* FIXME */

			list_head_init(head);
			if (!strmap_add(&pfile->targets, caller->funname, head))
				goto cleanup_pbranch; /* FIXME */
		}

		list_add(head, &parent->elem);
	}

	if (callee) {
		struct list_head *head;

		head = strmap_get(&ptree->branch_callees, callee->funname);
		if (!head) {
			head = talz(ptree, struct list_head);
			if (!head)
				goto cleanup_pbranch; /* FIXME */

			list_head_init(head);
			if (!strmap_add(&ptree->branch_callees, callee->funname,
					head))
				goto cleanup_pbranch; /* FIXME */
		}

		list_add(head, &child->elem);
	}

	if (unlikely(!ptree->firstbranch))
		ptree->firstbranch = pbranch;

	if (likely(ptree->lastbranch))
		ptree->lastbranch->next = pbranch;

	ptree->lastbranch = pbranch;

	return iErr;

cleanup_pbranch:
	tal_free(pbranch);

	return iErr;
}

// find a node with specified function name and file name and return its pointer
// or NULL if not found
ttreenode_t *ttreefindnode(ttree_t *ptree, char *funname, char *filename)
{
	ttreenode_t *pnode;
	ttreefile_t *pfile;

	if (!funname)
		return NULL;

	if (filename) {
		pfile = strmap_get(&ptree->node_files, filename);
		if (pfile) {
			pnode = strmap_get(&pfile->nodes, funname);

			if (pnode)
				return pnode;
		}
	} else {
		pnode = strmap_get(&ptree->node_funcs, funname);
		if (pnode)
			return pnode;
	}

	return NULL;
}

// find a branch with specified caller, callee and file name and return its
// pointer or NULL if not found;
// if pstart == NULL search will be performed over all branches, otherwise it
// will start from the specified branch
ttreebranch_t *ttreefindbranch(ttree_t *ptree, ttreenode_t *caller,
			       ttreenode_t *callee, char *filename,
			       ttreebranch_t *pstart)
{
	if (caller == NULL && callee == NULL)
		return NULL;

	if (filename && caller && callee) {
		ttreebranchfile_t *pfile;
		ttreebranchsource_t *psrc;

		assert(!pstart);

		pfile = strmap_get(&ptree->branch_exact, filename);
		if (!pfile)
			return NULL;

		psrc = strmap_get(&pfile->sources, caller->funname);
		if (!psrc)
			return NULL;

		return strmap_get(&psrc->targets, callee->funname);
	}

	if (caller && !callee) {
		ttreebranch_t *pbranch;
		/*
		 * Illegal filename for key if file is NULL, that way we can
		 * still enter the caller in the branch_callers map
		 */
		const char *key = filename ? filename : "/";
		ttreebranchfile_t *pfile;
		struct list_head *head;
		ttreeparent_t *parent;

		pfile = strmap_get(&ptree->branch_callers, key);
		if (!pfile)
			return NULL;

		head = strmap_get(&pfile->targets, caller->funname);
		if (!head)
			return NULL;

		parent = (pstart && pstart == ptree->lbranch) ?
			list_next(head, &pstart->parent, elem) :
			list_top(head, ttreeparent_t, elem);

		if (!parent)
			return NULL;

		pbranch = container_of(parent, ttreebranch_t, parent);

		ptree->lbranch = pbranch;

		return pbranch;
	}

	if (!filename && !caller && callee) {
		struct list_head *head;
		ttreebranch_t *pbranch;
		ttreechild_t *child;

		head = strmap_get(&ptree->branch_callees, callee->funname);
		if (!head)
			return NULL;

		child = (pstart && pstart == ptree->lbranch) ?
			list_next(head, &pstart->child, elem) :
			list_top(head, ttreechild_t, elem);

		if (!child)
			return NULL;

		pbranch = container_of(child, ttreebranch_t, child);

		ptree->lbranch = pbranch;

		return pbranch;
	}

	assert(false && "Unexpected filename/caller/callee combination");
}

// Return the extended name of a node (function name followed by filename) in
// the given string. The size of the string buffer must be given.
// On success 0 is returned, 1 otherwise.
int ttreegetextendednodename(char *sout, int isize, ttreenode_t *pnode)
{
	int ilen, iret;
	// check for some invalid input values
	if (sout == NULL || pnode == NULL || isize <= 0)
		return 1;
	// get the number characters we want to write
	ilen = strlen(pnode->funname);
	ilen += strlen(pnode->filename);
	ilen++;	// the '_' in-between
	// provided buffer size does not fit the resulting string
	if (isize <= ilen)
		goto fail;
	char *sfilename = NULL;
	iret = slibcpy(&sfilename, pnode->filename, 1);
	if (iret != 0)
		goto fail;
	iret = slibreplacechr(sfilename, '.', '_');
	if (iret != 0)
		goto fail;
	iret = snprintf(sout, isize, "%s#%s", pnode->funname, sfilename);
	// if string got truncated, return with error
	if (iret >= isize)
		goto fail;

	free(sfilename);
	return 0;

fail:
	free(sfilename);
	return 1;
}
