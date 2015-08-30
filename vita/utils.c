// Author: skogaby
// Lots of examples were followed using other emulators found on github
// This file is mostly implementing functions that aren't present in
// the libraries we have available, but we need.

#include "utils.h"

/***
 * Implementing stpcpy here so we don't need to build with libg.
 */
char *stpcpy (char *dst, const char *src)
{
	const size_t len = strlen (src);
	return (char *) memcpy (dst, src, len + 1) + len;
}

/***
 * Implementing strndup here for our own convenience.
 */
char *strndup (const char *s, size_t size)
{
	char *r;
	char *end = (char*)memchr(s, 0, size);

	if (end)
		size = end - s + 1;

	r = (char*)malloc(size);

	if (size)
	{
		memcpy(r, s, size-1);
		r[size-1] = '\0';
	}

	return r;
}