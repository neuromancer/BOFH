//
// BLOCKINFO EDITOR  
//

#ifndef __WIN32__
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "fileio.h"
#include "cfgfile.h"
#include "bme.h"

#define SPR_FONTS 0
#define SPR_EDITOR 1
#define MAXBLOCKS 10000

unsigned mousex = 160, mousey = 100;
int mouseb, prevmouseb;
int mark = 0;
int speed;
int key;
int blk = 0;
int screenmode = 0;
char printbuffer[80];
char copybuffer[16];
char infname[13] = {0,0,0,0,0,0,0,0,0,0,0,0,0};
char palname[13] = {0,0,0,0,0,0,0,0,0,0,0,0,0};
char blkname[13] = {0,0,0,0,0,0,0,0,0,0,0,0,0};

unsigned char *blkinfdata;

extern unsigned char datafile[];

void mainloop(void);
void printinfo(void);
void setscreenmode(void);
void loadinf(void);
void saveinf(void);
void loadconfig(void);
void saveconfig(void);
void enterfilename(char *text, char *buffer);
void help(void);
void printstatus(void);
void loadpalette(void);
void loadblocks(void);
void initstuff(void);
void handle_int(int a);
void generalcommands(void);
void getmousebuttons(void);
void getmousemove(void);

int main(int argc, char *argv[])
{
    io_openlinkeddatafile(datafile);

    loadconfig();
    initstuff();
    mainloop();
    saveconfig();
    return 0;
}

void loadconfig(void)
{
    FILE *handle = cfgfile_open("infedit.cfg", "rb");

    if (!handle) return;
    fread(&blkname, sizeof blkname, 1, handle);
    fread(&palname, sizeof palname, 1, handle);
    fread(&infname, sizeof infname, 1, handle);
    fread(&blk, sizeof blk, 1, handle);
    fread(&screenmode, sizeof screenmode, 1, handle);
    fread(&win_fullscreen, sizeof win_fullscreen, 1, handle);
    fclose(handle);
}

void saveconfig(void)
{
    FILE *handle = cfgfile_open("infedit.cfg", "wb");

    if (!handle) return;
    fwrite(&blkname, sizeof blkname, 1, handle);
    fwrite(&palname, sizeof palname, 1, handle);
    fwrite(&infname, sizeof infname, 1, handle);
    fwrite(&blk, sizeof blk, 1, handle);
    fwrite(&screenmode, sizeof screenmode, 1, handle);
    fwrite(&win_fullscreen, sizeof win_fullscreen, 1, handle);
    fclose(handle);
}

void mainloop(void)
{
    gfx_calcpalette(64, 0, 0, 0);
    gfx_setpalette();
    win_getspeed(60);

    for (;;)
    {
        speed = win_getspeed(60);
        key = kbd_getkey();
        getmousemove();
        getmousebuttons();
        if ((key == KEY_ESC) || (win_quitted)) break;
        generalcommands();
        gfx_fillscreen(0);
        printinfo();
        printstatus();
        gfx_drawsprite(mousex, mousey, 0x00010001);
        gfx_updatepage();
    }
}

void printinfo(void)
{
    int y, x, f;
    unsigned char *infoptr = &blkinfdata[16*blk];
    txt_print(40, 96, SPR_FONTS, "0");
    txt_print(40+24, 96, SPR_FONTS, "1");
    txt_print(40+2*24, 96, SPR_FONTS, "2");
    txt_print(40+3*24, 96, SPR_FONTS, "3");
    txt_print(40+4*24, 96, SPR_FONTS, "4");
    txt_print(40+5*24, 96, SPR_FONTS, "5");
    txt_print(40+6*24, 96, SPR_FONTS, "6");
    txt_print(40+7*24, 96, SPR_FONTS, "7");
    for (y = 0; y < 4; y++)
    {
        for (x = 0; x < 4; x++)
        {
            if (*infoptr & 1) f=0x00010005; else f=0x00010006;
            gfx_drawsprite(32+x*4,64+y*4,f);
            if (*infoptr & 2) f=0x00010005; else f=0x00010006;
            gfx_drawsprite(32+24+x*4,64+y*4,f);
            if (*infoptr & 4) f=0x00010005; else f=0x00010006;
            gfx_drawsprite(32+2*24+x*4,64+y*4,f);
            if (*infoptr & 8) f=0x00010005; else f=0x00010006;
            gfx_drawsprite(32+3*24+x*4,64+y*4,f);
            if (*infoptr & 16) f=0x00010005; else f=0x00010006;
            gfx_drawsprite(32+4*24+x*4,64+y*4,f);
            if (*infoptr & 32) f=0x00010005; else f=0x00010006;
            gfx_drawsprite(32+5*24+x*4,64+y*4,f);
            if (*infoptr & 64) f=0x00010005; else f=0x00010006;
            gfx_drawsprite(32+6*24+x*4,64+y*4,f);
            if (*infoptr & 128) f=0x00010005; else f=0x00010006;
            gfx_drawsprite(32+7*24+x*4,64+y*4,f);
            infoptr++;
        }
    }
}

void generalcommands(void)
{
    // Block selecting
    if (key == KEY_Z) blk--;
    if (key == KEY_X) blk++;
    if (key == KEY_COMMA) blk--;
    if (key == KEY_COLON) blk++;
    if (key == KEY_A) blk -= 10;
    if (key == KEY_S) blk += 10;
    if (key == KEY_F1) loadinf();
    if (key == KEY_F2) saveinf();
    if (key == KEY_F3) loadblocks();
    if (key == KEY_F4) loadpalette();
    if (blk < 0) blk = 0;
    if (blk > (int)gfx_nblocks) blk = gfx_nblocks;
    if (key == KEY_F10) help();
    if (key == KEY_F12)
    {
        screenmode++;
        if (screenmode > GFX_DOUBLESIZE) screenmode = 0;
        setscreenmode();
    }
    if (key == KEY_C) memset(&blkinfdata[blk*16], 0, 16);
    if (key == KEY_P) memcpy(copybuffer, &blkinfdata[blk*16], 16);
    if (key == KEY_T) memcpy(&blkinfdata[blk*16], copybuffer, 16);

    if ((mousey >= 64) && (mousey < 80))
    {
        if ((mousex >= 32) && (mousex < 8*24+32))
        {
            int bit = (mousex - 32) / 24;
            int fine = (mousex - 32) % 24;
            if (fine < 16)
            {
                if (mouseb & 1)
                {
                    blkinfdata[blk*16+((mousey-64)/4)*4+fine/4] |= 1 << bit;
                }
                if (mouseb & 2)
                {
                    blkinfdata[blk*16+((mousey-64)/4)*4+fine/4] &= 0xff - (1 << bit);
                }
            }
        }
    }
}

void printstatus(void)
{
    gfx_drawblock(8, 8, blk);
    sprintf(printbuffer, "%04d", blk);
    txt_print(32, 8, SPR_FONTS, printbuffer);
    txt_print(240,180, SPR_FONTS, "F10 = HELP");
}

void loadblocks(void)
{
    enterfilename("LOAD BLOCKS", blkname);
    gfx_loadblocks(blkname);
}

void loadinf(void)
{
    FILE *handle;
    enterfilename("LOAD BLOCKINFO", infname);
    handle = fopen(infname, "rb");
    if (!handle) return;
    fread(&blkinfdata[0], (gfx_nblocks+1) * 16, 1, handle);
    fclose(handle);
}

void saveinf(void)
{
    FILE *handle;
    enterfilename("SAVE BLOCKINFO", infname);
    handle = fopen(infname, "wb");
    if (!handle) return;
    fwrite(&blkinfdata[0], (gfx_nblocks+1) * 16, 1, handle);
    fclose(handle);
}

void loadpalette(void)
{
    enterfilename("LOAD PALETTE", palname);
    gfx_loadpalette(palname);
    gfx_calcpalette(64, 0, 0, 0);
    gfx_setpalette();
}

void help(void)
{
    gfx_fillscreen(0);
    txt_printcenter(0, SPR_FONTS, "BLOCKINFO EDITOR HELP");
    txt_printcenter(16, SPR_FONTS, "F1 - LOAD BLOCKINFO");
    txt_printcenter(24, SPR_FONTS, "F2 - SAVE BLOCKINFO");
    txt_printcenter(32, SPR_FONTS, "F3 - LOAD MAP");
    txt_printcenter(40, SPR_FONTS, "F4 - SAVE MAP");
    txt_printcenter(48, SPR_FONTS, "F10 - THIS SCREEN");
    txt_printcenter(56, SPR_FONTS, "F12 - SWITCH SCREENMODE");
    txt_printcenter(64, SPR_FONTS, "ESC - EXIT PROGRAM");
    txt_printcenter(80, SPR_FONTS, "Z,X,A,S - SELECT BLOCK");
    txt_printcenter(88, SPR_FONTS, "C - CLEAR BLOCKINFO FOR CURRENT BLOCK");
    txt_printcenter(96, SPR_FONTS, "P,T - PUT/TAKE BLOCKINFO TO COPYBUFFER");
    txt_printcenter(112, SPR_FONTS, "LEFT MOUSEBUTTON SETS A BIT");
    txt_printcenter(120, SPR_FONTS, "RIGHT MOUSEBUTTON CLEARS A BIT");
    gfx_updatepage();

    for (;;)
    {
        win_getspeed(60);
        key = kbd_getkey();
        getmousemove();
        getmousebuttons();
        if ((mouseb) || (key)) break;
    }
}

void enterfilename(char *text, char *buffer)
{
    int c;
    for (c = strlen(buffer); c < 13; c++) buffer[c] = 0;

    kbd_getascii();

    for (;;)
    {
        int ascii;
        int cursorpos;

        win_getspeed(60);
        key = kbd_getkey();
        ascii = kbd_getascii();
        getmousemove();
        getmousebuttons();

        cursorpos = strlen(buffer);
        if (ascii == 8)
        {
            if (cursorpos)
            {
                buffer[cursorpos-1] = 0;
            }
        }
        if (ascii == 27)
        {
            memset(buffer, 0, 13);
            return;
        }
        if (ascii == 13) return;
        if ((ascii >= 32) && (cursorpos < 12))
        {
            buffer[cursorpos] = toupper(ascii);
        }

        gfx_fillscreen(0);
        txt_printcenter(80, SPR_FONTS, text);
        txt_printcenter(90, SPR_FONTS, buffer);
        gfx_updatepage();
    }
}

void initstuff(void)
{
    FILE *handle;

    win_openwindow("Block-info Editor V1.26", NULL);
    win_fullscreen = 1;
    win_setmousemode(MOUSE_ALWAYS_HIDDEN);
    setscreenmode();
    kbd_init();
    mou_init();
    if (!gfx_loadsprites(SPR_FONTS, "fonts.spr"))
    {
        win_messagebox("Sprite load error (FONTS.SPR)");
        exit(1);
    }
    if (!gfx_loadsprites(SPR_EDITOR, "editor.spr"))
    {
        win_messagebox("Sprite load error (EDITOR.SPR)");
        exit(1);
    }
    blkinfdata = malloc((MAXBLOCKS+1)*16);
    if (!blkinfdata)
    {
        win_messagebox("No memory for blockinfotable!");
        exit(1);
    }
    memset(blkinfdata, 0, (MAXBLOCKS+1)*16);

    io_setfilemode(0); // Rest of file access happens without datafile

    gfx_loadblocks(blkname);
    gfx_loadpalette(palname);
    gfx_calcpalette(64,0,0,0);
    gfx_setpalette();
    handle = fopen(infname, "rb");
    if (!handle) return;
    fread(&blkinfdata[0], (gfx_nblocks+1) * 16, 1, handle);
    fclose(handle);
}

void setscreenmode(void)
{
    if (!gfx_init(320,200,60,screenmode | GFX_USE3PAGES))
    {
        win_messagebox("Graphics init error!\n");
        saveconfig();
        exit(1);
    }
}

void handle_int(int a)
{
    exit(0); // Atexit functions will be called!
}

void getmousemove(void)
{
    mou_getpos(&mousex, &mousey);
}

void getmousebuttons(void)
{
    prevmouseb = mouseb;
    mouseb = mou_getbuttons();
}

