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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _ALL_IN_ONE
#include "defines.h"
#include "outgraphviz.h"
#include "slib.h"
#endif // _ALL_IN_ONE

static FILE *grafile = NULL; // output file pointer

#define HSTYLESNO TT_MAXSTYLES // maximum colors + styles
#define HSTYLES1 3	     // number of colors

// user will specify just a number between 0 and (TT_MAXSTYLES - 1) to choose
// among some colors OR styles
char *hlstyles[HSTYLESNO] = {
    "red",    // 0			// Colors
    "blue",   // 1
    "green",  // 2
    "bold",   // 3 = HSTYLES1	// Styles
    "dashed", // 4
    "dotted"  // 5
};

// start graph
int outopen_gra(ttree_t *ptree __attribute__((unused)), treeparam_t *pparam)
{
	int iErr = 0, iEbase;
	char *sbasename = NULL;

	grafile = fopen(pparam->outfile, "w");
	if (grafile == NULL) {
		printf("\nError while opening output file\n");
		iErr = -1;
	} else {
		// name the graph after the output file base name, without
		// extension
		iEbase = slibbasename(&sbasename, pparam->outfile, 0);
		if (iEbase != 0)
			sbasename = "tceetree"; // on error use this default
		fprintf(grafile, "digraph %s\n{\n", sbasename);
		if (iEbase == 0)
			free(sbasename);
	}

	return iErr;
}

// end graph
int outclose_gra(ttree_t *ptree __attribute__((unused)),
		 treeparam_t *pparam __attribute__((unused)))
{
	int iErr = 0;

	fprintf(grafile, "}\n");

	if (fclose(grafile) != 0) {
		printf("\nError while closing output file\n");
		iErr = -1;
	}

	grafile = NULL;

	return iErr;
}

// print one node
int outnode_gra(ttreenode_t *pnode, treeparam_t *pparam)
{
	int iErr = 0;
	char *sclustername = NULL;
	char *sclusterlabel = NULL;

	if (grafile == NULL || pnode == NULL || pnode->funname == NULL)
		return -1;

	fprintf(grafile, "\t");
	if (pparam->doclusters) {
		// group functions into a cluster for each file using filename without
		// path and with extension as cluster label
		iErr = slibbasename(&sclusterlabel, pnode->filename, 1);
		if (iErr != 0)
			return iErr; 
		// if no filename present, group function into the library cluster
		if (sclusterlabel == NULL) {
			sclusterlabel = strdup(TT_LIBRARY);
			if (sclusterlabel == NULL)
				return errno;
		}

		sclustername = strdup(sclusterlabel);
		if (sclustername == NULL) {
			free(sclusterlabel);
			return errno;
		}

		iErr = slibreplacechr(sclustername, '.', '_');
		if (iErr != 0) {
			free(sclusterlabel);
			free(sclustername);
			return iErr;
		}

		// add statement for assigning function to the cluster
		fprintf(grafile, "subgraph \"cluster_%s\" { "
				 "label=\"%s\"; "
				 "labeljust=\"l\"; ",
			sclustername, sclusterlabel);
		free(sclusterlabel);
		free(sclustername);

		// print node name
		// when clustering functions by files, we need to create a
		// more unique name for the node by appending the filename
		char snodename[256];
		iErr = ttreegetextendednodename(snodename, sizeof(snodename), pnode);
		if (iErr != 0) {
			free(sclustername);
			free(sclusterlabel);
			return iErr;
		}
		fprintf(grafile, "\"%s\"", snodename);

		// add a label with just the function name to the node
		fprintf(grafile, " [label=\"%s\"]", pnode->funname);
	} else {
		// print node name
		fprintf(grafile, "\"%s\"", pnode->funname);
	}

	// print style information
	if (pnode->icolor > 0) {
		// add style or color attributes for path 
		// between root and specified function
		if (pparam->hlstyle >= HSTYLES1)
			fprintf(grafile, " [style=\"%s\"]",
				hlstyles[pparam->hlstyle]);
		else
			fprintf(grafile, " [color=\"%s\",fontcolor=\"%s\"]",
			    hlstyles[pparam->hlstyle],
			    hlstyles[pparam->hlstyle]);
	}
	fprintf(grafile, ";");
	// close cluster statement, if cluster enabled
	if (pparam->doclusters)
		fprintf(grafile, " }");
	fprintf(grafile, "\n");

	return iErr;
}

// print one branch
int outbranch_gra(ttreebranch_t *pbranch, treeparam_t *pparam)
{
	int iErr = 0;
	char *sbasename = NULL;

	if (grafile && pbranch && pbranch->parent.node && pbranch->child.node &&
	    pbranch->parent.node->funname && pbranch->child.node->funname) {
		// print the branch: caller -> callee;
		if (pparam->doclusters) {
			char scallername[256];
			char scalleename[256];
			iErr = ttreegetextendednodename(scallername, sizeof(scallername), 
											pbranch->parent.node);
			if (iErr != 0)
				return iErr;
			iErr = ttreegetextendednodename(scalleename, sizeof(scalleename),
											pbranch->child.node);
			if (iErr != 0)
				return iErr;
			fprintf(grafile, "\t\"%s\"->\"%s\"", scallername, scalleename);
		} else {
			fprintf(grafile, "\t\"%s\"->\"%s\"", pbranch->parent.node->funname,
			pbranch->child.node->funname);
		}
		if (pparam->printfile && pbranch->parent.filename) {
			// if enabled, print the filename where the call has
			// been found
			iErr = slibbasename(&sbasename, pbranch->parent.filename, 1);
			if (iErr == 0) {
				fprintf(grafile, " [label=\"%s\"", sbasename);
				if (pbranch->icolor > 0) {
					// if path is to be highlighted, add
					// color or style attributes
					if (pparam->hlstyle >= HSTYLES1)
						fprintf(
						    grafile, ",style=\"%s\"",
						    hlstyles[pparam->hlstyle]);
					else
						fprintf(
						    grafile, ",color=\"%s\","
							     "fontcolor=\"%s\"",
						    hlstyles[pparam->hlstyle],
						    hlstyles[pparam->hlstyle]);
				}
				fprintf(grafile, "]");
				free(sbasename);
			}
		} else {
			// filename is not printed near to the arrow
			if (pbranch->icolor > 0) {
				// if path is to be highlighted, add color or
				// style attributes
				if (pparam->hlstyle >= HSTYLES1)
					fprintf(grafile, " [style=\"%s\"]",
						hlstyles[pparam->hlstyle]);
				else
					fprintf(grafile, " [color=\"%s\"]",
						hlstyles[pparam->hlstyle]);
			}
		}
		fprintf(grafile, ";\n");
	}

	return iErr;
}
