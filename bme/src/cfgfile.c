/* Configuration file handling for BME utilities.

   Copyright (c) 2003, Kalle Olavi Niemitalo.
   This file may be used under the same terms as the rest of BME.
   See readme.txt.  */

#if __WIN32__

# include <stdio.h>
# include "cfgfile.h"

FILE *cfgfile_open(const char *filename, const char *opentype)
{
	/* Just use the current directory.  */
	return fopen(filename, opentype);
}

#else  /* !__WIN32__ */

# include <errno.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/stat.h>
# include "cfgfile.h"

FILE *cfgfile_open(const char *filename, const char *opentype)
{
	/* Use a ".bme" subdirectory of the home directory.  */
	const char *home;
	size_t namebufsize;
	char *namebuf = NULL;
	size_t printsize;
	FILE *file = NULL;

	home = getenv("HOME");
	if (home == NULL) {
		errno = ENOENT;
		goto out;
	}

	/* If this overflows, snprintf() will fail below.  */
	namebufsize = strlen(home) + 6 + strlen(filename) + 1;
	namebuf = malloc(namebufsize);
	if (namebuf == NULL) {
		errno = ENOMEM;
		goto out;
	}

	if (opentype[0] != 'r') {
		/* Try to create the ".bme" directory.  */
		printsize = snprintf(namebuf, namebufsize, "%s/.bme", home);
		if (printsize >= namebufsize) {
			errno = ENAMETOOLONG;
			goto out;
		}
		/* Ignore errors from mkdir().  If the directory
		   doesn't exist and we can't create it, then fopen()
		   will fail.  */
		mkdir(namebuf, S_IRUSR | S_IWUSR | S_IXUSR);
	}

	printsize = snprintf(namebuf, namebufsize, "%s/.bme/%s",
			     home, filename);
	if (printsize >= namebufsize) {
		errno = ENAMETOOLONG;
		goto out;
	}
	file = fopen(namebuf, opentype);

 out:
	free(namebuf);		/* may be NULL */
	return file;
}

#endif /* !__WIN32__ */
