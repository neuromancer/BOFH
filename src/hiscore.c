#include "bofh.h"
#include "extern.h"
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define NUMSCORES 10
#define SCORELISTSIZE (NUMSCORES * sizeof (HISCORE_ENTRY))

/*
 * Compare scores, in qsort style.  Return negative if *a is better
 * than *b (so *a should come before *b in the list).
 */
static int compare_scores(const void *av,
                          const void *bv)
{
        const HISCORE_ENTRY *const a = av;
        const HISCORE_ENTRY *const b = bv;
        
        if (a->score > b->score)
                return -1;
        else if (a->score < b->score)
                return +1;
        else
                return strcmp(a->name, b->name);
}


/*
 * Merge two sorted high-score lists into one.
 * Coalesce identical scores.
 * Arguments must not overlap.
 */
static void merge_scores(const HISCORE_ENTRY a[],
                         const HISCORE_ENTRY b[],
                         HISCORE_ENTRY out[])
{
        int i;
        for (i=0; i<NUMSCORES; ++i)
        {
                int cmp = compare_scores(a, b);
                if (cmp <= 0)
                        out[i] = *a++;
                if (cmp >= 0)
                        out[i] = *b++;
        }
}

/* Check whether the scores loaded from disk are valid.  */
static int scores_look_ok(const HISCORE_ENTRY arr[])
{
        int i;
        for (i=0; i<NUMSCORES; ++i)
        {
                if (memchr(arr[i].name, '\0', NAMELENGTH) == NULL)
                        return 0; /* too long name */
                if (i>0 && arr[i-1].score < arr[i].score)
                        return 0; /* wrong order */
                /*
                 * We don't check the sorting of names here, because
                 * older versions of BOFH didn't enforce that.
                 * Instead, merge_from_file() qsorts the arrays.
                 */
        }

        return 1;
}


static void merge_from_file(int fd)
{
        HISCORE_ENTRY theirs[NUMSCORES];
        if (lseek(fd, SEEK_SET, 0) == 0
            && read(fd, &theirs, SCORELISTSIZE) == SCORELISTSIZE
            && scores_look_ok(theirs))
        {
                HISCORE_ENTRY ours[NUMSCORES];
                memcpy(&ours, &hiscore, SCORELISTSIZE);
                qsort(ours, NUMSCORES, sizeof ours[0], compare_scores);
                qsort(theirs, NUMSCORES, sizeof theirs[0], compare_scores);
                merge_scores(ours, theirs, hiscore);
        }
}

static void save_to_file(int fd)
{
        if (lseek(fd, SEEK_SET, 0) == 0)
        {
                (void) write(fd, &hiscore, SCORELISTSIZE);
                /*
                 * If the write fails, then the score file may become
                 * corrupted.  It would be nice to mark it as invalid,
                 * but there is no reliable way to do so.
                 */
        }
}

void loadhiscore(void)
{
        int fd = open(DIR_SCORE "/bofh.hsc", O_RDONLY);
        if (fd != -1)
        {
#ifndef __WIN32__
                struct flock fl;
                fl.l_type = F_RDLCK;
                fl.l_whence = SEEK_SET;
                fl.l_start = 0;
                fl.l_len = 0;
                if (fcntl(fd, F_SETLKW, &fl) != -1)
#endif
                        merge_from_file(fd);
                close(fd);      /* releases the lock */
        }
}

void savehiscore(void)
{
        /*
         * If the following call creates the score file, it gives full
         * read+write permissions to everyone permitted by umask.
         * This is intentional.  Consider the following cases:
         *
         * 1. System-wide setuid installation.  This is a security
         * risk: if a user can crack the game, he can trojanize its
         * executable.  We don't support this configuration.
         *
         * 2. System-wide setgid installation.  The score file should
         * be owned by some system UID so that people cannot edit it
         * directly.  There is no way to create a file with such
         * permissions here, so it has to be created at installation
         * time, and the permissions we use here don't matter.
         *
         * 3. Personal installation.  The user should have a
         * reasonable umask.
         */
        int fd = open(DIR_SCORE "/bofh.hsc", O_RDWR | O_CREAT,
#ifndef __WIN32__
                      S_IRUSR | S_IRGRP | S_IROTH
                      | S_IWUSR | S_IWGRP | S_IWOTH);
#else
                      S_IREAD | S_IWRITE);
#endif                      
        if (fd != -1)
        {
#ifndef __WIN32__
                struct flock fl;
                fl.l_type = F_WRLCK;
                fl.l_whence = SEEK_SET;
                fl.l_start = 0;
                fl.l_len = 0;
                if (fcntl(fd, F_SETLKW, &fl) != -1)
#endif
                {
                        merge_from_file(fd);
                        save_to_file(fd);
                }
                close(fd);      /* releases the lock */
        }
}
