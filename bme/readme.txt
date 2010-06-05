BLASPHEMOUS MULTIMEDIA ENGINE V1.31
-----------------------------------

Written by Lasse Öörni (loorni@student.oulu.fi)
Some things in the sound engine by Olli Niemitalo (oniemita@student.oulu.fi)
Changes & improvements by Kalle Niemitalo (kon@iki.fi)


1. GENERAL INFORMATION
----------------------

BME was formerly called JUDAS V2.07fw, but I finally decided it was time
for a change, mainly because JUDAS was originally just a soundsystem and, as
far as I know, it isn't developed anymore :-)

BME is a library for (game) programming with an oldskool (DOS-like)
attitude, containing:

- 256 color graphics routines
  * Using SDL for low level support
  * Palette, Block (tile), Sprite routines
  * Some primitive functions like Plot, Line, Fillscreen

- Sound routines
  * Using SDL for low level support
  * Softwaremixing with linear interpolation
  * RAW/WAV sample support
  * MOD/XM/S3M playing

- Keyboard, joystick & mouse routines
  * Using SDL events

- Timing routines
  * Using SDL timing functions

- Datafile routines
  * For merging all data into one file and possibly linking it with the EXE

- Text routines
  * For printing text with sprites

- Block(tile) map routines
  * For oldskool 2D game background graphics

Applications using BME should work on any platform that has SDL ported on it.
See SDL homepage at

        http://www.libsdl.org

To use BME library under Win32, you should install the MINGW development
enviroment. For filesize reasons, precompiled binaries exist only for Win32.

BME is distributed under the terms of the BSD license:

Copyright (c) 2000-2003, Lasse Öörni, Olli Niemitalo. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

- Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.
- The names of its contributors may not be used to endorse or promote products
  derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


2. HOW TO USE - VERY SHORT INSTRUCTIONS
---------------------------------------

Win32/MINGW:
Include "bme.h" in your program and compile with:
gcc yourprogram.c libbme.a -lmingw32 -lSDLmain -lSDL

Unix-like systems:
Include "bme.h" in your program and compile with:
gcc yourprogram.c libbme.a -L/usr/X11R6/lib -lXext -lm -ldl -lSDLmain
-lSDL `sdl-config --libs --cflags`

Look at the included utilities' source code to see the how the library
functions are being used in them. Also, look at the graphical C64 tools
at http://covertbitops.cjb.net, they use BME. And finally, a fairly complex
example of the use of BME is the game BOFH: Servers Under Siege, available at

        http://www.student.oulu.fi/~loorni/software/winbofh.zip (binary)
        http://www.student.oulu.fi/~loorni/software/bofhsrc.zip (source)

Purpose of the utilities:

BMECONV  - Graphics converter program which can read IFF/LBM (Deluxepaint
           format) pictures and make raw pictures, palettes, sprite & block
           files of them
BMEPLAY  - Simple example of XM/MOD/S3M playing with Waveout. (a console
           application)
DATAFILE - compiles datafiles to be used with the io_opendatafile() function
DAT2INC  - Creates a C file containing a big char array :) out of a
           datafile. Then you can use io_openlinkeddatafile(datafile) to
           open the datas, without having to do suspicious things like
           linking additional binary data to the exe
INFEDIT  - Utility to create collision maps (blockinfos) for the blocks
MAPEDIT  - Utility to create block maps up to 4 layers deep


3. SUMMARY FOR ALL LIBRARY FUNCTIONS
------------------------------------

For most functions that return something, zero return value means failure
(look at the variable bme_error for error cause) and nonzero success. Except
BME IO functions that work like their counterparts in io.h.


SOUND
-----

int snd_init(unsigned mixrate, unsigned mixmode, unsigned bufferlength,
        unsigned channels, int usedirectsound);

        Initializes sound with the parameters given. bufferlength is the
        sound buffer length in milliseconds and mixmode is combined of
        MONO, STEREO, EIGHTBIT & SIXTEENBIT bit flags. mixrate is in hertz.
        channels is the maximum number of simultaneous sounds that can be
        mixed. (In the other functions, channels are numbered 0 - channels-1)

        usedirectsound is obsolete as of V1.27. SDL uses what it wants to
        use :)

        If stereo or 16-bit modes aren't supported, or mixrate isn't
        supported, snd_init() should "fall back" to 8-bit or mono modes.

        Sound will be mixed & played with SDL's audio callback
        mechanism so the "main program" doesn't have to worry about
        updating the sound buffer.

        If sound is re-initted without changing number of channels then all
        channels will continue playing.

        Minimum sound buffer size for glitch-free sound seems to depend on
        OS used and the soundcard drivers. Use 200ms or higher to be safe.

void snd_uninit(void);

        Shuts down the sound system. This is automatically registered by
        snd_init() as an atexit() function so you don't have to worry about
        this yourself.

void snd_setcustommixer(void (*custommixer)(Sint32 *dest, unsigned samples));

        Sets a custom mixer routine to be called when updating sound. The
        parameters are a pointer to a "clipping buffer" consisting of ints
        and the number of samples to calculate. (if snd_mixmode has the bit
        STEREO set, two ints need to be output for each sample)

        This function is only for those who know what they're doing and are
        implementing some kind of custom sound playing system (synthetic
        sounds perhaps.)

SAMPLE *snd_allocsample(int length);

        Allocate memory for a sample. Normally you don't need to call this.

void snd_freesample(SAMPLE *smp);

        Frees the sample.

void snd_playsample(SAMPLE *smp, unsigned chnum, unsigned frequency, unsigned char volume, unsigned char panning);

        Plays the sample on a channel with the given parameters. Volume is
        0-64 and panning is 0 (left) - 255 (right).

void snd_ipcorrect(SAMPLE *smp);

        Obsolete as of V1.27.

void snd_stopsample(unsigned chnum);

        Stops playing a sample.

void snd_preventdistortion(unsigned channels);

        Divides maximum mastervolume (255) with the amount of channels you
        are going to use, and sets it on all channels. This ensures that
        the sound output will not clip but will generally result in too
        quiet sound.

void snd_setmastervolume(unsigned chnum, unsigned char mastervol);

        Sets mastervolume on a channel. Mastervolume range is 0 (total silence)
        to 255 (maximum amplitude of the sound device)

void snd_setmusicmastervolume(unsigned musicchannels, unsigned char mastervol);

        Music is played on channels 0 - musicchannels-1. Sets mastervolume
        for them.

void snd_setsfxmastervolume(unsigned musicchannels, unsigned char mastervol);

        Sound effects can be played on channels musicchannels - channels-1.
        Sets mastervolume for them.

SAMPLE *snd_loadrawsample(char *name, int repeat, int end, unsigned char voicemode);

        Loads a 8bit signed data raw sample.

        Note for all sound/module loading functions from V1.21 onwards:
        Previously they didn't load anything if sound wasn't initialized first.
        But now this check is removed. So if you want to conserve memory when
        your program is in nosound-mode (check the variable snd_sndinitted,
        zero if sound is not initialized), don't call the loading functions :-)

SAMPLE *snd_loadwav(char *name);

        Loads a WAV sample. Stereo samples are converted to mono.

Next come the functions for XM loading/playing. MOD & S3M functions work
exactly the same way so they aren't explained.

int snd_loadxm(char *name);

        Loads an XM module.

void snd_freexm(void);

        Frees an XM module. The previous is automatically freed when loading
        a new module so you don't need to call this if you don't want to.

void snd_playxm(int pos);

        Starts playing from the position specified. This makes multi-songs
        possible in a single XM file. Playback will not start if sound was
        initted with fewer channels than in the XM.

void snd_stopxm(void);

        Stops XM playing.

unsigned char snd_getxmpos(void);

        Returns the current position.

unsigned char snd_getxmline(void);

        Returns the current pattern line.

unsigned char snd_getxmtick(void);

        Returns the current tick tempo counter.

unsigned char snd_getxmchannels(void);

        Returns number of channels used by the XM.

char *snd_getxmname(void);

        Returns name of the song.


WINDOW HANDLING & EVENTS
------------------------

int win_openwindow(char *appname, char *icon);

        Calls SDL_Init(), so this is the first thing you should call in 
        your program.

        win_fullscreen   - are graphics running in fullscreen mode
                           (default FALSE)
        win_quitted      - has the close button been pressed

void win_closewindow(void);

        Calls gfx_uninit().

void win_checkmessages(void);

        Checks SDL messages, like keyboard input, mouse move etc.
        win_getspeed() and kbd_waitkey() will always call this. If you
        spend a long time in your program without calling those functions
        it's a good idea to call this function once in a while.

        HINT: If you implement frame skipping in your program in such way:

                frameskip = win_getspeed(refreshrate);
                while (frameskip--) do_frame_movement();
                draw_everything();
                gfx_updatepage();

        then putting a call to win_checkmessages() into the beginning of
        do_frame_movement() will achieve smoother keyboard & mouse control on
        slower machines.

void win_messagebox(char *string);

        Doesn't do anything in V1.27.

int win_getspeed(int framerate);

        Gets the number of frames elapsed since this function was last
        called. Use this to synchronize to the screen updates and handle
        frameskipping.

        Before a loop (for example the game main loop) you should call this
        once to clear time accumulated before the loop.

        Note that this function keeps returning you nonzero frame count even
        if the application is minimized or inactive.

void win_setmousemode(int mode)

        This controls if the mouse cursor is visible or hidden in
        fullscreen & windowed modes.

        MOUSE_ALWAYS_VISIBLE - mouse cursor is visible in both fullscreen &
        windowed modes. 

        MOUSE_FULLSCREEN_HIDDEN - mouse cursor is hidden in fullscreen mode
        but visible in windowed mode. 

        MOUSE_ALWAYS_HIDDEN - mouse cursor is hidden in both modes.


GRAPHICS
--------

int gfx_init(unsigned xsize, unsigned ysize, unsigned framerate, unsigned flags);

        Initializes the 256 color graphics engine with the virtual screen 
        size and flags you specify. Framerate is obsolete as of V1.27

        Flags are:

        GFX_SCANLINES - Expand window size to 2x of the virtual screen size
        and make each other horizontal line blank for a TV-like feeling that's
        popular in emulators :-)

        GFX_DOUBLESIZE - Expand window size to 2x of the virtual screen size,
        doubling the pixels both horizontally and vertically. Be warned, this
        mode can increase CPU usage quite a lot because it requires the most
        accesses to the (slow) display adapter memory when updating the screen.

        GFX_FULLSCREEN - Initialize graphics in fullscreen mode.

        GFX_WINDOW - Initialize graphics in windowed mode.
        If neither of these is specified the last mode will be used. This is
        controlled by the variable win_fullscreen, and its default value
        at startup is 0 (windowed)

        GFX_NOSWITCHING - Disable automatic ALT-ENTER switching between
        windowed and fullscreen (this is taken care by the window procedure)

        In fullscreen mode there might not necessarily be a display mode that
        is exactly the size of the virtual screen. In this case SDL 
        displays a black border around the window.

void gfx_uninit(void);

        Frees all graphics engine resources.

int gfx_reinit(void);

        Re-initializes graphics engine with previous virtual screen size &
        mode. The window procedure uses this upon fullscreen/windowed
        switching.

void gfx_updatepage(void);

        Updates the screen to match the contents of the virtual 
        screen.

void gfx_blitwindow(void);

        Does nothing as of V1.27.

void gfx_setmaxcolors(int num);

        Sets number of colors in palettized modes. Call before calling
        gfx_init() for the first time. Does almost nothing as of V1.27.

int gfx_loadpalette(char *name);

        Loads an oldskool 768 byte palette. (can be created with the BMECONV
        utility)

        All 256 colors can be used, but color 0 is hard-coded to black and
        color 255 to white. 

void gfx_calcpalette(int fade, int radd, int gadd, int badd);

        Calculates palette with for gfx_setpalette(). All parameters
        are in the range 0-64.

void gfx_setpalette(void);

        Sets the palette.

void gfx_setclipregion(unsigned left, unsigned top, unsigned right, unsigned bottom);

        Sets clipping area for the drawing operations. Left and top signify
        the leftmost/topmost visible pixel while right & bottom signify the
        first invisible pixel. For example a fullscreen clipping area for a
        320x200 display is 0,0,320,200.

int gfx_loadblocks(char *name);

        Loads a block-file made with BMECONV.
        Blocks are meant to be used for background graphics, unlike sprites
        they don't have a hotspot (it's always 0,0).

int gfx_loadsprites(int num, char *name);

        Loads a spritefile made with BMECONV. There can be many spritefiles
        loaded at once, the number signifies this (default maximum is 256
        sprite files, numbers 0-255). See below how to increase this maximum
        value.

void gfx_freesprites(int num);

        Frees a spritefile.

void gfx_setmaxspritefiles(int num);

        Call this before the first call to gfx_loadsprites() if you want more
        or less spritefiles than the default (256). The value can be set only
        once during runtime, subsequent calls do nothing.

void gfx_drawblock(int x, int y, unsigned num);

        Draws a block. Block numbering starts from 1 and 0 is always an empty
        (& transparent) block.

void gfx_drawsprite(int x, int y, unsigned num);

        Draws a sprite. High 16 bits of "num" indicate spritefile number
        and low 16 bits indicate sprite frame within the spritefile (starting
        from 1)

void gfx_drawspritec(int x, int y, unsigned num, int color);

        Draws a sprite in a certain single color. Useful for example when
        making enemies "flash" while being hit in an oldskool action game.

void gfx_drawspritex(int x, int y, unsigned num, unsigned char *xlattable);

        Draws a sprite using a translation-table for colors. The translation
        table is an array of 256 bytes that maps each color to another. Note
        that this function is slower than the two above, but it's still quite
        useful for example drawing text with the same font in different colors.

void gfx_getspriteinfo(unsigned num);

        Returns sprite info in the following variables:
        extern int spr_xsize;
        extern int spr_ysize;
        extern int spr_xhotspot;
        extern int spr_yhotspot;

void gfx_fillscreen(int color);

        Fills the whole screen with a single color.

void gfx_plot(int x, int y, int color);

        Draws a point.

void gfx_line(int x1, int y1, int x2, int y2, int color);

        Draws a line. Supports full clipping!


KEYBOARD
--------

int kbd_init(void);

        Does nothing, returns always success.

void kbd_uninit(void);

        Does nothing.

int kbd_waitkey(void);

        Waits for a key and returns the SDL keycode. The key's state
        will be reset.

int kbd_getkey(void);

        Gets the SDL keycode if a key was pressed, otherwise returns zero.
        The key's state will be reset.

int kbd_checkkey(int rawcode);

        Checks if a key is down. Its state will also be reset.

int kbd_getascii(void);

        Get the latest ASCII code of a pressed key.
        Before a loop (for example a hiscore entry loop), you should
        call this once to clear the buffered character.

char *kbd_getkeyname(int rawcode);

        Get name for the key.

extern unsigned char win_keytable[];

        This is the keyboard state table for each key; zero is not
        pressed and nonzero pressed. kbd_checkkey(), kbd_getkey() etc.
        automatically zero entries of this table for key repeat.

extern unsigned char win_keystate[];

        This is a copy of the state table, that kbd_checkkey(), 
        kbd_getkey() etc. don't clear. Use this to check continuous 
        pressing of keys.


MOUSE
-----

int mou_init(void);

        Initializes the mouse system, basically just clears the mouse button
        status. Returns always success.

void mou_uninit(void);

        Does nothing. :-)

void mou_getpos(unsigned *x, unsigned *y);

        Gets current mouse position within the application window. It's always
        scaled to the size of the virtual screen so you don't have to make
        adjustments when using the doublesize-mode.

        In fullscreen mode the mouse pointer moves over the whole screen and
        the coordinates are once again scaled to the size of the virtual
        screen. However, this can introduce some distortion of the motion if
        the aspect ratios of virtual screen and the whole screen don't match.

        For accurate (and unlimited by the application window borders)
        mouse movement it is recommended to set the mouse cursor hidden and
        use only mou_getmove() function.

void mou_getmove(int *dx, int *dy);

        Gets relative mouse movement. Works by centering the (invisible)
        mouse pointer on the screen (or window) after each call and measuring
        the displacement from the center. Works only if the cursor is hidden
        (see win_setmousemode()), otherwise returns 0,0.

unsigned mou_getbuttons(void);

        Gets mouse button status. For the bit values, look at bme_main.h

        NOTE: In windowed mode there's a "bug" that if you press down a mouse
        button and release it outside the window the program will think the
        button is still down. By hiding the mouse cursor the cursor will be
        "trapped" inside the window and this "bug" never appears.


JOYSTICK
--------

int joy_detect(unsigned id);

        Detects presence of joystick (id ranging from 0 to n). As of V1.27,
        this is necessary before getting input from joystick: SDL requires
        joysticks to be "opened" first.

unsigned joy_getstatus(unsigned id, unsigned threshold);

        Returns joystick status (directions are "digital" too for true
        oldskool experience, look for bit values in bme_main.h)
        Threshold is for the directions, between 0-32768. (16384 in the
        halfway is quite good I think :-))


IO/DATAFILE
-----------

These are used by all other BME modules, so if you open a datafile
all BME functions will load the requested files from the datafile.
A datafile can be compiled with the DATAFILE program.

int io_open(char *name);
int io_lseek(int handle, int bytes, int whence);
int io_read(int handle, void *buffer, int size);
void io_close(int handle);

        work like the ones in io.h

int io_opendatafile(char *name);

        Opens a datafile. Data appended to an executable is no longer 
        supported.

int io_openlinkeddatafile(unsigned char *ptr)

        Opens a datafile from memory. Use DAT2INC to create an array out
        of your datafile, include the output in your program and then call
        
        io_openlinkeddatafile(datafile);

void io_setfilemode(int usedf);

        Use this to momentarily switch datafile use on/off. 1 is on and
        0 is off.


TEXT
----

void txt_print(int x, int y, unsigned spritefile, char *string);

        Prints text to coordinates x,y using the specified spritefile as fonts.
        The spritefile should contain 64 sprites which define the charset in
        the same order as C64 screen codes: Letters first (ascii codes
        64-95) and then numbers and other special characters (ascii codes
        32-63). Look at the included example FONTS.LBM to see how this is
        exactly done.

void txt_printcenter(int y, unsigned spritefile, char *string);

        Same as above, but centers the text automatically in X-direction.

void txt_printx(int x, int y, unsigned spritefile, char *string, unsigned char *xlattable);

        Like txt_print() but uses gfx_drawspritex() for color translation.

void txt_printcenterx(int y, unsigned spritefile, char *string, unsigned char *xlattable);

        Like txt_printcenter() but uses gfx_drawspritex() for color
        translation.

extern int txt_lastx, txt_lasty;

        Contain the X,Y position of last letter printed after calls to the
        above functions.

extern int txt_spacing;

        Amount of pixels to be added to the next letter's X position, in
        addition to the last letter's X size. By default this is -1.


BLOCK (TILE) MAP
----------------

int map_loadmap(char *name);

        Load a map created with MAPEDIT. Note: Palette and blocks aren't
        loaded automatically, you must load them yourself!

void map_freemap(void);

        Frees a map that has been loaded. map_loadmap() does this automatically
        when loading a new map.

int map_loadblockinfo(char *name);

        Loads a block-infofile (for background collision detection) created
        with INFEDIT.

void map_drawlayer(int l, int xpos, int ypos, int xorigin, int yorigin,
        int xblocks, int yblocks);

        Draws a layer (0-3) of the map you specify. xpos and ypos are the
        position on the map (in pixels), xorigin & yorigin are the starting
        place on screen (normally 0,0 but nonzero values can be used for
        splitscreen effects) and xblocks and yblocks are the number of blocks
        you want to be drawn in X and Y directions. For example on a 320x200
        display with 16x16 blocks, 21x14 blocks must be drawn to ensure all
        of the screen is covered.

void map_drawalllayers(int xpos, int ypos, int xorigin, int yorigin,
        int xblocks, int yblocks);

        Does the same as above, but for all layers.

unsigned char map_getblockinfo(int l, int xpos, int ypos);

        Gets blockinfobyte from layer l (0-3), map position xpos,ypos.
        What the blockinfobyte means, depends on you and the kind of BG
        collisions you want to detect. For example, I usually use this for
        platform games:

                Bit 0 - can walk on
                Bit 1 - is an obstacle
                Bit 2 - is a ladder
                Bit 3 - is a door

unsigned map_getblocknum(int l, int xpos, int ypos);

        Gets block number from layer l (0-3), map position xpos,ypos
        (in pixels.)

void map_setblocknum(int l, int xpos, int ypos, unsigned num);

        Sets block number to num at layer l (0-3), map position xpos,ypos
        (in pixels.)

void map_shiftblocksback(unsigned first, int amount, int step);

        Shifts blocks backwards in a circular fashion. first is the first block
        to be shifted, amount is the length of the shifting chain (in blocks)
        and step indicates by how many blocks they're shifted. Block 0 (empty)
        cannot be part of the shifting chain. This function doesn't actually
        move the block data around but just the pointers to blocks, so it can
        be used to achieve background animation fast & easily.

        An example: Let's say blocks 5-20 contain 4 frames of animation
        for some background "object" that consists of 4 blocks. The blocks
        are ordered this way:

                5-8   first frame
                9-12  second frame
                13-16 third frame
                17-20 fourth frame

        To animate this "object" correctly, map_shiftblocksback(5, 16, 4) would
        be used. (16 blocks, starting from 5, step 4)

void map_shiftblocksforward(unsigned first, int amount, int step);

        Works the same way as above, but shifts blocks forwards instead.
        Note that names are misleading because of the way animation works:
        when you have a block animation defined like in the example above, you
        need to call map_shitblocksback() to make the animation go forwards.


4. HOW TO USE THE UTILITIES
---------------------------

BMECONV
-------

BMECONV creates raw-bitmap, palette, block and spritedata files out of
LBM pictures.

Creating palettes and raw-bitmaps is quite straightforward. :-)

For blockfiles you just draw all the blocks in the picture, left to right,
up to down, without any spaces between them.

Blocks can use a transparent color whose number is specified on the command
line (default 252). Note that completely non-transparent blocks can be drawn
in an optimized manner so use them whenever possible!

For an example of blocks, look at TEST.LBM

For spritefiles you need to draw rectangles around the sprites
in the "rectangle color" that can be specified on the command line (default
254). BMECONV will sweep the picture left-right, top-bottom in search
of sprite rectangles and order them in that order (leftmost & topmost sprite
gets number 1, next on the right number 2 etc.)

Sprites have a hotspot as well which defines the "origin" of the sprite. This
can be marked on the sides of the sprite rectangle like in the picture below:
(r = rectangle color, h = hotspot color (command-line definable as wel, default
253) * = sprite pixels)

        r r r r r h r r r r
        r                 r
        r     * * * *     r
        r   * * * * * *   r
        r   * * * * * *   r
        h   * * * * * *   r
        r   * * * * * *   r
        r     * * * *     r
        r                 r
        r r r r r r r r r r

If a hotspot is not defined it is 0,0.

For an example of sprites, look at FONTS.LBM & EDITOR.LBM (sprites used by
MAPEDIT)


MAPEDIT & INFEDIT
-----------------

Press F10 in the programs for quite good online help. :-)
In INFEDIT the idea is that the leftmost 4x4 square represents bit 0 in the
blockinfodata, the next 4x4 square represents bit 1 and so on.

Note that regardless of the block size, INFEDIT currently supports only a
4x4 square. This will mean that each small square is 4 pixels for a 16x16
block, 8 pixels for 32x32 etc.


DAT2INC, DATAFILE
-----------------

These programs explain themselves quite well by running them without
parameters.


5. RECOMPILING BME WITH ANOTHER COMPILER
----------------------------------------

To do this, you must generate the makefile yourself. BME shouldn't 
depend on any evil things specific to GCC for example. Also, as of V1.27
it's 100% C code - no ASM modules at all.
 

6. VERSION HISTORY
------------------

V1.0    - Original release

V1.01   - gfx_line() didn't store EBX register, fixed
        - gfx_line() popped ESI & EDI in the wrong order, fixed
        - gfx_line() exited thru wrong exitpoint if the line was point-like
          and was outside the screen, fixed
        - Corrected clipping behaviour in gfx_line()
        - One extra palettechange when initting graphics; trying to make
          sure colors actually stay the way they should be in 8bpp mode
        - Fixed handling of long (>= 126 pixels) empty spriteslices in BMECONV

V1.02   - Added support for the STRETCHBLT doublesize mode, though it's
          generally quite slow (at least on NT4.0)
        - MAPEDIT and INFEDIT are now true windowed applications (no
          unnecessary console window opened anymore)

V1.05   - Scrapped the DIBSection code and changed to DirectDraw
        - Full 256 color palettes now supported (or at least 254)
        - Added fullscreen mode with automatic ALT-ENTER switching
          (automatic switching can also be disabled)
        - Added mou_setmode() to control visibility of mouse cursor in both
          windowed and fullscreen modes
        - Added mou_getmove() to get relative mouse movement
        - Added gfx_setmaxspritefiles() for dynamic allocation of the
          spritefile table
        - Added gfx_drawspritex() for drawing sprites with color translation
        - Fixed a text-positioning bug in MAPEDIT's help screen
        - Fixed the "window activation mouse click", it is ignored now
        - Changed snd_init() to just malloc() the sound buffers instead of
          GlobalAlloc().
        - Sound buffer length setting simplified: just the total length in
          milliseconds is told to snd_init() and it divides it into the
          required amount of 25ms fragments.
        - MAPEDIT & INFEDIT utilize the new functionality (fullscreen
          switching and relative mouse movement)

V1.06   - Added a balancing system to the sound buffer filling algorithm,
          trying to achieve even-sounding playback for fast repeating sound
          effects
        - Fixed windowed mode mouse movement when the window center point
          is outside the screen
        - Changed sound mixing to use a second thread instead of a callback
        - Brought back DIBSection support to gfx_init() as optional
        - gfx_init() will always place the program window topmost
        - Renamed mou_setmode() to win_setmousemode() to avoid unnecessary
          inclusion of mouse module to every program that uses graphics

V1.1    - Added support for LCC compiler
        - Added map_setblocknum()
        - Added map_shiftblocksback()
        - Added map_shiftblocksforward()
        - Fixed some limit checks in bme_map.c
        - Changed directory structure to include subdirectories
        - Changed all assembly code modules to NASM format
        - Changed sound engine to use 20ms buffer fragments instead
        - Removed the intense unrolling from the sound mixing inner loops.
          (there was about 10KB of them) This increases CPU use somewhat but
          reduces memory use: now each loaded sample doesn't need the 3KB
          safety buffer anymore
        - Removed some strange-named (heh) unused variables from bme_joy.c

V1.2    - Added DirectSound support
        - Added a warning of LCC bugs in the documentation; all utilities
          have now been compiled with BCC (as it was before) and there's no
          longer a precompiled library for LCC
        - Added read error checks to bme_gfx.c
        - Added a new utility, STRIPXM.EXE, which separates the song data and
          samples from an XM module, detecting any duplicate samples
        - Added better online-help to the commandline-utilities :-)
        - Fixed a crash in XM loading (pingpong loop in a zero-length sample)
        - Fixed window flicker in the BME example program; it was initting
          the graphics mode twice even when the first time (with DirectDraw)
          succeeded
        - Changed Waveout operation to be single-buffered
        - Changed sound update interval to 15ms
        - Changed the datafile identification string :-)
        - Removed some more vulgarities from the source code :-)

V1.21   - Added requirement for the DirectSound buffer to be a software buffer
          (because its contents will be constantly changing; any "upload" to
          soundcard memory is unacceptable)
        - Added the mixmode name table from JUDAS2.05y to BMEPLAY; it now
          displays the actual mixrate & mixmode in use
        - Changed some error codes for DirectDraw & DirectSound: now
          the error is BME_OPEN_ERROR if the DLL cannot be opened or the
          interface cannot be created, and BME_GRAPHICS_ERROR / BME_SOUND_ERROR
          in other cases.
        - Changed sound update interval back to 20ms
        - Removed all mixrate checks & adjustments from snd_init()
        - Removed checks for sound initialization from all snd_loadsomething()
          functions
        - Removed the error code BME_OUT_OF_CHANNELS; channel amount check
          moved from XM/S3M/MOD load functions to XM/S3M/MOD play functions
        - Removed WAVE_ALLOWSYNC from the WaveOut open call, because
          it really isn't what we want :-)
        - Removed some nonexistant variables from bme_snd.h

V1.22   - Added MINGW support
        - Added directory path stripping to DATAFILE.EXE: there is only room
          for 8+3 chars in the datafile's fileheaders. Now files can be
          collected to the datafile from different subdirectories, without
          problems
        - Added a reminder of the 8+3 chars filename limit to DATAFILE.EXE's
          usage text
        - Brought back the LCC compiled library
        - Fixed a memory leak in DATAFILE.EXE
        - Changed DirectDraw fullscreen mode palette to have PC_NOCOLLAPSE
          flag set in every color; don't think this is actually needed but
          doesn't hurt anyway
        - Moved the variable win_hwnd to bme.c to be sure any window/graphics
          code isn't included in pure sound-only applications (for example
          BMEPLAY.EXE)

V1.23   - Added the function gfx_setmaxcolors() to control palette size in
          8-bit screen modes
        - Added configurable text spacing
        - Added a gfx_updatepage() call in gfx_init(), to prevent random memory
          data displayed in windowed mode before main program's first call
          to gfx_updatepage()
        - Added wildcard support to DATAFILE.EXE (now there can be wildcards
          in the filelist-file)
        - Fixed crash related to re-entry of gfx_uninit() & gfx_init() when
          switching from fullscreen to windowed mode manually

V1.24   - Added kbd_getvirtualkey()
        - Icon can be specified in win_openwindow()

V1.25   - Added snd_setcustommixer()

V1.26   - Sound mixing thread exit check done in a different way -> no more
          lockup on my W2K system.

V1.27   - BME goes cross-platform by using the SDL library!
        - No ASM code anymore
        - Should be endian-independent
        - Removed EXELINK utility, replaced with DAT2INC for more portability
          (no binary data appended to executable)
        - Removed STRIPXM utility
        - Removed the example program :)

V1.28   - Uses the BSD license
        - Added some numeric keypad key codes to bme_main.h

V1.29   - Changes by Kalle Niemitalo integrated

V1.3    - endian.h renamed to fileio.h for possible clashes with system 
          include files
        - ifndef guards added to include files

V1.31   - Fixed handling of raw keycodes over 511 (ignored for now for the key
          state, still readable from kbd_getvirtualkey)   

