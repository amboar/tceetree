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
#endif // _ALL_IN_ONE

// copy string with automatic memory allocation
int slibcpy(char **sout, char const *sin, int errval)
{
	free(*sout); // Pay attention to load *sout with NULL if it has never
		     // been
		     // allocated before

	if (sin == NULL) {
		*sout = NULL; // NULL produces NULL
	} else {
		*sout = (char *)calloc(strlen(sin) + 1, sizeof(char));
		if (*sout == NULL) {
			printf("\nMemory allocation error\n");
			return errval; // the error value is specified as input
				       // parameter
		}

		strcpy(*sout, sin);
	}

	return 0;
}

// extract a file base name (with or without extension) from a full path name
int slibbasename(char **sbase, char *spath, int withext)
{
	int i, iErr = 0;

	if (spath == NULL) {
		*sbase = NULL; // NULL produces NULL
	} else {
		for (i = strlen(spath) - 1; i >= 0; i--)
			if (spath[i] == '\\' || spath[i] == '/')
				break;

		iErr = slibcpy(sbase, spath + i + 1, -1);

		if (iErr == 0 && !withext) {
			for (i = 0; i < (int)strlen(*sbase); i++)
				if ((*sbase)[i] == '.') {
					(*sbase)[i] = 0;
					break;
				}
		}
	}

	return iErr;
}
