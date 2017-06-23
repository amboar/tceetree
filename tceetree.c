/*
 * This source code is released for free distribution under the terms of the MIT License (MIT):
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
#include "ttreeparam.h"
#include "gettree.h"
#include "outtree.h"
#endif // _ALL_IN_ONE

const char sversion[] = "1.0.1";						// software version
const char sdefaultoutfile[] = "tceetree.out";			// default output file


// setting of string parameters
int paramstr(char ** sout, char const * sin)
{
	return slibcpy(sout, sin, -3);
}


// setting of string array parameters
int paramstrarr(char ** sout, int * outidx, int maxnum, char const * sin, char const * errmsg)
{
	if (*outidx >= maxnum)
	{
		printf(errmsg, maxnum);
		return -3;
	}

	return slibcpy(&sout[(*outidx)++], sin, -3);
}


// setting of default parameters
void paramdefault(treeparam_t *ptreeparam)
{
	memset(ptreeparam, 0, sizeof(treeparam_t));
	ptreeparam->fdepth = -1;							// default for called functions depth is maximum
	paramstr(&ptreeparam->infile, "cscope.out");		// if no input file is specified, default is "cscope.out"
	paramstr(&ptreeparam->outfile, sdefaultoutfile);	// default output file
	paramstr(&ptreeparam->shortdbfile, "");				// default shortened output file
	ptreeparam->outtype = TREEOUT_GRAPHVIZ;				// default is output for graphviz
}

// parameter cross checks
int paramcrosscheck(treeparam_t *ptreeparam)
{
	if (strcmp(ptreeparam->infile, ptreeparam->outfile) == 0)
	{
		printf("\nThe input file cannot be the same as the output file\n");
		return -1;
	}
	
	if (strcmp(ptreeparam->infile, ptreeparam->shortdbfile) == 0)
	{
		printf("\nThe input file cannot be the same as the shortened cscope output file\n");
		return -1;
	}

	if (strcmp(ptreeparam->outfile, ptreeparam->shortdbfile) == 0)
	{
		printf("\nThe output file cannot be the same as the shortened cscope output file\n");
		return -1;
	}

	return 0;
}

// free parameters memory
void paramfree(treeparam_t *ptreeparam)
{
	int i;

	free(ptreeparam->infile);
	free(ptreeparam->outfile);
	free(ptreeparam->shortdbfile);
	free(ptreeparam->callp);

	for (i = 0; i < ptreeparam->rootno; i++)
		free(ptreeparam->root[i]);
	for (i = 0; i < ptreeparam->excludfno; i++)
		free(ptreeparam->excludf[i]);
}


// print usage help
void usage(void)
{
	printf("\n");
	printf("Usage: tceetree [-c <depth>] [-C <depth>] [-d <file>] [-f] [-F] [-h]\n"
		   "                [-i <file>] [-o <file>] [-p <function>] [-r <root>]\n"
		   "                [-s <style>] [-v] [-V] [-x <function>]\n\n");
	printf("-c <depth>    Depth of tree for called functions: default is max.\n");
	printf("-C <depth>    Depth of tree for calling functions: default is 0.\n");
	printf("-d <file>     Output a shortened cscope output file: default is no output.\n"
		   "              The shortened file includes only function information and can\n"
		   "              be used as input (-i) for following calls to tceetree to\n"
		   "              increase speed on big projects.\n");
	printf("-f            Print the file name where the call is near to branch.\n");
	printf("-F            Group functions into one cluster for each source file.\n");
	printf("-h            Print this help.\n");
	printf("-i <file>     Input cscope output file: default is cscope.out.\n");
	printf("-o <file>     Output file for graphviz: default is %s.\n", sdefaultoutfile);
	printf("-p <function> Highlight call path till function.\n");
	printf("-r <root>     Root function of tree: default is main. This option may occur\n"
		   "              more than once for multiple roots (max %d).\n", TT_MAXROOTS);
	printf("-s <style>    Style for highlight call path:\n"
		   "              - 0 = red color (default);\n"
		   "              - 1 = blue color;\n"
		   "              - 2 = green color;\n"
		   "              - 3 = bold;\n"
		   "              - 4 = dashed;\n"
		   "              - 5 = dotted.\n");
	printf("-v            Print version.\n");
	printf("-V            Verbose output.\n");
	printf("-x <function> Function to be excluded from tree. This option may occur more\n"
		   "              than once for multiple functions (max %d).\n"
		   "              -x %s is a special case for excluding all library\n"
		   "              functions, i.e. not found defined in any file.\n", TT_MAXEXCLUDF, TT_LIBRARY);
}


// decoding of inline parameters
int usage_opt(char const * sopt, treeparam_t * ptreeparam)
{
	static char curopt = 0;					// holds option currently being parsed (e.g. 'd')
	int iErr = 0;
	int isoptval;							// = 1 when decoding value of option

	if (curopt == 0 && (sopt[0] != '-' || strlen(sopt) != 2))
		iErr = -1;
	else
	{
		isoptval = (curopt != 0);
		if (!isoptval)
			curopt = sopt[1];
		switch (curopt)
		{
			case 'c':
				if (isoptval)
				{
					if (strcmp(sopt, "max") == 0)
						ptreeparam->fdepth = -1;
					else
					if (sscanf(sopt, "%d", &ptreeparam->fdepth) != 1 || ptreeparam->fdepth < 0)
					{
						printf("\nCallees depth must be a number >= 0 or \"max\"\n");
						iErr = -3;
					}
					curopt = 0;
				}
				break;

			case 'C':
				if (isoptval)
				{
					if (strcmp(sopt, "max") == 0)
						ptreeparam->bdepth = -1;
					else
					if (sscanf(sopt, "%d", &ptreeparam->bdepth) != 1 || ptreeparam->bdepth < 0)
					{
						printf("\nCallers depth must be a number >= 0 or \"max\"\n");
						iErr = -3;
					}
					curopt = 0;
				}
				break;

			case 'd':
				if (isoptval)
				{
					iErr = paramstr(&ptreeparam->shortdbfile, sopt);
					curopt = 0;
				}
				break;				

			case 'f':
				ptreeparam->printfile = 1;
				curopt = 0;
				break;

			case 'F':
				ptreeparam->doclusters = 1;
				curopt = 0;
				break;

			case 'h':
				usage();
				iErr = -2;
				curopt = 0;
				break;

			case 'i':
				if (isoptval)
				{
					iErr = paramstr(&ptreeparam->infile, sopt);
					curopt = 0;
				}
				break;

			case 'o':
				if (isoptval)
				{
					iErr = paramstr(&ptreeparam->outfile, sopt);
					curopt = 0;
				}
				break;

			case 'p':
				if (isoptval)
				{
					iErr = paramstr(&ptreeparam->callp, sopt);
					curopt = 0;
				}
				break;

			case 'r':
				if (isoptval)
				{
					iErr = paramstrarr(ptreeparam->root, &ptreeparam->rootno, TT_MAXROOTS, sopt, "\nThe maximum number of root functions is %d\n");
					curopt = 0;
				}
				break;

			case 's':
				if (isoptval)
				{
					if (sscanf(sopt, "%d", &ptreeparam->hlstyle) != 1 || ptreeparam->hlstyle < 0 || ptreeparam->hlstyle >= TT_MAXSTYLES)
					{
						printf("\nHighlight style must be a number >= 0 and < %d\n", TT_MAXSTYLES);
						iErr = -3;
					}
					curopt = 0;
				}
				break;

			case 'v':
				printf("\n%s\n", sversion);
				iErr = -2;
				curopt = 0;
				break;

			case 'V':
				ptreeparam->verbose = 1;
				curopt = 0;
				break;

			case 'x':
				if (isoptval)
				{
					iErr = paramstrarr(ptreeparam->excludf, &ptreeparam->excludfno, TT_MAXEXCLUDF, sopt, "\nThe maximum number of excluded functions is %d\n");
					curopt = 0;
				}
				break;

			default:
				iErr = -1;
				break;
		}
	}

	if (iErr == -1)
		printf("\nInvalid option %s\n", sopt);

	return iErr;
}


int main(int argc, char * argv[])
{
	treeparam_t treeparam;
	ttree_t ttree;
	int i;
	int iErr = 0;

	// set parameters to default
	paramdefault(&treeparam);

	// decode inline options
	for (i = 1; i < argc; i++)
	{
		iErr = usage_opt(argv[i], &treeparam);
		if (iErr != 0)
			break;
	}

	if (iErr == 0)
		iErr = paramcrosscheck(&treeparam);

	if (iErr == 0)
	{
		if (treeparam.rootno == 0)
		{
			paramstr(&treeparam.root[0], "main");		// if no root is specified then start from main
			treeparam.rootno = 1;
		}

		ttreeinit(&ttree);								// initialize tree

		iErr = gettree(&ttree, &treeparam);				// read cscope file and get the whole tree
		if (iErr == 0)
			iErr = outtree(&ttree, &treeparam);			// make subtree output according to options
		
		ttreefree(&ttree);								// free tree memory
	}

	if (iErr == -2)
		iErr = 0;

	paramfree(&treeparam);								// free parameters memory

	//// printf("\ntceetree returns with: %d\n", iErr);

	return iErr;
}

