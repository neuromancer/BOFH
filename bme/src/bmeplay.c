//
// BME player - very simple program that plays XMs, S3Ms, MODs
//

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "fileio.h"
#include "bme.h"

int main(int argc, char **argv);
void handle_int(int);

int filetype;

char *mixmodename[] =
{
    "8-bit mono",
    "8-bit stereo",
    "16-bit mono",
    "16-bit stereo"
};


int main(int argc, char **argv)
{
    // Set a signal handler just to be sure that atexit functions get
    // called even if program is breaked

    signal(SIGINT, handle_int);

    if (argc < 2)
    {
        printf("Usage: BMEPLAY <mod/s3m/xm file>\n");
        return 0;
    }

    // Init sound using a 200ms Waveout buffer (more than enough
    // but latency is not a concern here)
    // snd_init() installs its own atexit() routine so we don't
    // have to do anything special at program exit

    if (snd_init(44100, SIXTEENBIT|STEREO, 200, 32, 0)) goto SND_OK;
    if (snd_init(22050, SIXTEENBIT|STEREO, 200, 32, 0)) goto SND_OK;
    if (!snd_init(11025, SIXTEENBIT|STEREO, 200, 32, 0))
    {
        printf("Soundsystem init failure!\n");
        return 1;
    }

    SND_OK:
    printf("Using %s sound at %d Hz\n", mixmodename[snd_mixmode], snd_mixrate);

    snd_setmusicmastervolume(32, 48);
    if (snd_loadxm(argv[1]))
    {
        filetype = 1;
        goto MOD_OK;
    }
    if (snd_loadmod(argv[1]))
    {
        filetype = 2;
        goto MOD_OK;
    }
    if (snd_loads3m(argv[1]))
    {
        filetype = 3;
        goto MOD_OK;
    }
    printf("Error loading module!\n");
    goto EXIT;

    MOD_OK:
    switch(filetype)
    {
        case 1:
        printf("XM name:%s\n", snd_getxmname());
        printf("Channels:%d\n", snd_getxmchannels());
        snd_playxm(0);
        break;

        case 2:
        printf("Module name:%s\n", snd_getmodname());
        printf("Channels:%d\n", snd_getmodchannels());
        snd_playmod(0);
        break;

        case 3:
        printf("S3M name:%s\n", snd_gets3mname());
        printf("Channels:%d\n", snd_gets3mchannels());
        snd_plays3m(0);
        break;
    }

	getchar();

    EXIT:
    return 0;
}

void handle_int(int signal_number)
{
    exit(0);
}

