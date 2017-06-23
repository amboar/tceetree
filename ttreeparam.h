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

#ifndef _TTREEPARAM_H
#define _TTREEPARAM_H

#define TT_MAXROOTS 5  // maximum number of roots
#define TT_MAXSTYLES 6 // maximum number of styles + colors
#define TT_MAXEXCLUDF                                                          \
	20 // maximum number of functions that can be excluded from tree

#define TT_LIBRARY "LIBRARY" // name for library functions cluster

// maybe in the future we will have output for tools different from graphviz:
typedef enum treeouttype_e {
	TREEOUT_GRAPHVIZ, // graphviz output file
	TREEOUT_MAXNUM    // valid values below this
} treeouttype_t;

typedef struct treeparam_st {
	treeouttype_t outtype; // type of output file
	int printfile;	 // print filename of call near to branch if != 0
	int doclusters; // group functions into a cluster for each source file
	int fdepth;     // depth of callees tree (-1 = maximum)
	int bdepth;     // depth of callers tree (-1 = maximum)
	char *infile;   // input file (not compressed cscope output file)
	char *outfile;  // output file to use as input for graphviz-dot
	char *shortdbfile;	    // shortened cscope output file
	char *root[TT_MAXROOTS];      // root function names
	int rootno;		      // number of root functions
	char *callp;		      // highlighted call path function name
	int hlstyle;		      // highlight style
	char *excludf[TT_MAXEXCLUDF]; // functions to be excluded from tree
	int excludfno; // number of functions to be excluded from tree
	int verbose;   // verbose output
} treeparam_t;

#endif // #ifndef _TTREEPARAM_H
