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

#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _ALL_IN_ONE
#include "defines.h"
#include "slib.h"
#endif // _ALL_IN_ONE

// copy string with automatic memory allocation
int slibcpy(char **sout, char const *sin, int errval)
{
	// Pay attention to load *sout with NULL if it has never been allocated
	// before
	free(*sout);

	if (sin == NULL) {
		*sout = NULL; // NULL produces NULL
		return 0;
	}

	*sout = strdup(sin);
	if (*sout == NULL) {
		printf("\nMemory allocation error\n");
		return errval;
	}

	return 0;
}

// extract a file base name (with or without extension) from a full path name
int slibbasename(char **sbase, char *spath, int withext)
{
	char *bpath, *bname;

	free(*sbase);

	if (spath == NULL) {
		*sbase = NULL; // NULL produces NULL
		return 0;
	}

	bpath = strdup(spath);
	if (!bpath)
		return -1;

	bname = basename(bpath);
	if (!withext) {
		char *dot = strrchr(bname, '.');

		if (dot)
			*dot = '\0';
	}

	*sbase = bname;

	return 0;
}
