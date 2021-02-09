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

#define _GNU_SOURCE
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _ALL_IN_ONE
#include "defines.h"
#include "gettree.h"
#endif // _ALL_IN_ONE

#define MAXLINEF 5000 // maximum length of a line in cscope.out

int gettree(ttree_t *ptree, treeparam_t *pparam)
{
	int iErr = 0;
	FILE *filein, *filedbout;
	char *sLine, *sfilename, *scaller;
	ttreenode_t *ncaller, *ncallee;
	long lineidx;

	sLine = malloc(3 * MAXLINEF);
	if (!sLine)
		return -1;

	sfilename = sLine + 1 * MAXLINEF;
	scaller = sLine + 2 * MAXLINEF;

	/* Get all Nodes */
	filein = fopen(pparam->infile, "r");
	if (filein == NULL) {
		printf("\nError while opening input file\n");
		goto cleanup_sLine;
	}

	filedbout = NULL;
	if (pparam->shortdbfile[0] != 0) {
		filedbout = fopen(pparam->shortdbfile, "w");
		if (filedbout == NULL) {
			printf("\nError while opening shortened cscope db file\n");
			iErr = -1;
			goto cleanup_filein;
		}
	}

	lineidx = 0;
	if (pparam->verbose)
		printf("\n");

	while (iErr == 0 && fgets(sLine, MAXLINEF, filein) != NULL) {
		bool interesting;

		lineidx++;
		if (pparam->verbose)
			printf("Getting tree nodes... line %ld\r", lineidx);

		if (sLine[0] != '\t')
			continue;

		interesting = sLine[1] == '@' ||
			      sLine[1] == '$' ||
			      sLine[1] == '`';
		if (filedbout && interesting)
			fputs(sLine, filedbout);

		*strchrnul(sLine, '\n') = '\0';

		switch (sLine[1]) {
		case '@':
			// filename where function is defined
			strcpy(sfilename, &sLine[2]);
			break;

		case '$':
			// add one node for each function definition
			if (!ttreeaddnode(ptree, &sLine[2], sfilename))
				goto cleanup_filein;
			break;
		case '#':
			// add one node for each macro definition
			if (!ttreeaddnode(ptree, &sLine[2], sfilename))
				goto cleanup_filein;
			break;
		default:
			break;
		}
	}

	if (lineidx == 0) {
		printf("\nInput file is empty\n");
		iErr = -1;
	}

	if (filedbout != NULL && fclose(filedbout) != 0) {
		printf("\nError while closing shortened cscope db file\n");
		iErr = -1;
	}

	if (iErr != 0)
		goto cleanup_filein;

	/* Get all Branches */
	if (fseek(filein, 0, SEEK_SET) != 0) {
		printf("\nError seeking to beginning of input file\n");
		iErr = -1;
		goto cleanup_filein;
	}

	if (pparam->verbose)
		printf("\n");

	lineidx = 0;
	while (iErr == 0 && fgets(sLine, MAXLINEF, filein) != NULL) {
		if (pparam->verbose) {
			lineidx++;
			printf("Getting tree branches... line %ld\r", lineidx);
		}

		if (sLine[0] != '\t')
			continue;

		*strchrnul(sLine, '\n') = '\0';

		switch (sLine[1]) {
		case '@':
			// get again filename where caller is defined
			strcpy(sfilename, &sLine[2]);
			break;

		case '$':
			// get the name of caller function
			strcpy(scaller, &sLine[2]);
			break;

		case '#':
			// get the name of caller macro
			strcpy(scaller, &sLine[2]);
			break;

		case '`':
			if (!*sfilename) {
				printf("\nFilename where the call is has not been found\n");
				iErr = -1;
				break;
			}

			// find the caller function node
			ncaller = ttreefindnode(ptree, scaller, sfilename);
			if (ncaller == NULL)
				break;

			// find the callee function node
			ncallee = ttreefindnode(ptree, &sLine[2], NULL);
			if (ncallee == NULL) {
				// could not find the callee function: it must
				// be a library function: create its node now
				ncallee = ttreeaddnode(ptree, &sLine[2], NULL);
			}
			// add branch
			if (iErr == 0)
				iErr = ttreeaddbranch(ptree, ncaller, ncallee,
						      sfilename);
			break;
		default:
			break;
		}

		if (iErr != 0)
			break;
	}

cleanup_filein:
	if (fclose(filein) != 0) {
		printf("\nError while closing input file\n");
		iErr = -1;
	}

cleanup_sLine:
	free(sLine);

	return iErr;
}
