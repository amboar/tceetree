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
#include <string.h>

#ifndef _ALL_IN_ONE
#include "defines.h"
#include "outgraphviz.h"
#include "outtree.h"
#endif // _ALL_IN_ONE

#define ROOTMARK                                                               \
	0x100 // special value used to find the path to be highlighted (-p
	      // option)

// start output
int outopen(ttree_t *ptree, treeparam_t *pparam)
{
	int iErr = 0;

	switch (pparam->outtype) {
	case TREEOUT_GRAPHVIZ:
		iErr = outopen_gra(ptree, pparam);
		break;

	default:
		iErr = -1;
		break;
	}

	return iErr;
}

// end output
int outclose(ttree_t *ptree, treeparam_t *pparam)
{
	int iErr = 0;

	switch (pparam->outtype) {
	case TREEOUT_GRAPHVIZ:
		iErr = outclose_gra(ptree, pparam);
		break;

	default:
		iErr = -1;
		break;
	}

	return iErr;
}

// output one node
int outnode(ttreenode_t *pnode, treeparam_t *pparam, int colr)
{
	int iErr = 0;

	if (pnode && pnode->outdone)
		return iErr;

	if (colr >= 0) {
		if (colr == ROOTMARK || pnode->icolor == ROOTMARK) {
			// assign color only while "rootmarking" or if node was
			// previously
			// "rootmarked"
			pnode->icolor = colr;
		}
	} else {
		if (pnode->icolor == ROOTMARK)
			pnode->icolor =
			    0; // Only "rootmarked", use default color/style

		// output node
		switch (pparam->outtype) {
		case TREEOUT_GRAPHVIZ:
			iErr = outnode_gra(pnode, pparam);
			break;

		default:
			iErr = -1;
			break;
		}
	}

	pnode->outdone = 1;

	return iErr;
}

// output one branch
int outbranch(ttreebranch_t *pbranch, treeparam_t *pparam, int colr)
{
	int iErr = 0;
	int i;

	if (pbranch && pbranch->outdone)
		return iErr;

	// check if branch should be excluded
	for (i = 0; i < pparam->excludfno; i++) {
		// check if library excluded
		if (strcmp(pparam->excludf[i], TT_LIBRARY) == 0 &&
		    pbranch->child->filename == NULL)
			return iErr;

		// check if caller or callee function name matches one of those
		// in the
		// exclusion list
		if (strcmp(pbranch->parent->funname, pparam->excludf[i]) == 0 ||
		    strcmp(pbranch->child->funname, pparam->excludf[i]) == 0)
			return iErr;
	}

	if (colr >= 0) {
		if (colr == ROOTMARK || pbranch->icolor == ROOTMARK) {
			// assign color only while "rootmarking" or if branch
			// was previously
			// "rootmarked"
			pbranch->icolor = colr;
		}
	} else {
		if (pbranch->icolor == ROOTMARK)
			pbranch->icolor =
			    0; // Only "rootmarked", use default color/style

		// output branch
		switch (pparam->outtype) {
		case TREEOUT_GRAPHVIZ:
			iErr = outbranch_gra(pbranch, pparam);
			break;

		default:
			iErr = -1;
			break;
		}
	}

	pbranch->outdone = 1;

	return iErr;
}

// output of a subtree (forward and backward) starting from pnode
int outsubtree(ttree_t *ptree, treeparam_t *pparam, ttreenode_t *pnode,
	       int fdepth, int bdepth, int colr)
{
	int iErr = 0;
	ttreebranch_t *pbranch;
	int i, prevcol;

	if (pnode == NULL)
		return iErr;

	if (pnode->subtreeoutdone)
		return iErr;

	for (i = 0; i < pparam->excludfno; i++) {
		// check if library excluded
		if (strcmp(pparam->excludf[i], TT_LIBRARY) == 0 &&
		    pnode->filename == NULL)
			return iErr;

		// check if node function name matches one of those in the
		// exclusion list
		if (strcmp(pnode->funname, pparam->excludf[i]) == 0)
			return iErr;
	}

	// before scanning subtree, output the node itself
	iErr = outnode(pnode, pparam, colr);

	if (colr > 0) {
		if (colr == ROOTMARK) {
			// if "rootmarking" (colr == ROOTMARK) and the last
			// function of the path
			// to be highlighted from roots has been found, then
			// stop scanning tree
			if (pparam->callp)
				if (strcmp(pnode->funname, pparam->callp) == 0)
					return iErr;
		} else {
			// if coloring and this is a root, then stop scanning
			// tree
			if (pnode->isroot)
				return iErr;
		}
	}

	if (iErr == 0 && fdepth != 0) {
		// forward (children) scanning
		if (fdepth > 0)
			fdepth--;

		pbranch = NULL;
		do {
			// find all branches starting from this node
			pbranch = ttreefindbranch(ptree, pnode, NULL,
						  pnode->filename, pbranch);

			if (pbranch) {
				if (!pbranch->outdone) {
					// if branch not done
					prevcol = pbranch->icolor;
					// output branch
					iErr = outbranch(pbranch, pparam, colr);
					if (iErr != 0)
						break;

					// do subtree
					if (pbranch->child !=
					    pnode) // avoid involving recursion
						   // in depth decrease
						if (colr <= 0 ||
						    colr == ROOTMARK ||
						    prevcol == ROOTMARK)
							iErr = outsubtree(
							    ptree, pparam,
							    pbranch->child,
							    fdepth, 0, colr);
				}

				pbranch = pbranch->next;
			}
		} while (pbranch != NULL);
	}

	if (iErr == 0 && bdepth != 0) {
		if (bdepth > 0)
			bdepth--;

		pbranch = NULL;
		do {
			// find all branches with this node as destination
			pbranch =
			    ttreefindbranch(ptree, NULL, pnode, NULL, pbranch);

			if (pbranch) {
				if (!pbranch->outdone) {
					// if branch not done
					prevcol = pbranch->icolor;
					// output branch
					iErr = outbranch(pbranch, pparam, colr);
					if (iErr != 0)
						break;

					// do subtree
					if (pbranch->parent !=
					    pnode) // avoid involving recursion
						   // in depth decrease
						if (colr <= 0 ||
						    colr == ROOTMARK ||
						    prevcol == ROOTMARK)
							iErr = outsubtree(
							    ptree, pparam,
							    pbranch->parent, 0,
							    bdepth, colr);
				}

				pbranch = pbranch->next;
			}
		} while (pbranch != NULL);
	}

	pnode->subtreeoutdone = 1;

	return iErr;
}

// tree output init
void outtreeinit(ttree_t *ptree, int color, int resetroot)
{
	ttreenode_t *pnode;
	ttreebranch_t *pbranch;

	pnode = ptree->firstnode;
	while (pnode != NULL) {
		pnode->outdone = 0;
		pnode->subtreeoutdone = 0;
		if (resetroot)
			pnode->isroot = 0;
		if (color >= 0)
			pnode->icolor = color;
		pnode = pnode->next;
	}

	pbranch = ptree->firstbranch;
	while (pbranch != NULL) {
		pbranch->outdone = 0;
		if (color >= 0)
			pbranch->icolor = color;
		pbranch = pbranch->next;
	}
}

// make tree output
int outtree(ttree_t *ptree, treeparam_t *pparam)
{
	int iErr = 0, iErrC;
	ttreenode_t *pnode;
	int i = 0;

	if (pparam->verbose)
		printf("\nMaking output... ");

	// start output
	iErr = outopen(ptree, pparam);
	if (iErr == 0) {
		// init, color = 0, reset root flags
		outtreeinit(ptree, 0, 1);

		// find all roots and set corresponding isroot flag
		for (i = 0; i < pparam->rootno; i++) {
			pnode = ptree->firstnode;
			while (pnode != NULL) {
				if (strcmp(pnode->funname, pparam->root[i]) ==
				    0) {
					pnode->isroot = 1;
					// break; break missing because the same
					// function can have multiple
					// definitions in different files
				}

				pnode = pnode->next;
			}
		}

		// mark all nodes found scanning the tree starting from the
		// specified root
		// functions
		pnode = ptree->firstnode;
		while (iErr == 0 && pnode != NULL) {
			if (pnode->isroot) {
				// init, don't set color, don't reset isroot
				// flags
				outtreeinit(ptree, -1, 0);
				iErr = outsubtree(ptree, pparam, pnode,
						  pparam->fdepth,
						  pparam->bdepth, ROOTMARK);
			}

			pnode = pnode->next;
		}

		if (pparam->callp) {
			// if an highlight path has been specified
			pnode = ptree->firstnode;
			while (iErr == 0 && pnode != NULL) {
				if (strcmp(pnode->funname, pparam->callp) ==
				    0) {
					// found the last function of path
					// init, don't set color, don't reset
					// isroot flags
					outtreeinit(ptree, -1, 0);
					// bdepth and fdepth are inverted on
					// purpose, because we are scanning
					// toward roots
					// in the opposite direction
					// last parameter = 1 because we are
					// coloring the path
					iErr = outsubtree(ptree, pparam, pnode,
							  pparam->bdepth,
							  pparam->fdepth, 1);
					// break; break missing because the same
					// function can have multiple
					// defintions in different files
				}

				pnode = pnode->next;
			}
		}

		// init, don't set color, don't reset isroot flags
		outtreeinit(ptree, -1, 0);

		// this is the actual output production
		pnode = ptree->firstnode;
		while (iErr == 0 && pnode != NULL) {
			// find all roots and start scanning from those nodes
			if (pnode->isroot)
				iErr = outsubtree(ptree, pparam, pnode,
						  pparam->fdepth,
						  pparam->bdepth, -1);

			pnode = pnode->next;
		}

		// terminate the tree output
		iErrC = outclose(ptree, pparam);
		if (iErr == 0)
			iErr = iErrC;
	}

	if (pparam->verbose)
		printf("done\n");

	return iErr;
}
