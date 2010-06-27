/*
 * BOFH EDITOR
 * -----------
 */

#include "bofh.h"

#define SPR_EDITORFONT 0
#define SPR_EDITOR 12
#define MAX_LAYERS 4
#define MINXSIZE 20
#define MINYSIZE 13

#define MAXMSGROW 10

#define MODE_MAPEDIT 0
#define MODE_INFEDIT 1
#define MODE_ACTOREDIT 2
#define MODE_RANDOMACTOREDIT 3
#define MODE_STAIREDIT 4
#define MODE_CLOSETEDIT 5
#define MODE_LIFTEDIT 6
#define MODE_PARAMEDIT 7
#define MODE_MESSAGEEDIT 8

typedef struct
{
        int x;
        int minvalue;
        int maxvalue;
        int *address;
        int special;
} PARAMCOLUMN;

typedef struct
{
        int y;
        int columns;
        PARAMCOLUMN column[5];
} PARAMROW;

typedef struct
{
	int xb;
	int yb;
	int type;
} RANDOMLOCATION;

typedef struct
{
	int xb;
	int yb;
	int type;
	int angle;
} EXACTLOCATION;

char *spritename[] = {
	"smallfnt.spr","player.spr", "weapon.spr", "machine.spr",
        "fistman.spr", "gunman.spr", "shotgman.spr", "uziman.spr",
        "tech.spr", "closet.spr", "leader.spr", "sadist.spr",
        "editor.spr", NULL
};

EXACTLOCATION eloc[MAX_ACTOR];
RANDOMLOCATION rloc[MAX_ACTOR];

int mousemovex = 0, mousemovey = 0;
int mouseinitted = 0;
int mousex = 160, mousey = 100, mouseb = 0, prevmouseb = 0;
int mark = 0;
int copybuffer = 0;
int speed;
int messagenum = 0;
int messagerow = 0;
int messagecolumn = 0;
int messageflash = 0;
int copybufferx, copybuffery;
int markx1, marky1, markx2, marky2;
int scrolllock = 0;
int screenmode = 0;
int layermenusel = 0;
int liftcursorpos = 0;
int blockx, blocky;
int xpos = 0, ypos = 0;
int paramx = 0, paramy = 0;
int paramflash = 0;
int key;
int blk = 0;
int cl = 0;
int clo = 0;
int clift = 0;
int actor = ACTOR_FIRSTACTOR;
int ractor = ACTOR_CAT5;
int transparent = 0;
int stairindex = -1;
int enterclosetname = 0;
int enternameflash;
int numrloc;
int layervisible[4] = {1,1,0,0};
int mode = MODE_MAPEDIT;
char printbuffer[80];
char missionname[13];
char infcopybuffer[16];

int randomcat5 = 0;
int randombnd = 0;
int randomdrill = 0;
int randomcrossbow = 0;
int randompistol = 0;
int randomshotgun = 0;
int randomuzi = 0;
int randomgrenade = 0;
int randombazooka = 0;
int randomscanner = 0;
int randommedikit = 0;
int randomenemy = 0;
int randomleader = 0;

int randomangle = 0;

char *actorname[] = {
"NONE",
"BOFH (DON'T USE)",
"MUZZLE (DON'T USE)",
"BULLET (DON'T USE)",
"SMOKE (DON'T USE)",
"ACER WORKSTATION",
"POMI WORKSTATION",
"SYSTECH WORKSTATION",
"IBM THINKPAD LAPTOP",
"DOCKING STATION",
"LASER PRINTER",
"INKJET PRINTER",
"BIG NETWORK PRINTER",
"CD TOWER",
"SUN SERVER",
"COMPAQ SERVER",
"EXPLOSION (DON'T USE)",
"FISTHIT (DON'T USE)",
"WHIPHIT (DON'T USE)",
"CHUNK (DON'T USE)",
"SPARK (DON'T USE)",
"BLOOD",
"FISTMAN",
"PISTOLMAN",
"SHOTGUNMAN",
"UZIMAN",
"TECHNICIAN",
"SADIST",
"LEADER",
"DEAD FISTMAN",
"DEAD BOFH (DON'T USE)",
"DEAD PISTOLMAN",
"DEAD SHOTGUNMAN",
"DEAD UZIMAN",
"DEAD TECHNICIAN",
"DEAD LEADER",
"DEAD SADIST",
"CAT-5 WHIP",
"PISTOL",
"SHOTGUN",
"UZI",
"SMALL MEDIKIT",
"BIG MEDIKIT",
"INSTRUCTIONS (DON'T USE)",
"DRILL",
"DRILL HIT (DON'T USE)",
"BAZOOKA",
"KEYBOARD",
"MULTITRACK",
"VIDEOTAPE RECORDER + MONITOR",
"SHELL",
"SHOTGSHELL",
"RICOCHET",
"TRAP BEAM (TO PROTECT THE SERVER ROOM)",
"FLAME (DON'T USE)",
"SHARD (DON'T USE)",
"FLYING GRENADE (DON'T USE)",
"GRENADE",
"BAZOOKA PROJECTILE (DON'T USE)",
"BAZOOKA STRAP",
"BAZOOKA USED",
"CROSSBOW",
"ARROW (DON'T USE)",
"SCANNER"};

short *temparea;

int numstairs = 0;
int numclosets = 0;
int numlifts = 0;
int numbombs = 0;
int victorybits = 1;
int firstdamagedblock = 1;

int bofhstartx = 10, bofhstarty = 6, bofhstartangle = 0;
int srminx = 0, srminy = 0, srmaxx = 19, srmaxy = 12;

int exactactors;
int randomactors;
int computers;
int mouseoveractor;
int mouseoverractor;
int mouseoverstair;

int begintime[MAX_DIFF] = {0,0,0,0,0};
int beginbombs[MAX_DIFF] = {0,0,0,0,0};
int beginterrorists[MAX_DIFF] = {0,0,0,0,0};
int beginpistolmen[MAX_DIFF] = {0,0,0,0,0};
int beginshotgunmen[MAX_DIFF] = {0,0,0,0,0};
int beginuzimen[MAX_DIFF] = {0,0,0,0,0};
int beginsadists[MAX_DIFF] = {0,0,0,0,0};
int beginleaders[MAX_DIFF] = {0,0,0,0,0};
int beginwhips[MAX_DIFF] = {0,0,0,0,0};
int begindrills[MAX_DIFF] = {0,0,0,0,0};
int begincrossbows[MAX_DIFF] = {0,0,0,0,0};
int beginpistols[MAX_DIFF] = {0,0,0,0,0};
int beginshotguns[MAX_DIFF] = {0,0,0,0,0};
int beginuzis[MAX_DIFF] = {0,0,0,0,0};
int begingrenades[MAX_DIFF] = {0,0,0,0,0};
int beginbazookas[MAX_DIFF] = {0,0,0,0,0};
int beginscanners[MAX_DIFF] = {0,0,0,0,0};
int beginsmallmedikits[MAX_DIFF] = {0,0,0,0,0};
int beginbigmedikits[MAX_DIFF] = {0,0,0,0,0};

int fistpr[] = {0, 14, 16, 18, 20};
int pistolpr[] = {0, 2, 3, 3, 4};
int shotgunpr[] = {0, 1, 1, 2, 2};
int uzipr[] = {0, 1, 1, 2, 3};
int crossbowpr[] = {0, 1, 1, 2, 3};
int flamepr[] = {0, 2, 3, 4, 5};

FILE *mf;

CLOSET closet[MAX_CLOSET];
LIFT lift[MAX_LIFT];
STAIRS stairs[MAX_STAIRS];
char briefingtext[MAXBRIEFINGLENGTH];
char victorytext[MAXBRIEFINGLENGTH];

PARAMROW paramrow[] = {
        {0, 3, {{160, 0, 519, &bofhstartx, 0},
              {190, 0, 519, &bofhstarty, 0},
              {220, 0, 1023, &bofhstartangle, 0}}},
        {6, 2, {{160, 0, 519, &srminx, 0},
                {190, 0, 519, &srminy, 0}}},
        {12, 2, {{160, 0, 519, &srmaxx, 0},
                 {190, 0, 519, &srmaxy, 0}}},
        {18, 1, {{160, 1, 15, &victorybits, 1}}},
        {36, 5, {{160, 0, 99, &begintime[0], 0},
                {190, 0, 99, &begintime[1], 0},
                {220, 0, 99, &begintime[2], 0},
                {250, 0, 99, &begintime[3], 0},
                {280, 0, 99, &begintime[4], 0}}},
        {42, 5, {{160, 0, 64, &beginbombs[0], 0},
                {190, 0, 64, &beginbombs[1], 0},
                {220, 0, 64, &beginbombs[2], 0},
                {250, 0, 64, &beginbombs[3], 0},
                {280, 0, 64, &beginbombs[4], 0}}},
        {48, 5, {{160, 0, 2048, &beginterrorists[0], 0},
                {190, 0, 2048, &beginterrorists[1], 0},
                {220, 0, 2048, &beginterrorists[2], 0},
                {250, 0, 2048, &beginterrorists[3], 0},
                {280, 0, 2048, &beginterrorists[4], 0}}},
        {54, 5, {{160, 0, 2048, &beginpistolmen[0], 0},
                {190, 0, 2048, &beginpistolmen[1], 0},
                {220, 0, 2048, &beginpistolmen[2], 0},
                {250, 0, 2048, &beginpistolmen[3], 0},
                {280, 0, 2048, &beginpistolmen[4], 0}}},
        {60, 5, {{160, 0, 2048, &beginshotgunmen[0], 0},
                {190, 0, 2048, &beginshotgunmen[1], 0},
                {220, 0, 2048, &beginshotgunmen[2], 0},
                {250, 0, 2048, &beginshotgunmen[3], 0},
                {280, 0, 2048, &beginshotgunmen[4], 0}}},
        {66, 5, {{160, 0, 2048, &beginuzimen[0], 0},
                {190, 0, 2048, &beginuzimen[1], 0},
                {220, 0, 2048, &beginuzimen[2], 0},
                {250, 0, 2048, &beginuzimen[3], 0},
                {280, 0, 2048, &beginuzimen[4], 0}}},
        {72, 5, {{160, 0, 2048, &beginsadists[0], 0},
                {190, 0, 2048, &beginsadists[1], 0},
                {220, 0, 2048, &beginsadists[2], 0},
                {250, 0, 2048, &beginsadists[3], 0},
                {280, 0, 2048, &beginsadists[4], 0}}},
        {78, 5, {{160, 0, 2048, &beginleaders[0], 0},
                {190, 0, 2048, &beginleaders[1], 0},
                {220, 0, 2048, &beginleaders[2], 0},
                {250, 0, 2048, &beginleaders[3], 0},
                {280, 0, 2048, &beginleaders[4], 0}}},
        {84, 5, {{160, 0, 2048, &beginwhips[0], 0},
                {190, 0, 2048, &beginwhips[1], 0},
                {220, 0, 2048, &beginwhips[2], 0},
                {250, 0, 2048, &beginwhips[3], 0},
                {280, 0, 2048, &beginwhips[4], 0}}},
        {90, 5, {{160, 0, 2048, &begindrills[0], 0},
                {190, 0, 2048, &begindrills[1], 0},
                {220, 0, 2048, &begindrills[2], 0},
                {250, 0, 2048, &begindrills[3], 0},
                {280, 0, 2048, &begindrills[4], 0}}},
        {96, 5, {{160, 0, 2048, &begincrossbows[0], 0},
                {190, 0, 2048, &begincrossbows[1], 0},
                {220, 0, 2048, &begincrossbows[2], 0},
                {250, 0, 2048, &begincrossbows[3], 0},
                {280, 0, 2048, &begincrossbows[4], 0}}},
        {102, 5, {{160, 0, 2048, &beginpistols[0], 0},
                {190, 0, 2048, &beginpistols[1], 0},
                {220, 0, 2048, &beginpistols[2], 0},
                {250, 0, 2048, &beginpistols[3], 0},
                {280, 0, 2048, &beginpistols[4], 0}}},
        {108, 5, {{160, 0, 2048, &beginuzis[0], 0},
                {190, 0, 2048, &beginuzis[1], 0},
                {220, 0, 2048, &beginuzis[2], 0},
                {250, 0, 2048, &beginuzis[3], 0},
                {280, 0, 2048, &beginuzis[4], 0}}},
        {114, 5, {{160, 0, 2048, &begingrenades[0], 0},
                {190, 0, 2048, &begingrenades[1], 0},
                {220, 0, 2048, &begingrenades[2], 0},
                {250, 0, 2048, &begingrenades[3], 0},
                {280, 0, 2048, &begingrenades[4], 0}}},
        {120, 5, {{160, 0, 2048, &beginbazookas[0], 0},
                {190, 0, 2048, &beginbazookas[1], 0},
                {220, 0, 2048, &beginbazookas[2], 0},
                {250, 0, 2048, &beginbazookas[3], 0},
                {280, 0, 2048, &beginbazookas[4], 0}}},
        {126, 5, {{160, 0, 2048, &beginscanners[0], 0},
                {190, 0, 2048, &beginscanners[1], 0},
                {220, 0, 2048, &beginscanners[2], 0},
                {250, 0, 2048, &beginscanners[3], 0},
                {280, 0, 2048, &beginscanners[4], 0}}},
        {132, 5, {{160, 0, 2048, &beginsmallmedikits[0], 0},
                {190, 0, 2048, &beginsmallmedikits[1], 0},
                {220, 0, 2048, &beginsmallmedikits[2], 0},
                {250, 0, 2048, &beginsmallmedikits[3], 0},
                {280, 0, 2048, &beginsmallmedikits[4], 0}}},
        {138, 5, {{160, 0, 2048, &beginbigmedikits[0], 0},
                {190, 0, 2048, &beginbigmedikits[1], 0},
                {220, 0, 2048, &beginbigmedikits[2], 0},
                {250, 0, 2048, &beginbigmedikits[3], 0},
                {280, 0, 2048, &beginbigmedikits[4], 0}}},
        {150, 5, {{160, 0, 64, &fistpr[0], 0},
                {190, 0, 64, &fistpr[1], 0},
                {220, 0, 64, &fistpr[2], 0},
                {250, 0, 64, &fistpr[3], 0},
                {280, 0, 64, &fistpr[4], 0}}},
        {156, 5, {{160, 0, 64, &pistolpr[0], 0},
                {190, 0, 64, &pistolpr[1], 0},
                {220, 0, 64, &pistolpr[2], 0},
                {250, 0, 64, &pistolpr[3], 0},
                {280, 0, 64, &pistolpr[4], 0}}},
        {162, 5, {{160, 0, 64, &shotgunpr[0], 0},
                {190, 0, 64, &shotgunpr[1], 0},
                {220, 0, 64, &shotgunpr[2], 0},
                {250, 0, 64, &shotgunpr[3], 0},
                {280, 0, 64, &shotgunpr[4], 0}}},
        {168, 5, {{160, 0, 64, &uzipr[0], 0},
                {190, 0, 64, &uzipr[1], 0},
                {220, 0, 64, &uzipr[2], 0},
                {250, 0, 64, &uzipr[3], 0},
                {280, 0, 64, &uzipr[4], 0}}},
        {174, 5, {{160, 0, 64, &crossbowpr[0], 0},
                {190, 0, 64, &crossbowpr[1], 0},
                {220, 0, 64, &crossbowpr[2], 0},
                {250, 0, 64, &crossbowpr[3], 0},
                {280, 0, 64, &crossbowpr[4], 0}}},
        {180, 5, {{160, 0, 64, &flamepr[0], 0},
                {190, 0, 64, &flamepr[1], 0},
                {220, 0, 64, &flamepr[2], 0},
                {250, 0, 64, &flamepr[3], 0},
                {280, 0, 64, &flamepr[4], 0}}},
        {192, 1, {{160, 1, 65535, &firstdamagedblock, 0}}}};

int editmain(void);
int geteditspeed(void);
void mainloop(void);
void mapedithelp(void);
void infedithelp(void);
void actoredithelp(void);
void randomactoredithelp(void);
void stairedithelp(void);
void closetedithelp(void);
void liftedithelp(void);
void paramedithelp(void);
void messageedithelp(void);
int ownloadmap(char *name);
int enterfilename(char *text, char *buffer);
void printstatus(void);
void movecursor(void);
void selectblock(void);
void mapeditcommands(void);
void infeditcommands(void);
void actoreditcommands(void);
void randomactoreditcommands(void);
void staireditcommands(void);
void closeteditcommands(void);
void lifteditcommands(void);
void parameditcommands(void);
void messageeditcommands(void);
void getblockxy(void);
void scrollmap(void);
void resizelayer(int l, int newxsize, int newysize);
void layeroptions(void);
void loadmission(void);
void savemission(void);
int initstuff(void);
void initmap(void);
int getkey_noshift(void);
void updatemouse(void);
void drawactor(int xp, int yp, int type, int angleturn);
int openmf(char *missionfilename);
int mfreadstring(char *dest, int length);
void closemf(void);
int mfreadint(void);
void readrloc(int type);
void writerloc(int type, FILE* handle);
void write8(FILE *handle, unsigned data);
void writele16(FILE *handle, unsigned data);
void writele32(FILE *handle, unsigned data);

int main(int argc, char *argv[])
{
  if (!initstuff()) return 666;
  memset(missionname, 0, 13);
  initmap();
  mainloop();
  return 0;
}

void mainloop(void)
{
  gfx_calcpalette(64, 0, 0, 0);
  gfx_setpalette();
  updatemouse();

  for (;;)
  {
    speed = geteditspeed();

    key = getkey_noshift();

    updatemouse();
    if (key == KEY_ESC) break;
    if (key == KEY_F1) mode = MODE_MAPEDIT;
    if (key == KEY_F2) mode = MODE_INFEDIT;
    if (key == KEY_F3) mode = MODE_ACTOREDIT;
    if (key == KEY_F4) mode = MODE_RANDOMACTOREDIT;
    if (key == KEY_F5)
    {
        mode = MODE_STAIREDIT;
        stairindex = -1;
    }
    if (key == KEY_F6)
    {
        mode = MODE_CLOSETEDIT;
        enterclosetname = 0;
    }
    if (key == KEY_F7) mode = MODE_LIFTEDIT;
    if (key == KEY_F8) mode = MODE_PARAMEDIT;
    if (key == KEY_F9) mode = MODE_MESSAGEEDIT;

    if (key == KEY_F11) loadmission();
    if (key == KEY_F12) savemission();
    if (key == KEY_SCROLLLOCK) scrolllock ^= 1;
    movecursor();

    switch(mode)
    {
      case MODE_MAPEDIT:
      scrollmap();
      getblockxy();
      selectblock();
      mapeditcommands();
      break;

      case MODE_INFEDIT:
      scrollmap();
      getblockxy();
      selectblock();
      infeditcommands();
      break;

      case MODE_ACTOREDIT:
      scrollmap();
      getblockxy();
      actoreditcommands();
      break;

      case MODE_RANDOMACTOREDIT:
      scrollmap();
      getblockxy();
      randomactoreditcommands();
      break;

      case MODE_STAIREDIT:
      scrollmap();
      getblockxy();
      staireditcommands();
      break;

      case MODE_CLOSETEDIT:
      scrollmap();
      getblockxy();
      closeteditcommands();
      break;

      case MODE_LIFTEDIT:
      scrollmap();
      getblockxy();
      lifteditcommands();
      break;

      case MODE_PARAMEDIT:
      scrollmap();
      getblockxy();
      parameditcommands();
      break;

      case MODE_MESSAGEEDIT:
      scrollmap();
      getblockxy();
      messageeditcommands();
      break;
    }
    win_asciikey = 0;

    transparent ^= 1;
    if (mode < MODE_PARAMEDIT)
    {
            gfx_fillscreen(6); // replaced transparent with 6 as it caused flashing
            if (layervisible[0]) map_drawlayer(0,xpos,ypos,0,0,21,14);
            if (layervisible[1]) map_drawlayer(1,xpos,ypos,0,0,21,14);
    }
    else gfx_fillscreen(0);

    printstatus();
    gfx_drawsprite(mousex, mousey, 0x000c0001);
    gfx_updatepage();
  }
}

void selectblock(void)
{
  /* Block selecting */
  if (key == KEY_Z) blk--;
  if (key == KEY_X) blk++;
  if (key == KEY_A) blk -= 10;
  if (key == KEY_S) blk += 10;
  if (blk < 0) blk = 0;
  if (blk > gfx_nblocks) blk = gfx_nblocks;
}

void messageeditcommands(void)
{
        char *textptr;
        int cursorpos;
        unsigned char ascii = kbd_getascii();
        ascii = toupper(ascii);

        if (key == KEY_TAB)
        {
                messagenum ^= 1;
                messagerow = 0;
        }

        if (key == KEY_DOWN) messagerow++;
        if (messagerow >= MAXMSGROW) messagerow = MAXMSGROW-1;

        if (key == KEY_UP) messagerow--;
        if (messagerow < 0) messagerow = 0;

        if (!messagenum) textptr = briefingtext + messagerow * 80;
        else textptr = victorytext + messagerow * 80;
        cursorpos = strlen(textptr);
        messagecolumn = cursorpos;

        if (ascii == 8)
        {
              if (cursorpos)
              {
                textptr[cursorpos-1] = 0;
              }
        }
        if (ascii == 13)
        {
                messagerow++;
                if (messagerow >= MAXMSGROW) messagerow = 0;
        }
        if ((ascii >= 32) && (cursorpos < 40))
        {
                textptr[cursorpos] = ascii;
        }
        messageflash++;
        if (key == KEY_F10) messageedithelp();
}

void parameditcommands(void)
{
        int maxrow = sizeof paramrow / sizeof(PARAMROW);

        if (key == KEY_DOWN)
        {
                paramy++;
                if (paramy >= maxrow) paramy = 0;
        }
        if (key == KEY_UP)
        {
                paramy--;
                if (paramy < 0) paramy = maxrow-1;
        }
        if (key == KEY_RIGHT) paramx++;
        if (key == KEY_LEFT) paramx--;
        if (paramx >= paramrow[paramy].columns) paramx = 0;
        if (paramx < 0) paramx = paramrow[paramy].columns-1;

        if (key == KEY_Z) (*paramrow[paramy].column[paramx].address)--;
        if (key == KEY_X) (*paramrow[paramy].column[paramx].address)++;
        if (key == KEY_A) (*paramrow[paramy].column[paramx].address) -= 10;
        if (key == KEY_S) (*paramrow[paramy].column[paramx].address) += 10;

        if (*paramrow[paramy].column[paramx].address >
             paramrow[paramy].column[paramx].maxvalue)
            *paramrow[paramy].column[paramx].address =
             paramrow[paramy].column[paramx].maxvalue;

        if (*paramrow[paramy].column[paramx].address <
             paramrow[paramy].column[paramx].minvalue)
            *paramrow[paramy].column[paramx].address =
             paramrow[paramy].column[paramx].minvalue;

        paramflash++;
        if (key == KEY_F10) paramedithelp();
}

void lifteditcommands(void)
{
  /* Lift selecting */
  int req = -1;
  if (key == KEY_Z) clift--;
  if (key == KEY_X) clift++;
  if (key == KEY_A) clift -= 10;
  if (key == KEY_S) clift += 10;
  if (clift < 0) clift = 0;
  if (clift >= MAX_LIFT) clift = MAX_LIFT-1;

  if (key == KEY_DOWN) liftcursorpos++;
  if (key == KEY_UP) liftcursorpos--;
  if (liftcursorpos < 0) liftcursorpos = 3;
  if (liftcursorpos > 3) liftcursorpos = 0;
  if (!lift[clift].floors) liftcursorpos = 0;

  if (key == KEY_RIGHT)
  {
        switch(liftcursorpos)
        {
                case 0:
                if (lift[clift].floors + lift[clift].firstfloor < MAX_FLOOR)
                {
                        lift[clift].floors++;
                }
                break;

                case 1:
                if (lift[clift].floors + lift[clift].firstfloor < MAX_FLOOR)
                {
                        lift[clift].firstfloor++;
                }
                break;

                case 2:
                lift[clift].speed++;
                break;

                case 3:
                lift[clift].startdelay += 5;
                break;
        }
  }

  if (key == KEY_LEFT)
  {
        switch(liftcursorpos)
        {
                case 0:
                if (lift[clift].floors > 0)
                {
                        lift[clift].floors--;
                }
                break;

                case 1:
                if (lift[clift].firstfloor > 0)
                {
                        lift[clift].firstfloor--;
                }
                break;

                case 2:
                if (lift[clift].speed > 0) lift[clift].speed--;
                break;

                case 3:
                if (lift[clift].startdelay > 0) lift[clift].startdelay -= 5;
                break;
        }
  }
        if (key == KEY_0) req = 0;
        if (key == KEY_1) req = 1;
        if (key == KEY_2) req = 2;
        if (key == KEY_3) req = 3;
        if (key == KEY_4) req = 4;
        if (key == KEY_5) req = 5;
        if (key == KEY_6) req = 6;
        if (key == KEY_7) req = 7;
        if (key == KEY_8) req = 8;
        if (key == KEY_9) req = 9;
        req -= lift[clift].firstfloor;

        if ((req >= 0) && (req < lift[clift].floors))
        {
                lift[clift].liftfloor[req].xb = blockx;
                lift[clift].liftfloor[req].yb = blocky;
        }
        if (key == KEY_F10) liftedithelp();
}

void closeteditcommands(void)
{
  int c;

  /* Closet selecting */
  if (!enterclosetname)
  {
  if (key == KEY_Z) clo--;
  if (key == KEY_X) clo++;
  if (key == KEY_A) clo -= 10;
  if (key == KEY_S) clo += 10;
  if (clo < 0) clo = 0;
  if (clo >= MAX_CLOSET) clo = MAX_CLOSET-1;

  if (mouseb & 1)
  {
        closet[clo].xb = blockx;
        closet[clo].yb = blocky;
  }

  if ((mouseb & 2) && (!prevmouseb))
  {
        if ((blockx == closet[clo].xb) &
           (blocky == closet[clo].yb))
        {
                closet[clo].xb = 0;
                closet[clo].yb = 0;
                closet[clo].anim = 0;
                memset(closet[clo].name, 0, 16);
                for (c = 0; c < MAX_EQUIP; c++) closet[clo].equipment[c] = EQUIP_NONE;
        }
  }

  if ((closet[clo].xb) && (closet[clo].yb))
  {
  if (key == KEY_G)
  {
        xpos = closet[clo].xb*16-160;
        ypos = closet[clo].yb*16-100;
  }

  if (key == KEY_F)
  {
        closet[clo].anim ^= 1;
  }

  if (key == KEY_1)
  {
        closet[clo].equipment[0]++;
        closet[clo].equipment[0] %= 6;
  }
  if (key == KEY_2)
  {
        closet[clo].equipment[1]++;
        closet[clo].equipment[1] %= 6;
  }
  if (key == KEY_3)
  {
        closet[clo].equipment[2]++;
        closet[clo].equipment[2] %= 6;
  }
  if (key == KEY_4)
  {
        closet[clo].equipment[3]++;
        closet[clo].equipment[3] %= 6;
  }
  if (key == KEY_5)
  {
        closet[clo].equipment[4]++;
        closet[clo].equipment[4] %= 6;
  }
  if (key == KEY_6)
  {
        closet[clo].equipment[5]++;
        closet[clo].equipment[5] %= 6;
  }
  if (key == KEY_7)
  {
        closet[clo].equipment[6]++;
        closet[clo].equipment[6] %= 6;
  }
  if (key == KEY_8)
  {
        closet[clo].equipment[7]++;
        closet[clo].equipment[7] %= 6;
  }
  if (key == KEY_9)
  {
        closet[clo].equipment[8]++;
        closet[clo].equipment[8] %= 6;
  }
  if (key == KEY_N) enterclosetname = 1;
  }
  }
  else
  {
        unsigned char ascii = kbd_getascii();
        int cursorpos = strlen(closet[clo].name);
        ascii = toupper(ascii);

        if (ascii == 8)
        {
              if (cursorpos)
              {
                closet[clo].name[cursorpos-1] = 0;
              }
        }
        if (ascii == 13)
        {
                enterclosetname = 0;
        }
        if ((ascii >= 32) && (ascii < 96) && (cursorpos < 15))
        {
                closet[clo].name[cursorpos] = ascii;
        }
        enternameflash++;
  }

  if (key == KEY_F10) closetedithelp();
}

void mapeditcommands(void)
{
  /* General commands */
  if (key == KEY_1)
  {
    cl = 0;
    mark = 0;
  }
  if (key == KEY_2)
  {
    cl = 1;
    mark = 0;
  }
  if ((map_layer[cl].xsize) && (map_layer[cl].ysize))
  {
    if (mouseb & 1)
    {
      map_layerdataptr[cl][blocky*map_layer[cl].xsize+blockx] = blk;
    }
    if (key == KEY_G)
    {
      blk = map_layerdataptr[cl][blocky*map_layer[cl].xsize+blockx];
    }
    if (key == KEY_T)
    {
      int x,y;
      if (copybuffer)
      {
        int c = 0;
        /* Paste will perform differently if wrapping is in use */
        if (map_layer[cl].xwrap & map_layer[cl].ywrap)
        {
          /* Wrap version */
          for (y = blocky; y < blocky+copybuffery; y++)
          {
            for (x = blockx; x < blockx+copybufferx; x++)
            {
              int realx = x % map_layer[cl].xsize;
              int realy = y % map_layer[cl].ysize;
              map_layerdataptr[cl][realy*map_layer[cl].xsize+realx] = temparea[c];
              c++;
            }
          }
        }
        else
        {
          /* Nonwrap version */
          for (y = blocky; y < blocky+copybuffery; y++)
          {
            for (x = blockx; x < blockx+copybufferx; x++)
            {
              if ((x < map_layer[cl].xsize) && (y < map_layer[cl].ysize))
              {
                map_layerdataptr[cl][y*map_layer[cl].xsize+x] = temparea[c];
                c++;
              }
            }
          }
        }
      }
    }

    if ((mouseb & 2) && (!(prevmouseb & 2)))
    {
      switch (mark)
      {
        case 0:
        markx1 = blockx;
        markx2 = blockx;
        marky1 = blocky;
        marky2 = blocky;
        mark = 1;
        break;

        case 1:
        markx2 = blockx;
        marky2 = blocky;
        if ((markx2 >= markx1) && (marky2 >= marky1)) mark = 2;
        else mark = 0;
        break;

        case 2:
        mark = 0;
        break;
      }
    }
    if (markx1 > map_layer[cl].xsize) mark = 0;
    if (markx2 > map_layer[cl].xsize) mark = 0;
    if (marky1 > map_layer[cl].ysize) mark = 0;
    if (marky2 > map_layer[cl].ysize) mark = 0;
    if (mark == 2)
    {
      int x,y;
      if (key == KEY_F)
      {
        for (y = marky1; y <= marky2; y++)
        {
          for (x = markx1; x <= markx2; x++)
          {
            map_layerdataptr[cl][y*map_layer[cl].xsize+x] = blk;
          }
        }
      }
      if (key == KEY_B)
      {
        int b = blk;
        for (x = markx1; x <= markx2; x++)
        {
          map_layerdataptr[cl][marky1*map_layer[cl].xsize+x] = b;
          b++;
        }
      }
      if (key == KEY_P)
      {
        int c = 0;
        copybuffer = 1;
        copybufferx = markx2-markx1+1;
        copybuffery = marky2-marky1+1;
        for (y = marky1; y <= marky2; y++)
        {
          for (x = markx1; x <= markx2; x++)
          {
            temparea[c] = map_layerdataptr[cl][y*map_layer[cl].xsize+x];
            c++;
          }
        }
      }
    }
  }

  if (key == KEY_3) layervisible[0] ^=1;
  if (key == KEY_4) layervisible[1] ^=1;
  if (key == KEY_O) layeroptions();
  if (key == KEY_F10) mapedithelp();
}

void getblockxy(void)
{
  blockx = mousex >> 4;
  blocky = mousey >> 4;
  if (map_layer[cl].xdivisor)
  {
    blockx = (mousex + xpos/map_layer[cl].xdivisor) >> 4;
  }
  if (map_layer[cl].ydivisor)
  {
    blocky = (mousey + ypos/map_layer[cl].ydivisor) >> 4;
  }
  if (map_layer[cl].xsize) blockx %= map_layer[cl].xsize;
  if (map_layer[cl].ysize) blocky %= map_layer[cl].ysize;
}

void staireditcommands(void)
{
        int c;
        STAIRS *sptr;
        int stairend = 0;
        mouseoverstair = -1;

        sptr = &stairs[0];
        for (c = 0; c < MAX_STAIRS; c++)
        {
                if ((sptr->src.xb) && (sptr->src.yb))
                {
                        if ((blockx == sptr->src.xb) &&
                           (blocky == sptr->src.yb))
                        {
                          mouseoverstair = c;
                          stairend = 0;
                        }

                        if ((sptr->dest.xb) && (sptr->dest.yb))
                        {
                                if ((blockx == sptr->dest.xb) &&
                                   (blocky == sptr->dest.yb))
                                {
                                  mouseoverstair = c;
                                  stairend = 1;
                                }
                        }
                }
                sptr++;
        }

        if (mouseoverstair != -1)
        {
                if (mouseb & 1)
                {
                        stairindex = mouseoverstair;
                }
                if (key == KEY_Q)
                {
                        if (!stairend)
                        {
                                stairs[mouseoverstair].src.angle -= 128;
                                stairs[mouseoverstair].src.angle &= 1023;
                        }
                        else
                        {
                                stairs[mouseoverstair].dest.angle -= 128;
                                stairs[mouseoverstair].dest.angle &= 1023;
                        }
                }
                if (key == KEY_W)
                {
                        if (!stairend)
                        {
                                stairs[mouseoverstair].src.angle += 128;
                                stairs[mouseoverstair].src.angle &= 1023;
                        }
                        else
                        {
                                stairs[mouseoverstair].dest.angle += 128;
                                stairs[mouseoverstair].dest.angle &= 1023;
                        }
                }
                if (key == KEY_C)
                {
                        stairs[mouseoverstair].src.xb = 0;
                        stairs[mouseoverstair].src.yb = 0;
                        stairs[mouseoverstair].src.angle = 0;
                        stairs[mouseoverstair].dest.xb = 0;
                        stairs[mouseoverstair].dest.yb = 0;
                        stairs[mouseoverstair].dest.angle = 0;
                        mouseoverstair = -1;
                        stairindex = -1;
                }
        }
        else
        {
                if ((mouseb & 1) && (!prevmouseb))
                {
                        sptr = &stairs[0];
                        for (c = 0; c < MAX_STAIRS; c++)
                        {
                                if ((!sptr->src.xb) && (!sptr->src.yb))
                                {
                                        sptr->src.xb = blockx;
                                        sptr->src.yb = blocky;
                                        sptr->src.angle = 0;
                                        sptr->dest.xb = 0;
                                        sptr->dest.yb = 0;
                                        sptr->dest.angle = 0;
                                        stairindex = c;
                                        mouseoverstair = c;
                                        break;
                                }
                                sptr++;
                        }
                }
        }
        if (stairindex != -1)
        {
                if ((mouseb & 2) && (!prevmouseb))
                {
                        if (mouseoverstair == -1)
                        {
                                stairs[stairindex].dest.xb = blockx;
                                stairs[stairindex].dest.yb = blocky;
                                stairs[stairindex].dest.angle = 0;
                                stairindex = -1;
                        }
                        else
                        {
                                stairindex = -1;
                        }
                }

                if (key == KEY_S)
                {
                        if ((stairs[stairindex].src.xb) && (stairs[stairindex].src.yb))
                        {
                                xpos = stairs[stairindex].src.xb*16-160;
                                ypos = stairs[stairindex].src.yb*16-100;
                        }
                }
                if (key == KEY_E)
                {
                        if ((stairs[stairindex].dest.xb) && (stairs[stairindex].dest.yb))
                        {
                                xpos = stairs[stairindex].dest.xb*16-160;
                                ypos = stairs[stairindex].dest.yb*16-100;
                        }
                }
        }
        if (key == KEY_F10) stairedithelp();
}

void actoreditcommands(void)
{
        int c;
        EXACTLOCATION *eptr;
        mouseoveractor = -1;

        /* Actor selecting */
        if (key == KEY_Z) actor--;
        if (key == KEY_X) actor++;
        if (key == KEY_A) actor -= 10;
        if (key == KEY_S) actor += 10;
        if (actor < ACTOR_FIRSTACTOR) actor = ACTOR_FIRSTACTOR;
        if (actor > ACTOR_LASTACTOR) actor = ACTOR_LASTACTOR;

        eptr = &eloc[0];
        for (c = 0; c < MAX_ACTOR; c++)
        {
                if ((blockx == eptr->xb) && (blocky == eptr->yb) && (eptr->type))
                {
                        mouseoveractor = c;
                }
                eptr++;
        }

        if (mouseoveractor >= 0)
        {
                if (key == KEY_G)
                {
                        actor = eloc[mouseoveractor].type;
                }
                if (key == KEY_Q)
                {
                        eloc[mouseoveractor].angle -= 128;
                        eloc[mouseoveractor].angle &= 1023;
                }
                if (key == KEY_W)
                {
                        eloc[mouseoveractor].angle += 128;
                        eloc[mouseoveractor].angle &= 1023;
                }
                if (mouseb & 2)
                {
                        eloc[mouseoveractor].xb = 0;
                        eloc[mouseoveractor].yb = 0;
                        eloc[mouseoveractor].type = 0;
                        eloc[mouseoveractor].angle = 0;
                        mouseoveractor = -1;
                }
        }
        if ((mouseoveractor == -1) && (mouseb & 1))
        {
                if ((blockx) && (blocky))
                {
                        eptr = &eloc[0];
                        for (c = 0; c < MAX_ACTOR; c++)
                        {
                                if (!eptr->type)
                                {
                                        eptr->type = actor;
                                        eptr->angle = 0;
                                        eptr->xb = blockx;
                                        eptr->yb = blocky;
                                        break;
                                }
                                eptr++;
                        }
                }
        }
        if (key == KEY_F10) actoredithelp();
}

void randomactoreditcommands(void)
{
        int c;
        int ok = 0;
        RANDOMLOCATION *rptr;
        int validindex = 0;
        static int validrandomactor[] = {
                ACTOR_CAT5, ACTOR_BND, ACTOR_CROSSBOW,
                ACTOR_PISTOL, ACTOR_SHOTGUN, ACTOR_UZI,
                ACTOR_GRENADE, ACTOR_BAZOOKA, ACTOR_SCANNER,
                ACTOR_BIGMEDIKIT, ACTOR_PISTOLMAN, ACTOR_LEADER};

        mouseoverractor = -1;

        randomangle += 8;
        randomangle &= 1023;

        /* Validate random actor type */
        while (!ok)
        {
                for (c = 0; c < 12; c++)
                {
                        if (ractor == validrandomactor[c])
                        {
                                ok = 1;
                                validindex = c;
                                break;
                        }
                }
                if (!ok) ractor++;
                if (ractor > ACTOR_LASTACTOR) ractor = ACTOR_FIRSTACTOR;
        }

        /* Actor selecting */
        if (key == KEY_Z) ractor = validrandomactor[(validindex+11)%12];
        if (key == KEY_X) ractor = validrandomactor[(validindex+1)%12];
        if (key == KEY_A) ractor = validrandomactor[(validindex+10)%12];
        if (key == KEY_S) ractor = validrandomactor[(validindex+2)%12];

        rptr = &rloc[0];
        for (c = 0; c < MAX_ACTOR; c++)
        {
                if ((blockx == rptr->xb) && (blocky == rptr->yb) && (rptr->type))
                {
                        mouseoverractor = c;
                }
                rptr++;
        }

        if (mouseoverractor >= 0)
        {
                if (key == KEY_G)
                {
                        ractor = rloc[mouseoverractor].type;
                }

                if ((mouseb & 2) && (!prevmouseb))
                {
                        rloc[mouseoverractor].xb = 0;
                        rloc[mouseoverractor].yb = 0;
                        rloc[mouseoverractor].type = 0;
                        mouseoverractor = -1;
                }
        }
        if ((mouseb & 1) && (!prevmouseb))
        {
                if ((blockx) && (blocky))
                {
                        rptr = &rloc[0];
                        for (c = 0; c < MAX_ACTOR; c++)
                        {
                                if (!rptr->type)
                                {
                                        rptr->type = ractor;
                                        rptr->xb = blockx;
                                        rptr->yb = blocky;
                                        break;
                                }
                                rptr++;
                        }
                }
        }
        if (key == KEY_F10) randomactoredithelp();
}

void infeditcommands(void)
{
  if (map_blkinfdata)
  {
    if (key == KEY_C) memset(&map_blkinfdata[blk*16], 0, 16);
    if (key == KEY_P) memcpy(infcopybuffer, &map_blkinfdata[blk*16], 16);
    if (key == KEY_T) memcpy(&map_blkinfdata[blk*16], infcopybuffer, 16);
    if ((mousey >= 32) && (mousey < 48))
    {
    	if ((mousex >= 8) && (mousex < 8*24+8))
    	{
    	        int bit = (mousex - 8) / 24;
    		int fine = (mousex - 8) % 24;
    		if (fine < 16)
    		{
    			if (mouseb & 1)
    			{
    				map_blkinfdata[blk*16+((mousey-32)/4)*4+fine/4] |= 1 << bit;
    			}
    			if (mouseb & 2)
    			{
    				map_blkinfdata[blk*16+((mousey-32)/4)*4+fine/4] &= 0xff - (1 << bit);
    			}
                }
  	}
    }
  }
  if (key == KEY_F10) infedithelp();
}

void movecursor(void)
{
  mousex += mousemovex;
  mousey += mousemovey;
  if (mousex < 0) mousex = 0;
  if (mousex > 319) mousex = 319;
  if (mousey < 0) mousey = 0;
  if (mousey > 199) mousey = 199;
}

void scrollmap(void)
{
  /* Map scrolling */
  /* Limit is according to the biggest layer */
  int limitx = 0;
  int limity = 0;
  int c;
  for (c = 0; c < MAX_LAYERS; c++)
  {
    if ((map_layer[c].xsize) && (map_layer[c].ysize))
    {
      int thislimitx, thislimity;
      thislimitx = (map_layer[c].xsize-MINXSIZE)*16*map_layer[c].xdivisor;
      if (limitx < thislimitx) limitx = thislimitx;
      thislimity = (map_layer[c].ysize-MINYSIZE)*16*map_layer[c].ydivisor;
      if (limity < thislimity) limity = thislimity;
    }
  }
  if (mode < MODE_LIFTEDIT)
  {
          if (key == KEY_LEFT) xpos -= 32;
          if (key == KEY_RIGHT) xpos += 32;
          if (key == KEY_UP) ypos -= 32;
          if (key == KEY_DOWN) ypos += 32;
  }
  for (c = 0; c < speed; c++)
  {
    if (!scrolllock)
    {
      if (mousex >= 320-16) xpos += 4;
      if (mousey >= 200-16) ypos += 4;
      if (mousex < 16) xpos -= 4;
      if (mousey < 16) ypos -= 4;
    }
  }
  if (xpos < 0) xpos = 0;
  if (ypos < 0) ypos = 0;
  if (xpos > limitx) xpos = limitx;
  if (ypos > limity) ypos = limity;
}

void printstatus(void)
{
  int c;
  EXACTLOCATION *eptr;
  RANDOMLOCATION *rptr;
  STAIRS *sptr;
  char *textptr;
  int incompletestairs = 0;
  int printstairs = -1;
  exactactors = 0;
  randomactors = 0;
  computers = 0;
  numstairs = 0;
  numclosets = 0;
  numlifts = 0;

  randomcat5 = 0;
  randombnd = 0;
  randomdrill = 0;
  randomcrossbow = 0;
  randompistol = 0;
  randomshotgun = 0;
  randomuzi = 0;
  randomgrenade = 0;
  randombazooka = 0;
  randomscanner = 0;
  randommedikit = 0;
  randomenemy = 0;
  randomleader = 0;


  switch(mode)
  {
    case MODE_INFEDIT:
    txt_print(220, 8, SPR_EDITORFONT, "BLOCKINFO EDITOR");
    if (map_blkinfdata)
    {
        int y, x, f;
        unsigned char *infoptr = &map_blkinfdata[16*blk];

        txt_print(8, 56, SPR_EDITORFONT, "BIT0");
        txt_print(8+24, 56, SPR_EDITORFONT, "BIT1");
        txt_print(8+2*24, 56, SPR_EDITORFONT, "BIT2");
        txt_print(8+3*24, 56, SPR_EDITORFONT, "BIT3");
        txt_print(8+4*24, 56, SPR_EDITORFONT, "BIT4");
        txt_print(8+5*24, 56, SPR_EDITORFONT, "BIT5");
        txt_print(8+6*24, 56, SPR_EDITORFONT, "BIT6");
        txt_print(8+7*24, 56, SPR_EDITORFONT, "BIT7");
        txt_print(8, 66, SPR_EDITORFONT, "OBST");
        txt_print(8+24, 66, SPR_EDITORFONT, "WALL");
        txt_print(8+2*24, 66, SPR_EDITORFONT, "DOOR");
        txt_print(8+3*24, 66, SPR_EDITORFONT, "OPEN");
        txt_print(8+4*24, 66, SPR_EDITORFONT, "STAI");
        txt_print(8+5*24, 66, SPR_EDITORFONT, "GLAS");
        txt_print(8+6*24, 66, SPR_EDITORFONT, "----");
        txt_print(8+7*24, 66, SPR_EDITORFONT, "VISI");
        for (y = 0; y < 4; y++)
    	{
    		for (x = 0; x < 4; x++)
    		{
    			if (*infoptr & 1) f=0x000c0005; else f=0x000c0006;
    			gfx_drawsprite(8+x*4,32+y*4,f);
    			if (*infoptr & 2) f=0x000c0005; else f=0x000c0006;
    			gfx_drawsprite(8+24+x*4,32+y*4,f);
    			if (*infoptr & 4) f=0x000c0005; else f=0x000c0006;
    			gfx_drawsprite(8+2*24+x*4,32+y*4,f);
    			if (*infoptr & 8) f=0x000c0005; else f=0x000c0006;
    			gfx_drawsprite(8+3*24+x*4,32+y*4,f);
    			if (*infoptr & 16) f=0x000c0005; else f=0x000c0006;
    			gfx_drawsprite(8+4*24+x*4,32+y*4,f);
    			if (*infoptr & 32) f=0x000c0005; else f=0x000c0006;
    			gfx_drawsprite(8+5*24+x*4,32+y*4,f);
    			if (*infoptr & 64) f=0x000c0005; else f=0x000c0006;
    			gfx_drawsprite(8+6*24+x*4,32+y*4,f);
    			if (*infoptr & 128) f=0x000c0005; else f=0x000c0006;
    			gfx_drawsprite(8+7*24+x*4,32+y*4,f);
    			infoptr++;
                        }
  		}
	  }
    break;

    case MODE_MAPEDIT:
    txt_print(220, 8, SPR_EDITORFONT, "MAP EDITOR");
    if (mark)
    {
      int mx, my;
      if (map_layer[cl].xdivisor) mx = markx1*16-xpos/map_layer[cl].xdivisor;
      else mx = markx1*16;
      if (map_layer[cl].xsize) mx %= (map_layer[cl].xsize*16);
      if (map_layer[cl].ydivisor) my = marky1*16-ypos/map_layer[cl].ydivisor;
      else my = marky1*16;
      if (map_layer[cl].ysize) my %= (map_layer[cl].ysize*16);
      gfx_drawsprite(mx,my,0x000c0002);
    }
    if (mark==2)
    {
      int mx, my;
      if (map_layer[cl].xdivisor) mx = markx2*16-xpos/map_layer[cl].xdivisor;
      else mx = markx2*16-xpos;
      if (map_layer[cl].xsize) mx %= (map_layer[cl].xsize*16);
      if (map_layer[cl].ydivisor) my = marky2*16-ypos/map_layer[cl].ydivisor;
      else my = marky2*16-ypos;
      if (map_layer[cl].ysize) my %= (map_layer[cl].ysize*16);
      gfx_drawsprite(mx,my,0x000c0003);
      sprintf(printbuffer, "XSIZE:%d", markx2 - markx1 + 1);
      txt_print(240, 170, SPR_EDITORFONT, printbuffer);
      sprintf(printbuffer, "YSIZE:%d", marky2 - marky1 + 1);
      txt_print(240, 180, SPR_EDITORFONT, printbuffer);
    }
    break;

    case MODE_ACTOREDIT:
    eptr = &eloc[0];
    for (c = 0; c < MAX_ACTOR; c++)
    {
        if ((eptr->type) && (eptr->xb) && (eptr->yb))
        {
                int xp = eptr->xb*16+8-xpos;
                int yp = eptr->yb*16+8-ypos;
	        int angleturn = ((eptr->angle + 64) / 128) & 7;

                if ((xp >= -16) && (xp < 336) && (yp >= -16) && (yp < 216))
                {
                        drawactor(xp, yp, eptr->type, angleturn);
                }
                exactactors++;
                if ((eptr->type >= ACTOR_FIRSTCOMPUTER) &&
                    (eptr->type <= ACTOR_LASTCOMPUTER)) computers++;
        }
        eptr++;
    }
    sprintf(printbuffer, "EXACT ACTORS: %d", exactactors);
    txt_print(220, 170, SPR_EDITORFONT, printbuffer);
    sprintf(printbuffer, "COMPUTERS: %d", computers);
    txt_print(220, 180, SPR_EDITORFONT, printbuffer);
    txt_print(220, 8, SPR_EDITORFONT, "EXACT ACTOR EDITOR");
    drawactor(16,16,actor,0);
    sprintf(printbuffer, "%04d", actor);
    txt_print(32, 8, SPR_EDITORFONT, printbuffer);
    sprintf(printbuffer, "%s", actorname[actor]);
    txt_print(32, 18, SPR_EDITORFONT, printbuffer);
    if (mouseoveractor != -1)
    {
        sprintf(printbuffer, "X: %d", eloc[mouseoveractor].xb);
        txt_print(8, 100, SPR_EDITORFONT, printbuffer);
        sprintf(printbuffer, "Y: %d", eloc[mouseoveractor].yb);
        txt_print(8, 110, SPR_EDITORFONT, printbuffer);
        sprintf(printbuffer, "TYPE: %d - %s", eloc[mouseoveractor].type, actorname[eloc[mouseoveractor].type]);
        txt_print(8, 120, SPR_EDITORFONT, printbuffer);
        sprintf(printbuffer, "DIR: %d", eloc[mouseoveractor].angle);
        txt_print(8, 130, SPR_EDITORFONT, printbuffer);
    }
    break;

    case MODE_RANDOMACTOREDIT:
    rptr = &rloc[0];
    for (c = 0; c < MAX_ACTOR; c++)
    {
        if ((rptr->type) && (rptr->xb) && (rptr->yb))
        {
                int xp = rptr->xb*16+8-xpos;
                int yp = rptr->yb*16+8-ypos;
	        int angleturn = ((randomangle + 64) / 128) & 7;

                if ((xp >= -16) && (xp < 336) && (yp >= -16) && (yp < 216))
                {
                        drawactor(xp, yp, rptr->type, angleturn);
                }
                if (rptr->type == ACTOR_CAT5) randomcat5++;
                if (rptr->type == ACTOR_BND) randombnd++;
                if (rptr->type == ACTOR_CROSSBOW) randomcrossbow++;
                if (rptr->type == ACTOR_PISTOL) randompistol++;
                if (rptr->type == ACTOR_SHOTGUN) randomshotgun++;
                if (rptr->type == ACTOR_UZI) randomuzi++;
                if (rptr->type == ACTOR_GRENADE) randomgrenade++;
                if (rptr->type == ACTOR_BAZOOKA) randombazooka++;
                if (rptr->type == ACTOR_SCANNER) randomscanner++;
                if (rptr->type == ACTOR_BIGMEDIKIT) randommedikit++;
                if (rptr->type == ACTOR_PISTOLMAN) randomenemy++;
                if (rptr->type == ACTOR_LEADER) randomleader++;
                randomactors++;
        }
        rptr++;
    }
    txt_print(220, 110, SPR_EDITORFONT, "RANDOM LOCATIONS");

    sprintf(printbuffer, "CAT5 %d", randomcat5);
    txt_print(220, 130, SPR_EDITORFONT, printbuffer);
    sprintf(printbuffer, "DRIL %d", randombnd);
    txt_print(220, 140, SPR_EDITORFONT, printbuffer);
    sprintf(printbuffer, "CBOW %d", randomcrossbow);
    txt_print(220, 150, SPR_EDITORFONT, printbuffer);
    sprintf(printbuffer, "PSTL %d", randompistol);
    txt_print(220, 160, SPR_EDITORFONT, printbuffer);
    sprintf(printbuffer, "SHOT %d", randomshotgun);
    txt_print(220, 170, SPR_EDITORFONT, printbuffer);
    sprintf(printbuffer, "UZI  %d", randomuzi);
    txt_print(220, 180, SPR_EDITORFONT, printbuffer);

    sprintf(printbuffer, "GRND %d", randomgrenade);
    txt_print(260, 130, SPR_EDITORFONT, printbuffer);
    sprintf(printbuffer, "BAZO %d", randombazooka);
    txt_print(260, 140, SPR_EDITORFONT, printbuffer);
    sprintf(printbuffer, "SCAN %d", randomscanner);
    txt_print(260, 150, SPR_EDITORFONT, printbuffer);
    sprintf(printbuffer, "MEDI %d", randommedikit);
    txt_print(260, 160, SPR_EDITORFONT, printbuffer);
    sprintf(printbuffer, "ENMY %d", randomenemy);
    txt_print(260, 170, SPR_EDITORFONT, printbuffer);
    sprintf(printbuffer, "LEAD %d", randomleader);
    txt_print(260, 180, SPR_EDITORFONT, printbuffer);

    txt_print(220, 8, SPR_EDITORFONT, "RANDOM ACTOR EDITOR");
    drawactor(16,16,ractor,0);
    sprintf(printbuffer, "%04d", ractor);
    txt_print(32, 8, SPR_EDITORFONT, printbuffer);
    if (ractor != ACTOR_PISTOLMAN)
    {
            sprintf(printbuffer, "%s", actorname[ractor]);
    }
    else
    {
            sprintf(printbuffer, "ENEMY");
    }
    txt_print(32, 18, SPR_EDITORFONT, printbuffer);
    if (mouseoverractor != -1)
    {
        sprintf(printbuffer, "X: %04d", rloc[mouseoverractor].xb);
        txt_print(8, 100, SPR_EDITORFONT, printbuffer);
        sprintf(printbuffer, "Y: %04d", rloc[mouseoverractor].yb);
        txt_print(8, 110, SPR_EDITORFONT, printbuffer);
        if (rloc[mouseoverractor].type != ACTOR_PISTOLMAN)
        {
                sprintf(printbuffer, "TYPE: %04d - %s", rloc[mouseoverractor].type, actorname[rloc[mouseoverractor].type]);
        }
        else
        {
                sprintf(printbuffer, "TYPE: %04d - ENEMY", rloc[mouseoverractor].type);
        }
        txt_print(8, 120, SPR_EDITORFONT, printbuffer);
    }
    break;

    case MODE_STAIREDIT:
    sptr = &stairs[0];
    for (c = 0; c < MAX_STAIRS; c++)
    {
        if ((sptr->src.xb) && (sptr->src.yb))
        {
                int xp = sptr->src.xb*16+8-xpos;
                int yp = sptr->src.yb*16+8-ypos;
	        int angleturn = ((sptr->src.angle + 64) / 128) & 7;

                if ((xp >= -16) && (xp < 336) && (yp >= -16) && (yp < 216))
                {
                        gfx_drawsprite(xp, yp, 0x000c0007+angleturn);
                }
                numstairs++;
                if ((sptr->dest.xb) && (sptr->dest.yb))
                {
                        int xp = sptr->dest.xb*16+8-xpos;
                        int yp = sptr->dest.yb*16+8-ypos;
        	        int angleturn = ((sptr->dest.angle + 64) / 128) & 7;

                        if ((xp >= -16) && (xp < 336) && (yp >= -16) && (yp < 216))
                        {
                                gfx_drawsprite(xp, yp, 0x000c000f+angleturn);
                        }
                }
                else incompletestairs++;
        }
        sptr++;
    }
    txt_print(220, 8, SPR_EDITORFONT, "STAIRS EDITOR");
    sprintf(printbuffer, "STAIRS: %d", numstairs);
    txt_print(220, 170, SPR_EDITORFONT, printbuffer);
    sprintf(printbuffer, "INCOMPLETE: %d", incompletestairs);
    txt_print(220, 180, SPR_EDITORFONT, printbuffer);
    if (stairindex != -1) printstairs = stairindex;
    if (mouseoverstair != -1) printstairs = mouseoverstair;
    if (printstairs != -1)
    {
        sprintf(printbuffer, "START X: %d", stairs[printstairs].src.xb);
        txt_print(8, 80, SPR_EDITORFONT, printbuffer);
        sprintf(printbuffer, "START Y: %d", stairs[printstairs].src.yb);
        txt_print(8, 90, SPR_EDITORFONT, printbuffer);
        sprintf(printbuffer, "START DIR: %d", stairs[printstairs].src.angle);
        txt_print(8, 100, SPR_EDITORFONT, printbuffer);
        sprintf(printbuffer, "END X: %d", stairs[printstairs].dest.xb);
        txt_print(8, 110, SPR_EDITORFONT, printbuffer);
        sprintf(printbuffer, "END Y: %d", stairs[printstairs].dest.yb);
        txt_print(8, 120, SPR_EDITORFONT, printbuffer);
        sprintf(printbuffer, "END DIR: %d", stairs[printstairs].dest.angle);
        txt_print(8, 130, SPR_EDITORFONT, printbuffer);
        if (stairindex != -1)
        txt_print(8, 150, SPR_EDITORFONT, "SET ENDPOINT WITH RIGHT MOUSEB.");
    }
    break;

    case MODE_CLOSETEDIT:
    if ((closet[clo].xb) && (closet[clo].xb))
    {
        static char *animstring[] = {"NO", "YES"};

        int xp = closet[clo].xb*16-xpos;
        int yp = closet[clo].yb*16-ypos;

        gfx_drawsprite(xp,yp,0x000c0017);

        /* Draw closet */
	gfx_drawsprite(223, 48, 0x90002);
	/* Draw all network equipment */
	for (c = MAX_EQUIP-1; c >= 0; c--)
	{
		switch(closet[clo].equipment[c])
		{
			case EQUIP_RIMA:
			gfx_drawsprite(223, 53 + 10 * c, 0x90003);
			break;

			case EQUIP_HP_HUB:
			gfx_drawsprite(223, 53 + 10 * c, 0x90004 + (rand()&1));
			break;

			case EQUIP_AT_T_HUB:
			gfx_drawsprite(223, 53 + 10 * c, 0x90006 + (rand()&1));
			break;

			case EQUIP_CISCO_SWITCH:
			gfx_drawsprite(223, 53 + 10 * c, 0x90008 + (rand()&1));
			break;

			case EQUIP_RJ45RIMA:
			gfx_drawsprite(223, 53 + 10 * c, 0x9000a);
			break;
		}
	}

        sprintf(printbuffer, "X: %d", closet[clo].xb);
        txt_print(8, 90, SPR_EDITORFONT, printbuffer);
        sprintf(printbuffer, "Y: %d", closet[clo].yb);
        txt_print(8, 100, SPR_EDITORFONT, printbuffer);
        sprintf(printbuffer, "ANIM: %s", animstring[closet[clo].anim]);
        txt_print(8, 110, SPR_EDITORFONT, printbuffer);
        if ((!enterclosetname) || (enternameflash & 16))
        {
                sprintf(printbuffer, "NAME: %s", closet[clo].name);
                txt_print(8, 120, SPR_EDITORFONT, printbuffer);
        }
    }

    txt_print(220, 8, SPR_EDITORFONT, "CLOSET EDITOR");
    for (c = 0; c < MAX_CLOSET; c++)
    {
        if ((closet[c].xb) && (closet[c].yb)) numclosets++;
    }
    sprintf(printbuffer, "CLOSETS: %d", numclosets);
    txt_print(220, 180, SPR_EDITORFONT, printbuffer);
    sprintf(printbuffer, "CLOSET NUM: %d", clo);
    txt_print(8, 80, SPR_EDITORFONT, printbuffer);
    break;

    case MODE_LIFTEDIT:
    if (lift[clift].floors)
    {
      for (c = 0; c < lift[clift].floors; c++)
      {
        int xp = lift[clift].liftfloor[c].xb*16+5-xpos;
        int yp = lift[clift].liftfloor[c].yb*16+5-ypos;
        sprintf(printbuffer, "%d", c+lift[clift].firstfloor);
        txt_print(xp, yp, SPR_EDITORFONT, printbuffer);
      }
    }

    txt_print(220, 8, SPR_EDITORFONT, "LIFT EDITOR");
    for (c = 0; c < MAX_LIFT; c++)
    {
        if (lift[c].floors) numlifts++;
    }
    sprintf(printbuffer, "LIFTS: %d", numlifts);
    txt_print(220, 180, SPR_EDITORFONT, printbuffer);
    sprintf(printbuffer, "LIFT NUM: %d", clift);
    txt_print(8, 30, SPR_EDITORFONT, printbuffer);
    sprintf(printbuffer, "FLOORS: %d", lift[clift].floors);
    txt_print(8, 40, SPR_EDITORFONT, printbuffer);
    gfx_drawsprite(-6, 39+liftcursorpos*10, 0x000c0004);
    if (lift[clift].floors)
    {
      sprintf(printbuffer, "FIRST FLOOR: %d", lift[clift].firstfloor);
      txt_print(8, 50, SPR_EDITORFONT, printbuffer);
      sprintf(printbuffer, "SPEED: %d", lift[clift].speed);
      txt_print(8, 60, SPR_EDITORFONT, printbuffer);
      sprintf(printbuffer, "START DELAY: %d", lift[clift].startdelay);
      txt_print(8, 70, SPR_EDITORFONT, printbuffer);
      for (c = 0; c < lift[clift].floors; c++)
      {
        sprintf(printbuffer, "%d. FLOOR POS: %d,%d", c+lift[clift].firstfloor,
           lift[clift].liftfloor[c].xb,
           lift[clift].liftfloor[c].yb);
        txt_print(8, 80+10*c, SPR_EDITORFONT, printbuffer);
      }
    }
    break;

    case MODE_PARAMEDIT:
    txt_print(8, 0, SPR_EDITORFONT, "BOFH START X Y DIR");
    txt_print(8, 6, SPR_EDITORFONT, "SERVER ROOM TOP LEFT X Y");
    txt_print(8, 12, SPR_EDITORFONT, "SERVER ROOM BOTTOM RIGHT X Y");
    txt_print(8, 18, SPR_EDITORFONT, "VICTORY CONDITION");
    txt_print(150, 30, SPR_EDITORFONT, "PRACT EASY MEDIUM HARD INSANE");
    txt_print(8, 36,  SPR_EDITORFONT, "TIME (MINUTES)");
    txt_print(8, 42,  SPR_EDITORFONT, "BOMBS");
    txt_print(8, 48,  SPR_EDITORFONT, "RANDOM ENEMIES");
    txt_print(8, 54,  SPR_EDITORFONT, "RANDOM PISTOLMEN");
    txt_print(8, 60,  SPR_EDITORFONT, "RANDOM SHOTGUNMEN");
    txt_print(8, 66,  SPR_EDITORFONT, "RANDOM UZIMEN");
    txt_print(8, 72,  SPR_EDITORFONT, "RANDOM SADISTS");
    txt_print(8, 78,  SPR_EDITORFONT, "RANDOM LEADERS");
    txt_print(8, 84,  SPR_EDITORFONT, "RANDOM WHIPS");
    txt_print(8, 90,  SPR_EDITORFONT, "RANDOM DRILLS");
    txt_print(8, 96,  SPR_EDITORFONT, "RANDOM CROSSBOWS");
    txt_print(8, 102, SPR_EDITORFONT, "RANDOM PISTOLS");
    txt_print(8, 108, SPR_EDITORFONT, "RANDOM UZIS");
    txt_print(8, 114, SPR_EDITORFONT, "RANDOM GRENADES");
    txt_print(8, 120, SPR_EDITORFONT, "RANDOM BAZOOKAS");
    txt_print(8, 126, SPR_EDITORFONT, "RANDOM SCANNERS");
    txt_print(8, 132, SPR_EDITORFONT, "RANDOM SMALL MEDIKITS");
    txt_print(8, 138, SPR_EDITORFONT, "RANDOM BIG MEDIKITS");
    txt_print(8, 150, SPR_EDITORFONT, "FISTMAN ATTACK PROBABILITY");
    txt_print(8, 156, SPR_EDITORFONT, "PISTOLMAN ATTACK PROBABILITY");
    txt_print(8, 162, SPR_EDITORFONT, "SHOTGMAN ATTACK PROBABILITY");
    txt_print(8, 168, SPR_EDITORFONT, "UZIMAN ATTACK PROBABILITY");
    txt_print(8, 174, SPR_EDITORFONT, "SADIST ATTACK PROBABILITY");
    txt_print(8, 180, SPR_EDITORFONT, "LEADER ATTACK PROBABILITY");
    txt_print(8, 192, SPR_EDITORFONT, "NUMBER OF FIRST DAMAGED BLOCK");

    for (c = 0; c < (sizeof paramrow / sizeof(PARAMROW)); c++)
    {
        int d;

        for (d = 0; d < paramrow[c].columns; d++)
        {
                if (!paramrow[c].column[d].special)
                {
                       sprintf(printbuffer, "%d", *paramrow[c].column[d].address);
                }
                else
                {
                       int bits = *paramrow[c].column[d].address;

                       memset(printbuffer, 0, 80);
                       if (bits & VB_BOMBS) strcat(printbuffer, "BOMBS ");
                       if (bits & VB_LEADERS) strcat(printbuffer, "LEADERS ");
                       if (bits & VB_TERRORISTS) strcat(printbuffer, "ENEMIES ");
                       if (bits & VB_COMPUTERS) strcat(printbuffer, "COMPUTERS");
                }
                if ((paramx != d) || (paramy != c) || (paramflash & 16))
                        txt_print(paramrow[c].column[d].x, paramrow[c].y, SPR_EDITORFONT, printbuffer);
        }
    }
    break;

    case MODE_MESSAGEEDIT:
    if (!messagenum)
    {
        textptr = briefingtext;
        txt_print(8, 0, SPR_EDITORFONT, "BRIEFING MESSAGE");
    }
    else
    {
        textptr = victorytext;
        txt_print(8, 0, SPR_EDITORFONT, "VICTORY MESSAGE");
    }
    for (c = 0; c < MAXMSGROW; c++)
    {
        txt_print(8, 20+c*8, SPR_EDITORFONT, textptr);
        textptr += 80;
        if ((c == messagerow) && (messageflash & 16))
        {
                int d;
                for (d = txt_lasty; d < txt_lasty+7; d++)
                {
                        gfx_line(txt_lastx, d, txt_lastx+4, d, 7);
                }
        }
    }
    sprintf(printbuffer, "ROW %d COLUMN %d", messagerow, messagecolumn);
    txt_print(8, 180, SPR_EDITORFONT, printbuffer);
    break;
  }

  if (mode <= MODE_INFEDIT)
  {
    gfx_drawblock(8, 8, blk);
    sprintf(printbuffer, "%04d", blk);
    txt_print(32, 8, SPR_EDITORFONT, printbuffer);
  }

  if (mode < MODE_PARAMEDIT)
  {
          sprintf(printbuffer, "X:%04d", blockx);
          txt_print(8, 170, SPR_EDITORFONT, printbuffer);
          sprintf(printbuffer, "Y:%04d", blocky);
          txt_print(8, 180, SPR_EDITORFONT, printbuffer);
          if (scrolllock) txt_print(240, 8, SPR_EDITORFONT, "SCRLOCK");
          sprintf(printbuffer, "L:%1d", cl+1);
          txt_print(64,170, SPR_EDITORFONT, printbuffer);
          sprintf(printbuffer, "V:");
          for (c = 0; c < MAX_LAYERS; c++)
          {
            char layerstring[2];
            layerstring[0] = '1'+c;
            layerstring[1] = 0;
            if (layervisible[c]) strcat(printbuffer,layerstring);
          }
          txt_print(64,180, SPR_EDITORFONT, printbuffer);
  }

}

void drawactor(int xp, int yp, int type, int angleturn)
{
        int frame;

        switch(type)
        {
                case ACTOR_ACER:
                frame = 0x00030001 + angleturn;
                gfx_drawsprite(xp, yp, frame);
                break;

                case ACTOR_POMI:
                frame = 0x00030009 + angleturn;
                gfx_drawsprite(xp, yp, frame);
                break;

                case ACTOR_SYSTECH:
                frame = 0x00030011 + angleturn;
                gfx_drawsprite(xp, yp, frame);
                break;

                case ACTOR_THINKPAD:
                frame = 0x00030019 + angleturn;
                gfx_drawsprite(xp, yp, frame);
                break;

                case ACTOR_DOCKINGSTAT:
                frame = 0x00030021 + angleturn;
                gfx_drawsprite(xp, yp, frame);
                break;

                case ACTOR_LASERJET:
                frame = 0x00030029 + angleturn;
                gfx_drawsprite(xp, yp, frame);
                break;

                case ACTOR_DESKJET:
                frame = 0x00030031 + angleturn;
                gfx_drawsprite(xp, yp, frame);
                break;

                case ACTOR_BIGLASERJET:
                frame = 0x00030039 + angleturn;
                gfx_drawsprite(xp, yp, frame);
                break;

                case ACTOR_CDSERVER:
                frame = 0x00030059;
                gfx_drawsprite(xp, yp, frame);
                break;

                case ACTOR_SUNSERVER:
                frame = 0x0003005a;
                gfx_drawsprite(xp, yp, frame);
                break;

                case ACTOR_COMPAQSERVER:
                frame = 0x0003005b + angleturn;
                gfx_drawsprite(xp, yp, frame);
                break;

                case ACTOR_KEYBOARD:
                frame = 0x00030056;
                gfx_drawsprite(xp, yp, frame);
                break;

                case ACTOR_MULTITRACK:
                frame = 0x00030057;
                gfx_drawsprite(xp, yp, frame);
                break;

                case ACTOR_VCR_AND_TV:
                frame = 0x00030058;
                gfx_drawsprite(xp, yp, frame);
                break;

                case ACTOR_BEAM:
                frame = 0x00030063 + (angleturn >> 1);
  		gfx_drawsprite(xp, yp, frame);
                break;

 		case ACTOR_DEADFISTMAN:
 		frame = 0x00040059 + angleturn * 2+1;
                gfx_drawsprite(xp, yp, frame);
                break;

 		case ACTOR_DEADPISTOLMAN:
 		frame = 0x00050041 + angleturn * 2+1;
                gfx_drawsprite(xp, yp, frame);
                break;

 		case ACTOR_DEADSHOTGUNMAN:
 		frame = 0x00060041 + angleturn * 2+1;
                gfx_drawsprite(xp, yp, frame);
                break;

 		case ACTOR_DEADUZIMAN:
 		frame = 0x00070041 + angleturn * 2+1;
                gfx_drawsprite(xp, yp, frame);
                break;

 		case ACTOR_DEADSADIST:
 		frame = 0x000b0059 + angleturn * 2+1;
                gfx_drawsprite(xp, yp, frame);
                break;

 		case ACTOR_DEADLEADER:
 		frame = 0x000a0041 + angleturn * 2+1;
                gfx_drawsprite(xp, yp, frame);
                break;

 		case ACTOR_DEADTECHNICIAN:
 		frame = 0x00080059 + angleturn * 2+1;
                gfx_drawsprite(xp, yp, frame);
                break;

 		case ACTOR_DEADBOFH:
 		frame = SPRI_DEADBOFH2;
                gfx_drawsprite(xp, yp, frame);
                break;

                case ACTOR_CAT5:
 		frame = SPRI_CAT5;
                gfx_drawsprite(xp, yp, frame);
 		break;

                case ACTOR_BND:
                frame = SPRI_BND;
                gfx_drawsprite(xp, yp, frame);
                break;

                case ACTOR_CROSSBOW:
 		frame = SPRI_CROSSBOW;
                gfx_drawsprite(xp, yp, frame);
 		break;

                case ACTOR_SCANNER:
 		frame = SPRI_SCANNER;
                gfx_drawsprite(xp, yp, frame);
 		break;

                case ACTOR_PISTOL:
 		frame = SPRI_PISTOL;
                gfx_drawsprite(xp, yp, frame);
 		break;

                case ACTOR_SHOTGUN:
 		frame = SPRI_SHOTGUN;
                gfx_drawsprite(xp, yp, frame);
 		break;

                case ACTOR_UZI:
 		frame = SPRI_UZI;
                gfx_drawsprite(xp, yp, frame);
 		break;

                case ACTOR_SMALLMEDIKIT:
 		frame = SPRI_SMALLMEDI;
                gfx_drawsprite(xp, yp, frame);
 		break;

                case ACTOR_BIGMEDIKIT:
 		frame = SPRI_BIGMEDI;
                gfx_drawsprite(xp, yp, frame);
 		break;

                case ACTOR_INSTRUCTIONS:
 		frame = SPRI_INSTRUCT;
                gfx_drawsprite(xp, yp, frame);
 		break;

                case ACTOR_GRENADE:
 		frame = SPRI_GRENADE;
                gfx_drawsprite(xp, yp, frame);
 		break;

                case ACTOR_BAZOOKA: // ***
                frame = SPRI_BAZOOKA;
                gfx_drawsprite(xp, yp, frame);
 		break;

                case ACTOR_BAZOOKA_STRAP: // ***
                frame = SPRI_BAZOOKA_STRAP;
                gfx_drawsprite(xp, yp, frame);
                break;

                case ACTOR_BAZOOKA_USED: // ***
                frame = SPRI_BAZOOKA_USED;
                gfx_drawsprite(xp, yp, frame);
                break;

                case ACTOR_SHELL:                 // ***
                frame = SPRI_SHELL;               //
                gfx_drawsprite(xp, yp, frame);  //
                break;                            //

                case ACTOR_SHOTGSHELL:                 // ***
                frame = SPRI_SHOTGSHELL;               //
                gfx_drawsprite(xp, yp, frame);       //
                break;                                 //

                case ACTOR_ARROW: // ***
                {
                  gfx_plot(xp, yp, 6);
                }
                break;

                case ACTOR_RICOCHET: // ***
                frame = SPRI_RICOCHET;
                gfx_drawsprite(xp, yp, frame);
                break;

                // ***
                case ACTOR_BULLET:
                break;

 		case ACTOR_BOFH:
 		frame = 0x00010001 + angleturn * 11;
                gfx_drawsprite(xp, yp, frame);
                break;

                case ACTOR_MUZZLE:
                frame = 0x00020019 + angleturn;
                gfx_drawsprite(xp, yp, frame);
                break;

                case ACTOR_SMOKE:
                frame = 0x00020021;
                gfx_drawsprite(xp, yp, frame);
                break;

                case ACTOR_EXPLOSION:
                frame = 0x00030041;
                gfx_drawsprite(xp, yp, frame);
                break;

                case ACTOR_CHUNK:
                frame = 0x00030045;
                gfx_drawsprite(xp, yp, frame);
                break;

                case ACTOR_SPARK:
                frame = 0x0003004d;
                gfx_drawsprite(xp, yp, frame);
                break;

                case ACTOR_BLOOD:
                frame = 0x00030051;
                gfx_drawsprite(xp, yp, frame);
                break;

                case ACTOR_FLAME:
 		frame = SPRI_FLAME;
                gfx_drawsprite(xp, yp, frame);
 		break;

                case ACTOR_SHARD:
 		frame = SPRI_SHARD;
                gfx_drawsprite(xp, yp, frame);
 		break;

                case ACTOR_FLYINGGRENADE:
 		frame = SPRI_GRENADE;
                gfx_drawsprite(xp, yp, frame);
 		break;

                // ***
                case ACTOR_BAZOOKA_PROJECTILE:
                frame = SPRI_BAZOOKA_PROJECTILE + angleturn;
                gfx_drawsprite(xp, yp, frame);
                break;

 		case ACTOR_FISTMAN:
 		frame = 0x00040001 + angleturn * 11;
                gfx_drawsprite(xp, yp, frame);
                break;

 		case ACTOR_PISTOLMAN:
 		frame = 0x00050001 + angleturn * 8;
                gfx_drawsprite(xp, yp, frame);
                break;

 		case ACTOR_SHOTGUNMAN:
 		frame = 0x00060001 + angleturn * 8;
                gfx_drawsprite(xp, yp, frame);
                break;

 		case ACTOR_UZIMAN:
 		frame = 0x00070001 + angleturn * 8;
                gfx_drawsprite(xp, yp, frame);
                break;

 		case ACTOR_SADIST:
 		frame = 0x000b0001 + angleturn * 11;
                gfx_drawsprite(xp, yp, frame);
                break;

 		case ACTOR_TECHNICIAN:
 		frame = 0x00080001 + angleturn * 11;
                gfx_drawsprite(xp, yp, frame);
                break;

 		case ACTOR_LEADER:
 		frame = 0x000a0001 + angleturn * 8;
                gfx_drawsprite(xp, yp, frame);
                break;
         }
}

void loadblocks(void)
{
  char textbuffer[13];

  textbuffer[0] = 0;
  if (!enterfilename("LOAD BLOCKS", textbuffer)) return;

  if (gfx_loadblocks(textbuffer))
  {
    memcpy(map_header.blocksname, textbuffer, 13);
  }
}

void mapedithelp(void)
{
  gfx_fillscreen(0);
  txt_printcenter(0, SPR_EDITORFONT, "MAP EDITOR HELP");
  txt_printcenter(20, SPR_EDITORFONT, "LEFT MOUSEBUTTON - DRAW BLOCKS");
  txt_printcenter(30, SPR_EDITORFONT, "RIGHT MOUSEBUTTON - MARK AREAS");
  txt_printcenter(40, SPR_EDITORFONT, "1,2 - SELECT LAYER TO WORK ON");
  txt_printcenter(50, SPR_EDITORFONT, "3,4 - TOGGLE LAYER VISIBILITY");
  txt_printcenter(60, SPR_EDITORFONT, "O - LAYER SIZE & OPTIONS");
  txt_printcenter(70, SPR_EDITORFONT, "G - GRAB BLOCK UNDER CURSOR");
  txt_printcenter(80, SPR_EDITORFONT, "P - PUT MARKED AREA TO COPYBUFFER");
  txt_printcenter(90, SPR_EDITORFONT, "T - TAKE AREA FROM COPYBUFFER");
  txt_printcenter(100, SPR_EDITORFONT, "F,B - FILL MARKED AREA");
  txt_printcenter(110, SPR_EDITORFONT, "Z,X - SELECT BLOCK");
  txt_printcenter(120, SPR_EDITORFONT, "A,S - SELECT BLOCK FAST");
  txt_printcenter(130, SPR_EDITORFONT, "I,J,K,M - CHANGE SIZE FAST IN LAYER OPT.");
  txt_printcenter(140, SPR_EDITORFONT, "F10 - HELP SCREEN");
  txt_printcenter(150, SPR_EDITORFONT, "F11 - LOAD MISSION");
  txt_printcenter(160, SPR_EDITORFONT, "F12 - SAVE MISSION");
  txt_printcenter(170, SPR_EDITORFONT, "SCROLL LOCK - TURN SCROLLING ON/OFF");
  gfx_updatepage();

  for (;;)
  {
    geteditspeed();
    key = getkey_noshift();
    updatemouse();
    if ((mouseb) || (key)) break;
  }
}

void infedithelp(void)
{
  gfx_fillscreen(0);
  txt_printcenter(0, SPR_EDITORFONT, "BLOCKINFO EDITOR HELP");
  txt_printcenter(20, SPR_EDITORFONT, "LEFT MOUSEBUTTON - SET BLOCKINFO BITS");
  txt_printcenter(30, SPR_EDITORFONT, "RIGHT MOUSEBUTTON - CLEAR BLOCKINFO BITS");
  txt_printcenter(40, SPR_EDITORFONT, "C - CLEAR BLOCKINFO");
  txt_printcenter(50, SPR_EDITORFONT, "P - PUT BLOCKINFO TO COPYBUFFER");
  txt_printcenter(60, SPR_EDITORFONT, "T - TAKE BLOCKINFO FROM COPYBUFFER");
  txt_printcenter(70, SPR_EDITORFONT, "Z,X - SELECT BLOCK");
  txt_printcenter(80, SPR_EDITORFONT, "A,S - SELECT BLOCK FAST");
  txt_printcenter(90, SPR_EDITORFONT, "F10 - HELP SCREEN");
  txt_printcenter(100, SPR_EDITORFONT, "F11 - LOAD MISSION");
  txt_printcenter(110, SPR_EDITORFONT, "F12 - SAVE MISSION");
  txt_printcenter(120, SPR_EDITORFONT, "SCROLL LOCK - TURN SCROLLING ON/OFF");
  gfx_updatepage();

  for (;;)
  {
    geteditspeed();
    key = getkey_noshift();
    updatemouse();
    if ((mouseb) || (key)) break;
  }
}

void actoredithelp(void)
{
  gfx_fillscreen(0);
  txt_printcenter(0, SPR_EDITORFONT, "EXACT ACTOR EDITOR HELP");
  txt_printcenter(20, SPR_EDITORFONT, "LEFT MOUSEBUTTON - CREATE EXACT ACTOR");
  txt_printcenter(30, SPR_EDITORFONT, "RIGHT MOUSEBUTTON - REMOVE EXACT ACTOR");
  txt_printcenter(40, SPR_EDITORFONT, "G - GRAB ACTOR UNDER CURSOR");
  txt_printcenter(50, SPR_EDITORFONT, "Z,X - SELECT ACTOR");
  txt_printcenter(60, SPR_EDITORFONT, "A,S - SELECT ACTOR FAST");
  txt_printcenter(70, SPR_EDITORFONT, "Q,W - ROTATE ACTOR");
  txt_printcenter(80, SPR_EDITORFONT, "F10 - HELP SCREEN");
  txt_printcenter(90, SPR_EDITORFONT, "F11 - LOAD MISSION");
  txt_printcenter(100, SPR_EDITORFONT, "F12 - SAVE MISSION");
  txt_printcenter(110, SPR_EDITORFONT, "SCROLL LOCK - TURN SCROLLING ON/OFF");
  gfx_updatepage();

  for (;;)
  {
    geteditspeed();
    key = getkey_noshift();
    updatemouse();
    if ((mouseb) || (key)) break;
  }
}

void randomactoredithelp(void)
{
  gfx_fillscreen(0);
  txt_printcenter(0, SPR_EDITORFONT, "RANDOM ACTOR EDITOR HELP");
  txt_printcenter(20, SPR_EDITORFONT, "LEFT MOUSEBUTTON - CREATE RANDOM ACTOR");
  txt_printcenter(30, SPR_EDITORFONT, "RIGHT MOUSEBUTTON - REMOVE RANDOM ACTOR");
  txt_printcenter(40, SPR_EDITORFONT, "G - GRAB ACTOR UNDER CURSOR");
  txt_printcenter(50, SPR_EDITORFONT, "Z,X - SELECT ACTOR");
  txt_printcenter(60, SPR_EDITORFONT, "A,S - SELECT ACTOR FAST");
  txt_printcenter(70, SPR_EDITORFONT, "F10 - HELP SCREEN");
  txt_printcenter(80, SPR_EDITORFONT, "F11 - LOAD MISSION");
  txt_printcenter(90, SPR_EDITORFONT, "F12 - SAVE MISSION");
  txt_printcenter(100, SPR_EDITORFONT, "SCROLL LOCK - TURN SCROLLING ON/OFF");
  gfx_updatepage();

  for (;;)
  {
    geteditspeed();
    key = getkey_noshift();
    updatemouse();
    if ((mouseb) || (key)) break;
  }
}

void stairedithelp(void)
{
  gfx_fillscreen(0);
  txt_printcenter(0, SPR_EDITORFONT, "STAIR EDITOR HELP");
  txt_printcenter(20, SPR_EDITORFONT, "LEFT MOUSEBUTTON - PLACE NEW STAIRS/SELECT STAIRS");
  txt_printcenter(30, SPR_EDITORFONT, "RIGHT MOUSEBUTTON - SET ENDPOINT OF SELECTED STAIRS");
  txt_printcenter(40, SPR_EDITORFONT, "Q,W - ROTATE STAIRS");
  txt_printcenter(50, SPR_EDITORFONT, "S - GO TO STARTPOINT OF SELECTED STAIRS");
  txt_printcenter(60, SPR_EDITORFONT, "E - GO TO ENDPOINT OF SELECTED STAIRS");
  txt_printcenter(70, SPR_EDITORFONT, "C - DELETE STAIRS");
  txt_printcenter(80, SPR_EDITORFONT, "F10 - HELP SCREEN");
  txt_printcenter(90, SPR_EDITORFONT, "F11 - LOAD MISSION");
  txt_printcenter(100, SPR_EDITORFONT, "F12 - SAVE MISSION");
  txt_printcenter(110, SPR_EDITORFONT, "SCROLL LOCK - TURN SCROLLING ON/OFF");
  gfx_updatepage();

  for (;;)
  {
    geteditspeed();
    key = getkey_noshift();
    updatemouse();
    if ((mouseb) || (key)) break;
  }
}

void closetedithelp(void)
{
  gfx_fillscreen(0);
  txt_printcenter(0, SPR_EDITORFONT, "NETWORK CLOSET EDITOR HELP");
  txt_printcenter(20, SPR_EDITORFONT, "LEFT MOUSEBUTTON - CREATE/RE-POSITION CLOSET");
  txt_printcenter(30, SPR_EDITORFONT, "RIGHT MOUSEBUTTON - REMOVE CLOSET");
  txt_printcenter(40, SPR_EDITORFONT, "Z,X - SELECT CLOSET");
  txt_printcenter(50, SPR_EDITORFONT, "A,S - SELECT CLOSET FAST");
  txt_printcenter(60, SPR_EDITORFONT, "G - GO TO CLOSET LOCATION");
  txt_printcenter(70, SPR_EDITORFONT, "F - TOGGLE ANIMATION");
  txt_printcenter(80, SPR_EDITORFONT, "N - EDIT CLOSET NAME");
  txt_printcenter(90, SPR_EDITORFONT, "1-9 - EDIT CLOSET EQUIPMENT");
  txt_printcenter(100, SPR_EDITORFONT, "F10 - HELP SCREEN");
  txt_printcenter(110, SPR_EDITORFONT, "F11 - LOAD MISSION");
  txt_printcenter(120, SPR_EDITORFONT, "F12 - SAVE MISSION");
  txt_printcenter(130, SPR_EDITORFONT, "SCROLL LOCK - TURN SCROLLING ON/OFF");
  gfx_updatepage();

  for (;;)
  {
    geteditspeed();
    key = getkey_noshift();
    updatemouse();
    if ((mouseb) || (key)) break;
  }
}

void liftedithelp(void)
{
  gfx_fillscreen(0);
  txt_printcenter(0, SPR_EDITORFONT, "LIFT EDITOR HELP");
  txt_printcenter(20, SPR_EDITORFONT, "Z,X - SELECT LIFT");
  txt_printcenter(30, SPR_EDITORFONT, "A,S - SELECT LIFT FAST");
  txt_printcenter(40, SPR_EDITORFONT, "CURSOR UP,DOWN - SELECT VALUE TO CHANGE");
  txt_printcenter(50, SPR_EDITORFONT, "CURSOR LEFT,RIGHT - CHANGE VALUE");
  txt_printcenter(60, SPR_EDITORFONT, "0-9 - PLACE LIFT ON FLOOR");
  txt_printcenter(70, SPR_EDITORFONT, "F10 - HELP SCREEN");
  txt_printcenter(80, SPR_EDITORFONT, "F11 - LOAD MISSION");
  txt_printcenter(90, SPR_EDITORFONT, "F12 - SAVE MISSION");
  txt_printcenter(100, SPR_EDITORFONT, "SCROLL LOCK - TURN SCROLLING ON/OFF");
  gfx_updatepage();

  for (;;)
  {
    geteditspeed();
    key = getkey_noshift();
    updatemouse();
    if ((mouseb) || (key)) break;
  }
}

void messageedithelp(void)
{
  gfx_fillscreen(0);
  txt_printcenter(0, SPR_EDITORFONT, "MESSAGE EDITOR HELP");
  txt_printcenter(20, SPR_EDITORFONT, "TAB - SWITCH BETWEEN BRIEFING & VICTORY TEXT");
  txt_printcenter(30, SPR_EDITORFONT, "F10 - HELP SCREEN");
  txt_printcenter(40, SPR_EDITORFONT, "F11 - LOAD MISSION");
  txt_printcenter(50, SPR_EDITORFONT, "F12 - SAVE MISSION");
  txt_printcenter(60, SPR_EDITORFONT, "SCROLL LOCK - TURN SCROLLING ON/OFF");
  txt_printcenter(80, SPR_EDITORFONT, "WARNING! THIS TEXT EDITOR IS VERY PRIMITIVE! :-)");
  gfx_updatepage();

  for (;;)
  {
    geteditspeed();
    key = getkey_noshift();
    updatemouse();
    if ((mouseb) || (key)) break;
  }
}

void paramedithelp(void)
{
  gfx_fillscreen(0);
  txt_printcenter(0, SPR_EDITORFONT, "PARAMETER EDITOR HELP");
  txt_printcenter(20, SPR_EDITORFONT, "Z,X - ADJUST VALUE");
  txt_printcenter(30, SPR_EDITORFONT, "A,S - ADJUST VALUE");
  txt_printcenter(40, SPR_EDITORFONT, "CURSOR UP,DOWN,LEFT,RIGHT - SELECT VALUE TO CHANGE");
  txt_printcenter(50, SPR_EDITORFONT, "F10 - HELP SCREEN");
  txt_printcenter(60, SPR_EDITORFONT, "F11 - LOAD MISSION");
  txt_printcenter(70, SPR_EDITORFONT, "F12 - SAVE MISSION");
  txt_printcenter(80, SPR_EDITORFONT, "SCROLL LOCK - TURN SCROLLING ON/OFF");
  gfx_updatepage();

  for (;;)
  {
    geteditspeed();
    key = getkey_noshift();
    updatemouse();
    if ((mouseb) || (key)) break;
  }
}

void initmap(void)
{
  int c;
  memset(map_header.blocksname, 0, sizeof(map_header.blocksname));
  memset(map_header.palettename, 0, sizeof(map_header.palettename));

  for (c = 0; c < MAX_LAYERS; c++)
  {
    map_layer[c].xsize = 0;
    map_layer[c].ysize = 0;
    map_layer[c].xdivisor = 1;
    map_layer[c].ydivisor = 1;
    map_layer[c].xwrap = 0;
    map_layer[c].ywrap = 0;
    map_layerdataptr[c] = malloc(MAXMAPSIZE*MAXMAPSIZE*2);
    if (!map_layerdataptr[c])
    {
      printf("Out of memory when reserving map area!\n");
      exit(1);
    }
    memset(map_layerdataptr[c], 0, MAXMAPSIZE*MAXMAPSIZE*2);
  }
  map_layer[0].xsize = MINXSIZE;
  map_layer[0].ysize = MINYSIZE;
  map_layer[1].xsize = MINXSIZE;
  map_layer[1].ysize = MINYSIZE;
  temparea = malloc(MAXMAPSIZE*MAXMAPSIZE*2);
  if (!temparea)
  {
    printf("Out of memory when reserving map work area!\n");
    exit(1);
  }
}

int enterfilename(char *text, char *buffer)
{
  int c;
  for (c = strlen(buffer); c < 13; c++) buffer[c] = 0;

  for (;;)
  {
    unsigned char ascii;
    int cursorpos;

    geteditspeed();
    key = getkey_noshift();
    ascii = kbd_getascii();
    ascii = tolower(ascii);
    updatemouse();

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
      return 0;
    }
    if (ascii == 13) return 1;
    if ((ascii >= 32) && (cursorpos < 12))
    {
      buffer[cursorpos] = ascii;
    }

    gfx_fillscreen(0);
    txt_printcenter(80, SPR_EDITORFONT, text);
    txt_printcenter(90, SPR_EDITORFONT, buffer);
    gfx_updatepage();
  }
}

int getkey_noshift(void)
{
        int c;

        for (c = 0; c < MAX_KEYS; c++)
        {
                if ((win_keytable[c]) && (c != KEY_LEFTSHIFT) && (c != KEY_RIGHTSHIFT))
                {
                        win_keytable[c] = 0;
                        return c;
                }
        }
        return 0;
}

void layeroptions(void)
{
  int newsizex = map_layer[cl].xsize;
  int newsizey = map_layer[cl].ysize;

  for (;;)
  {
    geteditspeed();
    key = getkey_noshift();

    if ((key == KEY_ESC) || (key == KEY_ENTER)) break;
    if (key == KEY_DOWN) layermenusel++;
    if (key == KEY_UP) layermenusel--;
    if (layermenusel < 0) layermenusel = 5;
    if (layermenusel > 5) layermenusel = 0;
    if (key == KEY_I)
    {
      newsizey -= 10;
      if (newsizey < MINYSIZE) newsizey = 0;
    }
    if (key == KEY_J)
    {
      newsizex -= 10;
      if (newsizex < MINXSIZE) newsizex = 0;
    }
    if (key == KEY_M)
    {
      if (newsizey) newsizey += 10;
      else newsizey = MINYSIZE;
    }
    if (key == KEY_K)
    {
      if (newsizex) newsizex += 10;
      else newsizex = MINXSIZE;
    }
    if (key == KEY_RIGHT)
    {
      switch(layermenusel)
      {
        case 0:
        if (newsizex) newsizex++;
        else newsizex = MINXSIZE;
        break;

        case 1:
        if (newsizey) newsizey++;
        else newsizey = MINYSIZE;
        break;

        /*
        case 2:
        map_layer[cl].xdivisor++;
        break;

        case 3:
        map_layer[cl].ydivisor++;
        break;

        case 4:
        map_layer[cl].xwrap ^= 1;
        break;

        case 5:
        map_layer[cl].ywrap ^= 1;
        break;
        */
      }
    }
    if (key == KEY_LEFT)
    {
      switch(layermenusel)
      {
        case 0:
        if (newsizex) newsizex--;
        if (newsizex < MINXSIZE) newsizex = 0;
        break;

        case 1:
        if (newsizey) newsizey--;
        if (newsizey < MINYSIZE) newsizey = 0;
        break;
        /*
        case 2:
        if (map_layer[cl].xdivisor) map_layer[cl].xdivisor--;
        break;

        case 3:
        if (map_layer[cl].ydivisor) map_layer[cl].ydivisor--;
        break;

        case 4:
        map_layer[cl].xwrap ^= 1;
        break;

        case 5:
        map_layer[cl].ywrap ^= 1;
        break;
        */
      }
    }

    gfx_fillscreen(0);
    sprintf(printbuffer, "LAYER %d OPTIONS", cl+1);
    txt_printcenter(40, SPR_EDITORFONT, printbuffer);
    sprintf(printbuffer, "X SIZE %04d", newsizex);
    txt_printcenter(60, SPR_EDITORFONT, printbuffer);
    sprintf(printbuffer, "Y SIZE %04d", newsizey);
    txt_printcenter(70, SPR_EDITORFONT, printbuffer);
    sprintf(printbuffer, "X DIVISOR %03d", map_layer[cl].xdivisor);
    txt_printcenter(80, SPR_EDITORFONT, printbuffer);
    sprintf(printbuffer, "Y DIVISOR %03d", map_layer[cl].ydivisor);
    txt_printcenter(90, SPR_EDITORFONT, printbuffer);
    if (map_layer[cl].xwrap) txt_printcenter(100, SPR_EDITORFONT, "X WRAP ON");
    else txt_printcenter(100, SPR_EDITORFONT, "X WRAP OFF");
    if (map_layer[cl].ywrap) txt_printcenter(110, SPR_EDITORFONT, "Y WRAP ON");
    else txt_printcenter(110, SPR_EDITORFONT, "Y WRAP OFF");
    gfx_drawsprite(100, 60+layermenusel*10, 0x000c0004);
    gfx_updatepage();
  }

  if ((newsizex != map_layer[cl].xsize) || (newsizey != map_layer[cl].ysize))
  {
    resizelayer(cl, newsizex, newsizey);
  }
  mark = 0;
  copybuffer = 0;
}

void resizelayer(int l, int newxsize, int newysize)
{
  int x,y;
  if ((newxsize > MAXMAPSIZE) || (newysize > MAXMAPSIZE)) return;
  memcpy(temparea, map_layerdataptr[l], map_layer[l].xsize*map_layer[l].ysize*2);
  for (y = 0; y < newysize; y++)
  {
    for (x = 0; x < newxsize; x++)
    {
      map_layerdataptr[l][y*newxsize+x] = blk;
    }
  }
  for (y = 0; y < map_layer[l].ysize; y++)
  {
    for (x = 0; x < map_layer[l].xsize; x++)
    {
      map_layerdataptr[l][y*newxsize+x] = temparea[y*map_layer[l].xsize+x];
    }
  }
  map_layer[l].xsize = newxsize;
  map_layer[l].ysize = newysize;
  mark = 0;
  copybuffer = 0;
}

void loadmission(void)
{
        int c;
        char *textptr;

	char mapnamebuf[80];
        char blknamebuf[80];
        char infnamebuf[80];
        char misnamebuf[80];

        stairindex = -1;
        enterclosetname = 0;

        if (!enterfilename("LOAD MISSION:", missionname)) return;

        initmap();

        strcpy(mapnamebuf, DIR_MISSIONS "/");
        strcpy(blknamebuf, DIR_MISSIONS "/");
        strcpy(infnamebuf, DIR_MISSIONS "/");
        strcpy(misnamebuf, DIR_MISSIONS "/");

        strcat(mapnamebuf, missionname);
        strcat(blknamebuf, missionname);
        strcat(infnamebuf, missionname);
        strcat(misnamebuf, missionname);

        strcat(mapnamebuf, ".map");
        strcat(blknamebuf, ".blk");
        strcat(infnamebuf, ".inf");
        strcat(misnamebuf, ".mis");

        /* Load the mission */
        ownloadmap(mapnamebuf);
        gfx_loadblocks(blknamebuf);
        map_loadblockinfo(infnamebuf);

        if (!openmf(misnamebuf)) return;

        /* Read in player start pos. */
        bofhstartx = mfreadint();
        bofhstarty = mfreadint();
        bofhstartangle = mfreadint();

        /* Read in dimensions of server room */
        srminx = mfreadint();
        srminy = mfreadint();
        srmaxx = mfreadint();
        srmaxy = mfreadint();

        /* Read in victory condition & texts */
        victorybits = mfreadint();

        memset(briefingtext, sizeof briefingtext, 0);
	textptr = briefingtext;
	for (;;)
	{
		mfreadstring(textptr, 80);
		if (*textptr == '$')
		{
		        *textptr = 0;
		        break;
                }
		textptr += 80;
	}
        memset(victorytext, 0, sizeof victorytext);
	textptr = victorytext;
	for (;;)
	{
		mfreadstring(textptr, 80);
		if (*textptr == '$')
		{
		        *textptr = 0;
		        break;
                }
		textptr += 80;
	}

        /* Read in number of first damaged block */
        firstdamagedblock = mfreadint();

        /* Read in number of bombs, enemies, items etc. */
        for (c = 0; c < MAX_DIFF; c++)  begintime[c] = mfreadint();
        for (c = 0; c < MAX_DIFF; c++)	beginbombs[c] = mfreadint();
        for (c = 0; c < MAX_DIFF; c++)	beginterrorists[c] = mfreadint();
        for (c = 0; c < MAX_DIFF; c++)	beginpistolmen[c] = mfreadint();
        for (c = 0; c < MAX_DIFF; c++)	beginshotgunmen[c] = mfreadint();
        for (c = 0; c < MAX_DIFF; c++)	beginuzimen[c] = mfreadint();
        for (c = 0; c < MAX_DIFF; c++)	beginsadists[c] = mfreadint();
        for (c = 0; c < MAX_DIFF; c++)	beginleaders[c] = mfreadint();
        for (c = 0; c < MAX_DIFF; c++)	beginwhips[c] = mfreadint();
        for (c = 0; c < MAX_DIFF; c++)	begindrills[c] = mfreadint();
        for (c = 0; c < MAX_DIFF; c++)	begincrossbows[c] = mfreadint();
        for (c = 0; c < MAX_DIFF; c++)	beginpistols[c] = mfreadint();
        for (c = 0; c < MAX_DIFF; c++)	beginshotguns[c] = mfreadint();
        for (c = 0; c < MAX_DIFF; c++)	beginuzis[c] = mfreadint();
        for (c = 0; c < MAX_DIFF; c++)	begingrenades[c] = mfreadint();
        for (c = 0; c < MAX_DIFF; c++)	beginbazookas[c] = mfreadint();
        for (c = 0; c < MAX_DIFF; c++)	beginscanners[c] = mfreadint();
        for (c = 0; c < MAX_DIFF; c++)	beginsmallmedikits[c] = mfreadint();
        for (c = 0; c < MAX_DIFF; c++)	beginbigmedikits[c] = mfreadint();

        /* Read in enemy attack probabilities */
        for (c = 0; c < MAX_DIFF; c++) fistpr[c] = 64-mfreadint();
        for (c = 0; c < MAX_DIFF; c++) pistolpr[c] = 64-mfreadint();
        for (c = 0; c < MAX_DIFF; c++) shotgunpr[c] = 64-mfreadint();
        for (c = 0; c < MAX_DIFF; c++) uzipr[c] = 64-mfreadint();
        for (c = 0; c < MAX_DIFF; c++) crossbowpr[c] = 64-mfreadint();
        for (c = 0; c < MAX_DIFF; c++) flamepr[c] = 64-mfreadint();

        /* Read in stairs */
        memset(&stairs[0], 0, sizeof stairs);
        for (c=0;;c++)
        {
                stairs[c].src.xb = mfreadint();
                stairs[c].src.yb = mfreadint();
                stairs[c].src.angle = mfreadint();
                stairs[c].dest.xb = mfreadint();
                stairs[c].dest.yb = mfreadint();
                stairs[c].dest.angle = mfreadint();
                if ((!stairs[c].src.xb) && (!stairs[c].src.yb)) break;
        }

        /* Read in closets */
        memset(&closet[0], 0, sizeof closet);
        for (c=0;;c++)
        {
        	int d;
        	closet[c].xb = mfreadint();
        	closet[c].yb = mfreadint();
        	closet[c].anim = mfreadint();
        	if ((!closet[c].xb) && (!closet[c].yb)) break;
                memset(closet[c].name, 0, 16);
        	mfreadstring(closet[c].name, 16);
        	for (d = 0; d < MAX_EQUIP; d++) closet[c].equipment[d] = mfreadint();
        }

        /* Read in lifts */
        memset(&lift[0], 0, sizeof lift);
        for (c=0;;c++)
        {
                int d;

                lift[c].floors = mfreadint();
                lift[c].speed = mfreadint();
                lift[c].startdelay = mfreadint();
                lift[c].firstfloor = mfreadint();
                lift[c].angle = mfreadint();
                if (!lift[c].floors) break;
                for (d = 0; d < lift[c].floors; d++)
                {
                        lift[c].liftfloor[d].xb = mfreadint();
                        lift[c].liftfloor[d].yb = mfreadint();
                }
        }

        memset(&eloc[0], 0, sizeof eloc);
        /* Read in stationary actors */
        for (c=0;;c++)
        {
        	eloc[c].xb = mfreadint();
        	eloc[c].yb = mfreadint();
        	eloc[c].type = mfreadint();
        	eloc[c].angle = mfreadint();
        	if (!eloc[c].type) break;
        }

        /* Read in random actors */
        memset(&rloc[0], 0, sizeof rloc);
        numrloc = 0;
	readrloc(ACTOR_CAT5);
	readrloc(ACTOR_BND);
	readrloc(ACTOR_CROSSBOW);
	readrloc(ACTOR_PISTOL);
	readrloc(ACTOR_SHOTGUN);
	readrloc(ACTOR_UZI);
	readrloc(ACTOR_GRENADE);
	readrloc(ACTOR_BAZOOKA);
	readrloc(ACTOR_SCANNER);
	readrloc(ACTOR_BIGMEDIKIT);
	readrloc(ACTOR_LEADER);
	readrloc(ACTOR_PISTOLMAN);

	xpos = (bofhstartx-10)*16;
	ypos = (bofhstarty-7)*16;

        closemf();
}

int ownloadmap(char *name)
{
  	int handle, c;

        handle = io_open(name);
        if (handle == -1) return 0;
        /* Load map header */
        io_read(handle, &map_header, sizeof(MAPHEADER));
        /* Load each layer */
        for (c = 0; c < MAX_LAYERS; c++)
        {
                map_layer[c].xsize = io_readle32(handle);
                map_layer[c].ysize = io_readle32(handle);
                map_layer[c].xdivisor = io_read8(handle);
                map_layer[c].ydivisor = io_read8(handle);
                map_layer[c].xwrap = io_read8(handle);
                map_layer[c].ywrap = io_read8(handle);
                if ((map_layer[c].xsize) && (map_layer[c].ysize))
                {
                        int d;
                        map_layerdataptr[c] = malloc(map_layer[c].xsize*map_layer[c].ysize * sizeof(Uint16));
                        if (!map_layerdataptr[c])
                        {
                                io_close(handle);
                                map_freemap();
                                return 0;
                        }
                        for (d = 0; d < map_layer[c].xsize * map_layer[c].ysize; d++)
                        {
                                map_layerdataptr[c][d] = io_readle16(handle);
                        }
                }
        }
        io_close(handle);
	return 1;
}

void savemission(void)
{
        int c;
        FILE *handle;
        int tempint;
        char *textptr;

        char endstring[] = "$";
	char mapnamebuf[80];
        char blknamebuf[80];
        char infnamebuf[80];
        char misnamebuf[80];

        stairindex = -1;
        enterclosetname = 0;

        if (!enterfilename("SAVE MISSION:", missionname)) return;

        strcpy(mapnamebuf, DIR_MISSIONS "/");
        strcpy(blknamebuf, DIR_MISSIONS "/");
        strcpy(infnamebuf, DIR_MISSIONS "/");
        strcpy(misnamebuf, DIR_MISSIONS "/");

        strcat(mapnamebuf, missionname);
        strcat(blknamebuf, missionname);
        strcat(infnamebuf, missionname);
        strcat(misnamebuf, missionname);

        strcat(mapnamebuf, ".map");
        strcat(blknamebuf, ".blk");
        strcat(infnamebuf, ".inf");
        strcat(misnamebuf, ".mis");

        /* Save map */
        handle = fopen(mapnamebuf, "wb");
        if (handle)
        {
          /* Write map header */
          fwrite(&map_header, sizeof(MAPHEADER), 1, handle);
          /* Write each layer */
          for (c = 0; c < MAX_LAYERS; c++)
          {
            fwritele32(handle, map_layer[c].xsize);
            fwritele32(handle, map_layer[c].ysize);
            fwrite8(handle, map_layer[c].xdivisor);
            fwrite8(handle, map_layer[c].ydivisor);
            fwrite8(handle, map_layer[c].xwrap);
            fwrite8(handle, map_layer[c].ywrap);
            if ((map_layer[c].xsize) && (map_layer[c].ysize))
            {
              int d;
              for (d = 0; d < map_layer[c].xsize * map_layer[c].ysize; d++)
              {
                fwritele16(handle, map_layerdataptr[c][d]);
              }
            }
          }
          fclose(handle);
        }

        /* Save blockinfo */
        if (map_blkinfdata)
        {
                handle = fopen(infnamebuf, "wb");
                if (handle)
                {
                        fwrite(&map_blkinfdata[0], gfx_nblocks*16, 1, handle);
                        fclose(handle);
                }
        }

        /* Save mission file */
        handle = fopen(misnamebuf, "wb");
        if (!handle) return;

        /* Write player start pos. */
        writele32(handle, bofhstartx);
        writele32(handle, bofhstarty);
        writele32(handle, bofhstartangle);

        /* Write dimensions of server room */
        writele32(handle, srminx);
        writele32(handle, srminy);
        writele32(handle, srmaxx);
        writele32(handle, srmaxy);

        /* Write victory condition & texts */
        writele32(handle, victorybits);

        for (c = 0; c < MAXMSGROW; c++)
        {
                int len;

        	textptr = briefingtext + 80*c;
                len = strlen(textptr);
                fwrite(textptr, len+1, 1, handle);
        }
        fwrite(endstring, 2, 1, handle);
        for (c = 0; c < MAXMSGROW; c++)
        {
                int len;

        	textptr = victorytext + 80*c;
                len = strlen(textptr);
                fwrite(textptr, len+1, 1, handle);
        }
        fwrite(endstring, 2, 1, handle);

        /* Write number of first damaged block */
        writele32(handle, firstdamagedblock);

        /* Write number of bombs, enemies, items etc. */
        for (c = 0; c < MAX_DIFF; c++) writele32(handle, begintime[c] );
        for (c = 0; c < MAX_DIFF; c++) writele32(handle, beginbombs[c] );
        for (c = 0; c < MAX_DIFF; c++) writele32(handle, beginterrorists[c] );
        for (c = 0; c < MAX_DIFF; c++) writele32(handle, beginpistolmen[c] );
        for (c = 0; c < MAX_DIFF; c++) writele32(handle, beginshotgunmen[c] );
        for (c = 0; c < MAX_DIFF; c++) writele32(handle, beginuzimen[c] );
        for (c = 0; c < MAX_DIFF; c++) writele32(handle, beginsadists[c] );
        for (c = 0; c < MAX_DIFF; c++) writele32(handle, beginleaders[c] );
        for (c = 0; c < MAX_DIFF; c++) writele32(handle, beginwhips[c] );
        for (c = 0; c < MAX_DIFF; c++) writele32(handle, begindrills[c] );
        for (c = 0; c < MAX_DIFF; c++) writele32(handle, begincrossbows[c] );
        for (c = 0; c < MAX_DIFF; c++) writele32(handle, beginpistols[c] );
        for (c = 0; c < MAX_DIFF; c++) writele32(handle, beginshotguns[c] );
        for (c = 0; c < MAX_DIFF; c++) writele32(handle, beginuzis[c] );
        for (c = 0; c < MAX_DIFF; c++) writele32(handle, begingrenades[c] );
        for (c = 0; c < MAX_DIFF; c++) writele32(handle, beginbazookas[c] );
        for (c = 0; c < MAX_DIFF; c++) writele32(handle, beginscanners[c] );
        for (c = 0; c < MAX_DIFF; c++) writele32(handle, beginsmallmedikits[c] );
        for (c = 0; c < MAX_DIFF; c++) writele32(handle, beginbigmedikits[c] );

        /* Write enemy attack probabilities */
        for (c = 0; c < MAX_DIFF; c++)
        {
                tempint = 64-fistpr[c];
                writele32(handle, tempint);
        }
        for (c = 0; c < MAX_DIFF; c++)
        {
                tempint = 64-pistolpr[c];
                writele32(handle, tempint);
        }
        for (c = 0; c < MAX_DIFF; c++)
        {
                tempint = 64-shotgunpr[c];
                writele32(handle, tempint);
        }
        for (c = 0; c < MAX_DIFF; c++)
        {
                tempint = 64-uzipr[c];
                writele32(handle, tempint);
        }
        for (c = 0; c < MAX_DIFF; c++)
        {
                tempint = 64-crossbowpr[c];
                writele32(handle, tempint);
        }
        for (c = 0; c < MAX_DIFF; c++)
        {
                tempint = 64-flamepr[c];
                writele32(handle, tempint);
        }

        /* Write stairs */
        for (c=0;c<MAX_STAIRS;c++)
        {
                if ((stairs[c].src.xb) && (stairs[c].src.yb))
                {
                        writele32(handle, stairs[c].src.xb);
                        writele32(handle, stairs[c].src.yb);
                        writele32(handle, stairs[c].src.angle);
                        writele32(handle, stairs[c].dest.xb);
                        writele32(handle, stairs[c].dest.yb);
                        writele32(handle, stairs[c].dest.angle);
                }
        }
        writele32(handle, 0);
        writele32(handle, 0);
        writele32(handle, 0);
        writele32(handle, 0);
        writele32(handle, 0);
        writele32(handle, 0);

        /* Write closets */
        for (c=0;c<MAX_CLOSET;c++)
        {
                int d;
                if ((closet[c].xb) && (closet[c].yb))
                {
                        writele32(handle, closet[c].xb);
                        writele32(handle, closet[c].yb);
                        writele32(handle, closet[c].anim);
                        fwrite(closet[c].name, strlen(closet[c].name)+1, 1, handle);
                	for (d = 0; d < MAX_EQUIP; d++) writele32(handle, closet[c].equipment[d]);
                }
        }
        writele32(handle, 0);
        writele32(handle, 0);
        writele32(handle, 0);

        /* Write lifts */
        for (c=0;c<MAX_LIFT;c++)
        {
                int d;
                if (lift[c].floors)
                {
                        writele32(handle, lift[c].floors);
                        writele32(handle, lift[c].speed);
                        writele32(handle, lift[c].startdelay);
                        writele32(handle, lift[c].firstfloor);
                        writele32(handle, lift[c].angle);
                        for (d = 0; d < lift[c].floors; d++)
                        {
                                writele32(handle, lift[c].liftfloor[d].xb);
                                writele32(handle, lift[c].liftfloor[d].yb);
                        }
                }
        }
        tempint = 0;
        writele32(handle, 0);
        writele32(handle, 0);
        writele32(handle, 0);
        writele32(handle, 0);
        writele32(handle, 0);

        /* Write stationary actors */
        for (c=0;c<MAX_ACTOR;c++)
        {
                if (eloc[c].type)
                {
                        writele32(handle, eloc[c].xb);
                        writele32(handle, eloc[c].yb);
                        writele32(handle, eloc[c].type);
                        writele32(handle, eloc[c].angle);
                }
        }
        writele32(handle, 0);
        writele32(handle, 0);
        writele32(handle, 0);
        writele32(handle, 0);

        /* Write random actors */
	writerloc(ACTOR_CAT5, handle);
	writerloc(ACTOR_BND, handle);
	writerloc(ACTOR_CROSSBOW, handle);
	writerloc(ACTOR_PISTOL, handle);
	writerloc(ACTOR_SHOTGUN, handle);
	writerloc(ACTOR_UZI, handle);
	writerloc(ACTOR_GRENADE, handle);
	writerloc(ACTOR_BAZOOKA, handle);
	writerloc(ACTOR_SCANNER, handle);
	writerloc(ACTOR_BIGMEDIKIT, handle);
	writerloc(ACTOR_LEADER, handle);
	writerloc(ACTOR_PISTOLMAN, handle);
        fclose(handle);
}

void readrloc(int type)
{
        for(;;)
        {
                rloc[numrloc].xb = mfreadint();
                rloc[numrloc].yb = mfreadint();
                if ((!rloc[numrloc].xb) && (!rloc[numrloc].yb)) break;
                rloc[numrloc].type = type;
                numrloc++;
        }
}

void writerloc(int type, FILE* handle)
{
        int c;

        for (c=0;c<MAX_ACTOR;c++)
        {
                if ((rloc[c].type == type) && (rloc[c].xb) && (rloc[c].yb))
                {
                        writele32(handle, rloc[c].xb);
                        writele32(handle, rloc[c].yb);
                }
        }
        writele32(handle, 0);
        writele32(handle, 0);
}

int initstuff(void)
{
  int c;

  FILE *handle = fopen(DIR_USERCFG "/bofh.cfg", "rb");
  win_fullscreen = 1;
  if (handle)
  {
    int sightline;

    fread(&sightline, sizeof sightline, 1, handle);
    fread(&screenmode, sizeof screenmode, 1, handle);
    fread(&win_fullscreen, sizeof win_fullscreen, 1, handle);
    fclose(handle);
  }

  kbd_init();
  if (!win_openwindow("BOFH Editor", NULL)) return 0;
  win_setmousemode(MOUSE_ALWAYS_HIDDEN);
  if (!gfx_loadpalette(DIR_DATA "/bofh.pal"))
  {
    win_messagebox("Palette load error (bofh.pal)");
    return 0;
  }
  for (c = 0;; c++)
  {
         char spritenamebuf[80];

        if (!spritename[c]) break;
         strcpy(spritenamebuf, DIR_DATA "/");
         strcat(spritenamebuf, spritename[c]);

	if (!gfx_loadsprites(c, spritenamebuf))
	{
	    win_messagebox("Editor sprite load error");
	    return 0;
	}
  }
  if (!gfx_init(320,200,70,screenmode|GFX_USE3PAGES))
  {
    if (!gfx_init(320,200,70,screenmode|GFX_USEDIBSECTION))
    {
      win_messagebox("Graphics init error!");
      return 0;
    }
  }
  return 1;
}

int openmf(char *missionfilename)
{
        closemf();
        mf = fopen(missionfilename, "rb");
        if (!mf) return 0;
        return 1;
}

void closemf(void)
{
	if (mf) fclose(mf);
	mf = NULL;
}

int mfreadstring(char *dest, int length)
{
        unsigned char c;

       	char *orig = dest;
        while (--length)
        {
                c = fgetc(mf);
                if (!c) break;
                *dest++ = c;
        }
	*dest = 0;
	return strlen(orig);
}

int mfreadint(void)
{
  unsigned char bytes[4];

  fread(bytes, 4, 1, mf);
  return (bytes[0]) | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24);
}

void updatemouse(void)
{
	prevmouseb = mouseb;

        mouseb = mou_getbuttons();
        mou_getmove(&mousemovex, &mousemovey);
}

int geteditspeed(void)
{
        int gamespeed;
	for (;;)
	{
		gamespeed = win_getspeed(70);
		if (gamespeed) break;
	}
	if (gamespeed > MAXFRAMESKIP) gamespeed = MAXFRAMESKIP;
        return gamespeed;
}

void write8(FILE *handle, unsigned data)
{
  char bytes[1];

  bytes[0] = data;
  fwrite(bytes, 1, 1, handle);
}

void writele16(FILE *handle, unsigned data)
{
  char bytes[2];

  bytes[0] = data;
  bytes[1] = data >> 8;
  fwrite(bytes, 2, 1, handle);
}

void writele32(FILE *handle, unsigned data)
{
  char bytes[4];

  bytes[0] = data;
  bytes[1] = data >> 8;
  bytes[2] = data >> 16;
  bytes[3] = data >> 24;
  fwrite(bytes, 4, 1, handle);
}



