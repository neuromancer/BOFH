/*
 * BOFH: Servers Under Siege V1.5
 * ------------------------------
 * Action game by Lasse Öörni (Cadaver), Kalle Niemitalo & Olli Niemitalo
 * (Yehar) in year 2000. Music by Olli Niemitalo & Tuomas Mäkelä (Kuunvarjo).
 */

#define CHANNELS 32

#include "bofh.h"
#include "extern.h"
#include "sincos.h"
#include "keybinds.h"

ACTOR actor[MAX_ACTOR];
BOMB bomb[MAX_BOMB];
CLOSET closet[MAX_CLOSET];
LIFT lift[MAX_LIFT];
STAIRS stairs[MAX_STAIRS];

// ***
struct {
  ACTOR *owner;
  int vol;
} channelextras[CHANNELS];

unsigned mixrate = 22050;
unsigned mixmode = STEREO|SIXTEENBIT;

int numstairs = 0;
int numclosets = 0;
int numlifts = 0;
int numbombs = 0;

int srminx, srminy, srmaxx, srmaxy;

int prevtune = -1;
int bombs = 0;
int instructions[MAX_BOMB];
ACTOR *speaker;
int weapon;
int screenmode = 0;
int directsound = 0;
int gametime;
int showinstr;
int showinstrtime;
int beginservers;
int beginworkstations;
int paused = 0;
int score = 0;
int terrorists = 0;
int leaders = 0;
int computers = 0;
int gameover = 0;
int sightline = 0;
int musicvolume = 64;
int sfxvolume = 96;
int xpos, ypos;
char mutesound = 0;
char mouseinitted = 0;
int controlscheme = CTRL_ABSOLUTE;
unsigned mouseposx = 0;
unsigned mouseposy = 0;
int mousemovex = 0;
int mousemovey = 0;
int prevmovex = 0;
int prevmovey = 0;
int avgmovex = 0;
int avgmovey = 0;
unsigned mouseb = 0;
unsigned prevmouseb = 0;
int difficulty = DIFF_INSANE;
int gamespeed;
char *gamemsg;
int trapmsg = 0;
int gamemsgtime = 0;
int lifecheat = 0;
int timecheat = 0;
int ammocheat = 0;
int speedcheat = 0;
int instrcheat = 0;
int enemycheat = 0;
int grenadecheat = 0;
int victory = 0;
int missionindex = 0;
int nummissions = 0;
int key;
int trycheatstring = -1;
unsigned trycheatindex = 0;
int xshift = 0;
int yshift = 0;
int throwstrength;
int fastforward;
int liftnumber;
int lifthere;
int liftsound;
int scannerdelay;
int kills;
int actors;
int victorybits;
int firstdamagedblock;

unsigned char *firebuf;
char textbuf[80];

char briefingtext[MAXBRIEFINGLENGTH];
char victorytext[MAXBRIEFINGLENGTH];
char missionlist[MAXMISSIONS][13];

HISCORE_ENTRY hiscore[] = {{"BURZUM", 10000},
			     {"IMMORTAL", 9000},
			     {"MAYHEM", 8000},
			     {"DARKTHRONE", 7000},
			     {"ISENGARD", 6000},
			     {"ABIGOR", 5000},
                             {"NECROMANTIA", 4000},
                             {"ABSU", 3000},
			     {"TROLL", 2000},
                             {"ENSLAVED", 1000}};

char *menutext[] = {"START GAME", "OPTIONS", "HIGHSCORE", "INTRO", "EXIT"};
char *difftext[] = {"PRACTICE", "EASY", "MEDIUM", "HARD", "INSANE"};
char *keytext[] = {
	"MOVE FWD", "MOVE BWD", "TURN LEFT", "TURN RIGHT", "STRAFE LEFT",
	"STRAFE RIGHT", "STRAFE KEY", "WALK KEY", "ATTACK", "NEXT WEAPON",
	"PREV WEAPON", "PAUSE KEY", "TOGGLE SIGHT LINE", "TOGGLE MUSIC",
	"VIEW NOTES", "CUT RED", "CUT GREEN", "CUT BLUE", "CUT YELLOW",
	"MOUSE ATTACK",	"MOUSE NEXT WEAPON", "MOUSE PREV WEAPON"
};
SAMPLE *smp[MAX_SMP];
char *samplename[] = {
        "taunt1.smp","taunt2.smp","taunt3.smp","taunt4.smp","taunt5.smp","taunt6.smp",
	"hit1.smp","hit2.smp","hit3.smp","hit4.smp","hit5.smp","hit6.smp",
        "die1.smp","die2.smp","die3.smp",
	"pistol.smp", "shotgun.smp", "uzi.smp", "explode.smp",
        "fist1.smp", "fist2.smp", "fist3.smp", "fist4.smp",
        "swish.smp", "klonk.smp",
        "bder-on.smp", "bder-hit.smp", "bder-run.smp", "bder-off.smp",
        "gunload.smp",
        "shell1.smp", "shell2.smp",
        "ricoch1.smp", "ricoch2.smp", "glass.smp", "liftstrt.smp", "liftstop.smp",
        "bazooka1.smp", "bazload.smp", "crossbow.smp", "arrowhit.smp",
        NULL
};
SAMPLELOOPPOINT repeats[] = {
                { SMP_BND_ON,  0x48BE },
                { SMP_BND_HIT, 0x29A9 },
                { SMP_BND_RUN, 0x02FC },
                { SMP_LIFTSTART, 0x4B90 }
};

char *spritename[] = {
	"bigfnt.spr", "player.spr", "weapon.spr", "machine.spr",
        "fistman.spr", "gunman.spr", "shotgman.spr", "uziman.spr",
        "tech.spr", "closet.spr", "leader.spr", "sadist.spr",
        "title.spr", "smallfnt.spr", "blackfnt.spr", NULL
};

int ammo[WEAPNUM];                /* FIST CAT5 B&D  CROS PIST SHOT UZI GREN BAZO SCAN */
const int maxammo[WEAPNUM]        = {999, 999, 999, 50,  200, 100, 200, 20,   5, 999};
const int attackdelaytbl[WEAPNUM] = { 15,  20,   2, 40,  20,  25,    5, 35,  70, 20};
int actorradius[] = {
        /* Empty actor */
        0,
        /* The BOFH */
	6*DEC,
	/* Muzzle, bullet, smoke */
	0, 0, 0,
	/* Workstations + printers */
	5*DEC, 7*DEC, 7*DEC, 4*DEC, 7*DEC, 5*DEC, 5*DEC, 8*DEC,
	/* Servers */
	7*DEC, 5*DEC, 8*DEC,
	/* Various small objects + explosions */
	0, 0, 0, 0, 0, 0,
        /* Enemies */
	8*DEC, 8*DEC, 8*DEC, 8*DEC, 8*DEC, 8*DEC, 8*DEC,
	/* Corpses */
	0, 0, 0, 0, 0, 0, 0, 0,
	/* Collectable items */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // ***
	/* Studio equipment :-) */
	4*DEC, 4*DEC, 4*DEC,
        /* Shells & ricochets */
        0, 0, 0,
        /* Laser beam */
        0,
        /* Flame */
        0,
        /* Glass shard */
        0,
        /* Grenades */
        0, 0,
        /* Bazooka projectiles, used bazookas *** */
        0, 0, 0,
        /* Crossbow, arrow & scanner */
        0, 0, 0
	};

/* Enemy attack probability for each difficulty
 * (64 = never, 63 = rarest, 0 = always :-) */
int fistpr[MAX_DIFF];
int pistolpr[MAX_DIFF];
int shotgunpr[MAX_DIFF];
int uzipr[MAX_DIFF];
int crossbowpr[MAX_DIFF];
int flamepr[MAX_DIFF];

int comphealth[] = {5, 5, 5, 5, 5, 5, 5, 15, 10, 10, 10};
int enemyhealth[] = {15, 10, 15, 10, 13, 10, 40};
int enemyscore[] = {350, 750, 1000, 1500, 2000, 2000, 5000};
int deadenemytype[] = {ACTOR_DEADFISTMAN, ACTOR_DEADPISTOLMAN,
	ACTOR_DEADSHOTGUNMAN, ACTOR_DEADUZIMAN, ACTOR_DEADTECHNICIAN,
	ACTOR_DEADSADIST, ACTOR_DEADLEADER};
int deadenemyitem[] = {ACTOR_NONE, ACTOR_PISTOL, ACTOR_SHOTGUN, ACTOR_UZI,
	ACTOR_NONE, ACTOR_CROSSBOW, ACTOR_NONE};
int bnd_hittime;
enum bnd_sound bnd_sound;

/* All cheat strings must begin with different letters */
#define NUMCHEATS 7
char *cheatstring[] = {
	"QBOPLTJB",
	"WPJNBB",
	"OPQFVUUB",
	"BJLBB",
	"PIKFJUB",
	"UBQFUUBWBB",
	"TJSQBMFJUB"};
int *cheatvalue[] = {&ammocheat, &lifecheat, &speedcheat, &timecheat, &instrcheat, &enemycheat, &grenadecheat};

int main(int argc, char **argv)
{
        int returnvalue = bofhmain();
        return returnvalue;
}

int bofhmain(void)
{
        win_fullscreen = 1;

        loadhiscore();
        loadconfig();

	/* Perform initializations */
        if (!initstuff()) return 666;

        /* Show splash screen */
        showsplash();

	/* Go to title screen! */
        titlescreen();
        saveconfig();

        return 0;
}

void getgamespeed(void)
{
	for (;;)
	{
		gamespeed = win_getspeed(70);
		if (gamespeed) break;
	}
	if (gamespeed > MAXFRAMESKIP) gamespeed = MAXFRAMESKIP;
}
void showsplash(void)
{
        int handle;
        int delay = 0;
	char *splashbuf = malloc(64000);
	if (!splashbuf) return;

	handle = io_open(DIR_DATA "/splash.raw");
	if (handle == -1)
	{
		free(splashbuf);
		return;
	}
	io_read(handle, splashbuf, 64000);
	io_close(handle);

	gfx_loadpalette(DIR_DATA "/splash.pal");
        memcpy(gfx_vscreen, splashbuf, 320*200);
        gfx_setpalette();
        gfx_updatepage();
	playfx(24, SMP_EXPLODE, 22000, 20, 64);
	playfx(25, SMP_EXPLODE, 22050, 20, 128);
	playfx(26, SMP_EXPLODE, 22100, 20, 192);
        getgamespeed();

	for (;;)
	{
	        updatemouse();
	        getgamespeed();
                delay += gamespeed;
                if (kbd_getkey()) break;
                if (delay > 4*70) break;
        }
        free(splashbuf);
        for (delay = 0; delay < 3; delay++)
        {
		gfx_fillscreen(0);
		gfx_updatepage();
	}
}

void checkglobalkeys(void)
{
        if (key == musickey)
        {
              	musicvolume ^= 64;
	       	snd_setmusicmastervolume(FIRSTFXCHAN, musicvolume);
	}
        if (key == KEY_F12)
        {
                screenmode++;
                if (screenmode > GFX_DOUBLESIZE) screenmode = 0;
	        if (!gfx_init(320,200,70,screenmode | GFX_USE3PAGES))
        	{
                        if (!gfx_init(320,200,70,screenmode | GFX_USEDIBSECTION))
                        {
                		win_messagebox("Graphics init failed!");
                                saveconfig();
                        	exit(0);
                        }
        	}
        }
        if (key == KEY_F11)
        {
                directsound ^= 1;
        	snd_init(mixrate, mixmode, 150, CHANNELS, directsound);
        }

        if (win_quitted)
        {
                saveconfig();
                exit(0);
        }
}

void playmusic(int tune)
{
        if (prevtune == tune) return;
        prevtune = tune;

        switch(tune)
        {
                case MUSIC_MAIN:
                snd_loadxm(DIR_DATA "/sdriver.xm");
                snd_playxm(0);
                break;

                case MUSIC_HISCORE:
                snd_loadxm(DIR_DATA "/shining.xm");
                snd_playxm(0);
                break;
        }
}

void stopmusic(void)
{
        snd_stopxm();
        prevtune = -1;
}

void titlescreen(void)
{
	int phase = TITLE_PRESENTS;
        int phasetime = 0;
        int phasevar = 0;
        int c;
	int menuselection = 0;

        gfx_loadpalette(DIR_DATA "/bofh.pal");
        gfx_setpalette();

        playmusic(MUSIC_MAIN);

	memset(firebuf, 64, 32000);

        kbd_getascii();
	getgamespeed();
        updatemouse();
	for (;;)
	{
        	getgamespeed();
                updatemouse();
                key = kbd_getkey();

                checkglobalkeys();
		if (key == KEY_ESC)
		{
			return;
		}
		if ((key == KEY_SPACE) || (key == KEY_ENTER) ||
			((mouseb & MOUSEB_LEFT) && (!(prevmouseb & MOUSEB_LEFT))))
		{
			playfx(FXCHAN_ENEMYSHOOT, SMP_SHOTGUN, 22050, 64, 128);
			menuselection = menu();
			if (menuselection == 0) return;
			if (menuselection == 1) phase = TITLE_HISCORE; 
			if (menuselection == 2) phase = TITLE_PRESENTS;
			
			phasetime = 0;
			phasevar = 0;
		}

		checkcheats();

		for (; gamespeed; gamespeed--)
		{
	                switch(phase)
	                {
	                	case TITLE_PRESENTS:
	                	if (phasetime < 300)
	                	{
	                                if (phasevar < 40) phasevar++;
	                        }
	                        else
	                        {
	                        	phasevar -= 2;
	                        }
	                        break;

	                	case TITLE_LOGO:
	                	if (phasetime < 300)
	                	{
	                                if (phasevar < 40)
	                                {
	                                	phasevar++;
	                                	if (phasevar == 40) playfx(FXCHAN_EXPLODE, SMP_EXPLODE, 22050, 64, 128);
	                                }
	                        }
	                        else
	                        {
	                        	phasevar++;
	                        }
	                        break;

	                        case TITLE_HISCORE:
	                        if (phasetime < 300)
	                        {
	                        	if (phasevar < 100) phasevar++;
	                        }
	                        else
	                        {
	                        	phasevar++;
	                        }
	                        break;

	                        case TITLE_CREDITS:
	                	if (phasetime < 300)
	                	{
	                                if (phasevar < 40) phasevar++;
	                        }
	                        else
	                        {
	                        	phasevar++;
	                        }
	                        break;

	                        case TITLE_CONTROLS:
	                        if (phasetime < 600)
	                        {
	                        	if (phasevar < 200) phasevar++;
	                        }
	                        else
	                        {
	                        	phasevar++;
	                	}
	                        break;

	                        case TITLE_RESOLUTION:
	                        if (phasetime < 600)
	                        {
	                        	if (phasevar < 200) phasevar++;
	                        }
	                        else
	                        {
	                        	phasevar++;
	                	}
	                        break;

	                }
	                phasetime++;
			if (phasetime > (phase < TITLE_CONTROLS ? 400 : 800))
	                {
                                phasetime = 0;
                                phasevar = 0;
                                phase++;
                                if (phase > TITLE_RESOLUTION) phase=TITLE_PRESENTS;
                                /*
                                 * Reload the scores every time, in
                                 * case another process has changed
                                 * them.
                                 */
                                if (phase == TITLE_HISCORE) loadhiscore();
		        }

	        }

	        fireeffect();

	        switch(phase)
	        {
	        	case TITLE_PRESENTS:
	                txt_printcenter(-50 + phasevar*3, SPR_FONTS, "ELECTRIC HAREM");
	                txt_printcenter(220 - phasevar * 3, SPR_FONTS, "PRESENTS");
	                break;

	                case TITLE_LOGO:
	                gfx_drawsprite(-160 + phasevar*8, 60, 0xc0001);
	                gfx_drawsprite(480 - phasevar*8, 140, 0xc0002);
	                break;

			case TITLE_HISCORE:
			txt_printcenter(200-phasevar*2, SPR_FONTS, "LEGENDARY BOFHS");
                        for (c = 0; c < 10; c++)
                        {
                        	sprintf(textbuf, "%02d. %-19s %06d", c+1, hiscore[c].name, hiscore[c].score);
				txt_printcenter(220-phasevar*2+c*17, SPR_FONTS, textbuf);
                        }
                        break;

                        case TITLE_CREDITS:
                        txt_print(-300+phasevar*8, 0, SPR_FONTS, "MAIN DESIGN & PROGRAMMING");
                        txt_print(340-phasevar*8, 20, SPR_FONTS, "LASSE \\\\RNI");
                        txt_print(-300+phasevar*8, 50, SPR_FONTS, "ADDITIONAL DESIGN & PROGRAMMING");
                        txt_print(340-phasevar*8, 70, SPR_FONTS, "KALLE NIEMITALO, OLLI NIEMITALO");
                        txt_print(-300+phasevar*8, 100, SPR_FONTS, "GRAPHICS & SOUND EFFECTS");
                        txt_print(340-phasevar*8, 120, SPR_FONTS, "LASSE \\\\RNI, OLLI NIEMITALO");
                        txt_print(-300+phasevar*8, 150, SPR_FONTS, "MUSIC");
                        txt_print(340-phasevar*8, 170, SPR_FONTS, "OLLI NIEMITALO, TUOMAS M[KEL[");
                        break;

                        case TITLE_CONTROLS:
                        txt_printcenter(200-phasevar, SPR_FONTS, "USE ARROW KEYS OR MOUSE TO CONTROL");
                        txt_printcenter(220-phasevar, SPR_FONTS, "THE MOVEMENT OF THE BOFH.");
                        txt_printcenter(240-phasevar, SPR_FONTS, " ");
                        txt_printcenter(260-phasevar, SPR_FONTS, "SPACE/LEFT MOUSEB.  = USE WEAPON   ");
                        txt_printcenter(280-phasevar, SPR_FONTS, "ENTER/RIGHT MOUSEB. = CHANGE WEAPON");
                        txt_printcenter(300-phasevar, SPR_FONTS, " ");
                        txt_printcenter(320-phasevar, SPR_FONTS, "ALT = STRAFE       SHIFT = SLOW MO.");
                        txt_printcenter(340-phasevar, SPR_FONTS, "P = PAUSE GAME     ESC = ABORT GAME");
                        txt_printcenter(360-phasevar, SPR_FONTS, "S = SIGHT-LINE     M = MUSIC ON/OFF");
                        txt_printcenter(380-phasevar, SPR_FONTS, "V = VIEW BOMB DEFUSING INSTRUCTIONS");
                        break;

                        case TITLE_RESOLUTION:
                        txt_printcenter(200-phasevar, SPR_FONTS, " PRESS F11 TO TOGGLE BETWEEN WAVE- ");
                        txt_printcenter(220-phasevar, SPR_FONTS, " OUT & DIRECTSOUND SOUND OUTPUT.   ");
                        txt_printcenter(240-phasevar, SPR_FONTS, " ");
                        txt_printcenter(260-phasevar, SPR_FONTS, " PRESS F12 TO TOGGLE BETWEEN THREE ");
                        txt_printcenter(280-phasevar, SPR_FONTS, " AVAILABLE SCREEN RESOLUTIONS:     ");
                        txt_printcenter(300-phasevar, SPR_FONTS, " ");
                        txt_printcenter(320-phasevar, SPR_FONTS, "   1X-SIZE   SCANLINES   2X-SIZE   ");
                        txt_printcenter(340-phasevar, SPR_FONTS, " ");
                        txt_printcenter(360-phasevar, SPR_FONTS, " PRESS ALT-ENTER TO TOGGLE BETWEEN ");
                        txt_printcenter(380-phasevar, SPR_FONTS, " FULLSCREEN AND WINDOWED DISPLAY.  ");
                        break;
	        }
	        gfx_updatepage();
	}
}

void loadconfig(void)
{
	FILE *handle = fopen(DIR_USERCFG "/bofh.cfg", "rb");
	if (!handle) return;
	fread(&sightline, sizeof sightline, 1, handle);
	fread(&screenmode, sizeof screenmode, 1, handle);
	fread(&win_fullscreen, sizeof win_fullscreen, 1, handle);
	fread(&directsound, sizeof directsound, 1, handle);
	fread(&upkey,       sizeof upkey,       1, handle);
        fread(&downkey,     sizeof downkey,     1, handle);
        fread(&leftkey,     sizeof leftkey,     1, handle);
        fread(&rightkey,    sizeof rightkey,    1, handle);
        fread(&strafeleft,  sizeof strafeleft,  1, handle);
        fread(&straferight, sizeof straferight,  1, handle);
        fread(&strafekey,   sizeof strafekey,   1, handle);
        fread(&walkkey,     sizeof walkkey,     1, handle);
        fread(&attackkey,   sizeof attackkey,   1, handle);
        fread(&nextweap,   sizeof nextweap,   1, handle);
	fread(&prevweap,   sizeof prevweap,     1, handle);
        fread(&pausekey,    sizeof pausekey,    1, handle);
        fread(&linekey,     sizeof linekey,     1, handle);
        fread(&musickey,    sizeof musickey,    1, handle);
        fread(&noteskey,    sizeof noteskey,    1, handle);
        fread(&redkey,      sizeof redkey,      1, handle);
        fread(&greenkey,    sizeof greenkey,    1, handle);
        fread(&bluekey,     sizeof bluekey,     1, handle);
        fread(&yellowkey,   sizeof yellowkey,   1, handle);
	fread(&mouseattack, sizeof mouseattack, 1, handle);
	fread(&mousenextweap, sizeof mousenextweap, 1, handle);
	fread(&mousenextweap, sizeof mousenextweap, 1, handle);
	fread(&mousesens,   sizeof mousesens,   1, handle);
	fclose(handle);
}

void saveconfig(void)
{
	FILE *handle = fopen(DIR_USERCFG "/bofh.cfg", "wb");
	if (!handle) return;
	fwrite(&sightline, sizeof sightline, 1, handle);
	fwrite(&screenmode, sizeof screenmode, 1, handle);
	fwrite(&win_fullscreen, sizeof win_fullscreen, 1, handle);
	fwrite(&directsound, sizeof directsound, 1, handle);
	fwrite(&upkey,       sizeof upkey,       1, handle);
        fwrite(&downkey,     sizeof downkey,     1, handle);
        fwrite(&leftkey,     sizeof leftkey,     1, handle);
        fwrite(&rightkey,    sizeof rightkey,    1, handle);
        fwrite(&strafeleft,  sizeof strafeleft,  1, handle);
        fwrite(&straferight, sizeof straferight, 1, handle);
        fwrite(&strafekey,   sizeof strafekey,   1, handle);
        fwrite(&walkkey,     sizeof walkkey,     1, handle);
        fwrite(&attackkey,   sizeof attackkey,   1, handle);
        fwrite(&nextweap,   sizeof nextweap,   1, handle);
	fwrite(&prevweap,   sizeof prevweap,     1, handle);
        fwrite(&pausekey,    sizeof pausekey,    1, handle);
        fwrite(&linekey,     sizeof linekey,     1, handle);
        fwrite(&musickey,    sizeof musickey,    1, handle);
        fwrite(&noteskey,    sizeof noteskey,    1, handle);
        fwrite(&redkey,      sizeof redkey,      1, handle);
        fwrite(&greenkey,    sizeof greenkey,    1, handle);
        fwrite(&bluekey,     sizeof bluekey,     1, handle);
        fwrite(&yellowkey,   sizeof yellowkey,   1, handle);
 	fwrite(&mouseattack, sizeof mouseattack, 1, handle);
	fwrite(&mousenextweap, sizeof mousenextweap, 1, handle);
	fwrite(&mouseprevweap, sizeof mouseprevweap, 1, handle);
	fwrite(&mousesens,   sizeof mousesens,   1, handle);
	fclose(handle);
}

int menu(void)
{
	int move = 0;
	int menuoption = 0;
	char flash = 0;
	int c;

	kbd_getascii();
	getgamespeed();
	updatemouse();
	for (;;)
	{
		getgamespeed();
		updatemouse();
		key = kbd_getkey();

		checkglobalkeys();
		if (key == KEY_ESC)
		{
			playfx(FXCHAN_ENEMYSHOOT, SMP_SHOTGUN, 22050, 64, 128);
			return 2;
		}
		if ((key == KEY_SPACE) || (key == KEY_ENTER) ||
				((mouseb & MOUSEB_LEFT) && (!(prevmouseb & MOUSEB_LEFT))))
		{
			playfx(FXCHAN_ENEMYSHOOT, SMP_SHOTGUN, 22050, 64, 128);
			if (menuoption == 0)
			{
				if (selectdifficulty())
				{
					for (;;)
					{
					score = 0;
					game(missionlist[missionindex]);
					if (((checkhiscore() == 1) && (restartdialog()))
							|| (restartdialog())) return 1;
					}
				}    
			}
			if (menuoption == 1) optionsmenu();
			if (menuoption == 2) return 1;
			if (menuoption == 3) return 2;
			if (menuoption == 4) return 0;
		}
		if (key == KEY_UP) move -= 64;
		if (key == KEY_DOWN) move += 64;

		move += mousemovey;
		while ((move >= 5*64) || (move < 0))
		{
			if (move >= 5*64) move -= 5*64;
			if (move < 0) move += 5*64;
		}

		menuoption = move / 64;

		for (; gamespeed; gamespeed--)
		{
			flash++;
		}

		fireeffect();
		for (c = 0; c < 5; c++)
		{
			if (((c == menuoption) && (flash & 16)) || (c != menuoption))
			{
				txt_printcenter(50+20*c, SPR_FONTS, menutext[c]);
			}
		}
		gfx_updatepage();
	}
}

int restartdialog(void)
{
	kbd_getascii();
	getgamespeed();
	updatemouse(); 
	for (;;)
	{
          	getgamespeed();
		updatemouse();
		key = kbd_getkey();

		checkglobalkeys();
		if ((key == KEY_N) || (key == KEY_ESC))  return 1;
	       
		if (key != 0) return 0;

                fireeffect();
 		txt_printcenter(80, SPR_FONTS, "RESTART? Y/N");
		gfx_updatepage();
	}
}

int selectdifficulty(void)
{
        int move = difficulty * 64 + 32;
        int movex = 0;
        char flash = 0;
        int c;
        DIR *dir;
        struct dirent *de;

        nummissions = 0;
        dir = opendir(DIR_MISSIONS);
        if (dir)
        {
        	while ((de = readdir(dir)))
        	{
                        char buf[256];
                        int len;

                        strcpy(buf, de->d_name);
	                len = strlen(buf);
	                if (len > 4)
	                {
                                if ((!strcmp(&buf[len-4], ".mis")) || (!strcmp(&buf[len-4], ".MIS")))
                                {
        	                        buf[len-4] = 0;
        	                        strcpy(&missionlist[nummissions][0], buf);
        	                        nummissions++;
				        if (!strcmp(buf,"original"))
				           missionindex = nummissions - 1;
				   
	                                if (nummissions >= MAXMISSIONS) break;
                                }
	                }
                }
                closedir(dir);
        }

        if (missionindex >= nummissions) missionindex = 0;

        if (!nummissions)
        {
                gfx_fillscreen(0);
                txt_printcenter(80, SPR_FONTS, "NO MISSION FILES FOUND!");
                gfx_updatepage();
        	getgamespeed();

                for (;;)
                {
	        	getgamespeed();
	                updatemouse();
	                key = kbd_getkey();
                        checkglobalkeys();
			if ((key == KEY_SPACE) || (key == KEY_ENTER) ||
				((mouseb & MOUSEB_LEFT) && (!(prevmouseb & MOUSEB_LEFT))))
			{
				playfx(FXCHAN_ENEMYSHOOT, SMP_SHOTGUN, 22050, 64, 128);
                         	break;
                        }
                }
                return 0;
        }

        kbd_getascii();
        getgamespeed();
        updatemouse();
	for (;;)
	{
	        getgamespeed();
                updatemouse();
                key = kbd_getkey();

	        checkglobalkeys();
		if (key == KEY_ESC)
		{
			playfx(FXCHAN_ENEMYSHOOT, SMP_SHOTGUN, 22050, 64, 128);
			return 0;
		}
		if ((key == KEY_SPACE) || (key == KEY_ENTER) ||
			((mouseb & MOUSEB_LEFT) && (!(prevmouseb & MOUSEB_LEFT))))
		{
			playfx(FXCHAN_ENEMYSHOOT, SMP_SHOTGUN, 22050, 64, 128);
			return 1;
		}
		if (key == KEY_UP) move -= 64;
		if (key == KEY_DOWN) move += 64;
		if (key == KEY_LEFT) missionindex--;
		if (key == KEY_RIGHT) missionindex++;

                move += mousemovey;
		while ((move >= 5*64) || (move < 0))
		{
			if (move >= 5*64) move -= 5*64;
			if (move < 0) move += 5*64;
		}
		movex += mousemovex;
                if (movex > 64)
                {
                        movex = 0;
                        missionindex++;
                }
                if (movex < -64)
                {
                        movex = 0;
                        missionindex--;
                }

                if ((mouseb & MOUSEB_RIGHT) && (!(prevmouseb & MOUSEB_RIGHT))) missionindex++;

                if (missionindex < 0) missionindex = nummissions-1;
                if (missionindex >= nummissions) missionindex = 0;

                difficulty = move / 64;

		checkcheats();

		for (; gamespeed; gamespeed--)
		{
			flash++;
		}

	        fireeffect();
		for (c = 0; c < 5; c++)
		{
                        sprintf(textbuf, "MISSION: %s", missionlist[missionindex]);
			txt_printcenter(20, SPR_FONTS, textbuf);
			if (((c == difficulty) && (flash & 16)) || (c != difficulty))
			{
				txt_printcenter(50+20*c, SPR_FONTS, difftext[c]);
			}
			txt_printcenter(160, SPR_FONTS, "LEFT & RIGHT TO SELECT MISSION");
		}
	        gfx_updatepage();
	}
}

int optionsmenu(void)
{
	int move = 0;
	int keyselect = 0;
	int keycode;
        unsigned buttoncode;
	char flash = 0;
	int c;

	kbd_getascii();
	getgamespeed();
	updatemouse();
	for (;;)
	{
		getgamespeed();
		updatemouse();
		key = kbd_getkey();

		checkglobalkeys();
		if (key == KEY_ESC)
		{
			playfx(FXCHAN_ENEMYSHOOT, SMP_SHOTGUN, 22050, 64, 128);
			return 0;
		}
		if ((key == KEY_SPACE) || (key == KEY_ENTER) ||
				((mouseb & MOUSEB_LEFT) && (!(prevmouseb & MOUSEB_LEFT))))
		{
			playfx(FXCHAN_ENEMYSHOOT, SMP_SHOTGUN, 22050, 64, 128);
			if (keyselect > 18)
			{
				buttoncode = buttondialog();
				if (keyselect == 19) mouseattack = buttoncode;
				if (keyselect == 20) mousenextweap = buttoncode;
				if (keyselect == 21) mouseprevweap = buttoncode;
			}
			else
			{
				keycode = keydialog();
				if (keycode != 0)
				{
					if (keyselect == 0) upkey = keycode;
					if (keyselect == 1) downkey = keycode;
					if (keyselect == 2) leftkey = keycode;
					if (keyselect == 3) rightkey = keycode;
					if (keyselect == 4) strafeleft = keycode;
					if (keyselect == 5) straferight = keycode;
					if (keyselect == 6) strafekey = keycode;
					if (keyselect == 7) walkkey = keycode;
					if (keyselect == 8) attackkey = keycode;
					if (keyselect == 9) nextweap = keycode;
					if (keyselect == 10) prevweap = keycode;
					if (keyselect == 11) pausekey = keycode;
					if (keyselect == 12) linekey = keycode;
					if (keyselect == 13) musickey = keycode;
					if (keyselect == 14) noteskey = keycode;
					if (keyselect == 15) redkey = keycode;
					if (keyselect == 16) greenkey = keycode;
					if (keyselect == 17) bluekey = keycode;
					if (keyselect == 18) yellowkey = keycode;
				}
			}
		}
		if (key == KEY_UP) move -= 64;
		if (key == KEY_DOWN) move += 64;
		if (key == KEY_LEFT) mousesens += 1;
		if (key == KEY_RIGHT) mousesens -=1;

		move += mousemovey;
		while ((move >= 22*64) || (move < 0))
		{
			if (move >= 22*64) move -= 22*64;
			if (move < 0) move += 22*64;
		}

		if (mousesens > 99) mousesens = 99;
		if (mousesens < 1) mousesens = 1;

		keyselect = move / 64;

		for (; gamespeed; gamespeed--)
		{
			flash++;
		}

		fireeffect();
		for (c = 0; c < 22; c++)
		{
			if (((c == keyselect) && (flash & 16)) || (c != keyselect))
			{
				if (c < 19) txt_print(10, (10*c)+10, SPR_SMALLFONTS, keytext[c]);
				else txt_print(190, 10*(c-16), SPR_SMALLFONTS, keytext[c]);
			}
		}
		txt_print(190, 10, SPR_SMALLFONTS, "MOUSE SENSITIVITY:");
                sprintf(textbuf, "%d", -(mousesens - 100)); 
		txt_print(230, 20, SPR_SMALLFONTS, textbuf);
		txt_print(145, 170, SPR_SMALLFONTS, "USE UP & DOWN TO NAVIGATE");
		txt_print(145, 180, SPR_SMALLFONTS, "LEFT & RIGHT TO CHANGE MOUSE SENS");
		txt_print(145, 190, SPR_SMALLFONTS, "ENTER TO SELECT AND ESC TO EXIT");
		gfx_updatepage();
	}
}

int keydialog(void)
{
	kbd_getascii();
	getgamespeed();
	for (;;)
	{
          	getgamespeed();
		key = kbd_getkey();
		checkglobalkeys();
		if (key == KEY_ESC)  return 0;
		if (key != 0) return key;

                fireeffect();
 		txt_printcenter(80, SPR_FONTS, "TYPE KEY TO BIND, ESC TO CANCEL");
		gfx_updatepage();
	}
}

int buttondialog(void)
{
	kbd_getascii();
	getgamespeed();
	updatemouse(); 
	for (;;)
	{
          	getgamespeed();
		key = kbd_getkey();
                updatemouse();
		checkglobalkeys();
		if (key == KEY_ESC)  return 0;
		if (mouseb != prevmouseb) return mouseb;

                fireeffect();
 		txt_printcenter(80, SPR_FONTS, "PRESS BUTTON TO BIND, ESC TO CANCEL");
		gfx_updatepage();
	}
} 

void game(char *missionname)
{
        char mapnamebuf[80];
        char blknamebuf[80];
        char infnamebuf[80];
        char misnamebuf[80];

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
        if((!map_loadmap(mapnamebuf)) ||
           (!gfx_loadblocks(blknamebuf)) ||
           (!map_loadblockinfo(infnamebuf)) ||
           (!initgame(misnamebuf)))
        {
                gfx_fillscreen(0);
                txt_printcenter(80, SPR_FONTS, "ERROR LOADING MISSION!");
                gfx_updatepage();
	        getgamespeed();
                for (;;)
                {
		        getgamespeed();
	                updatemouse();
	                key = kbd_getkey();
  	                checkglobalkeys();
			if ((key == KEY_SPACE) || (key == KEY_ENTER) ||
				((mouseb & MOUSEB_LEFT) && (!(prevmouseb & MOUSEB_LEFT))))
			{
				playfx(FXCHAN_ENEMYSHOOT, SMP_SHOTGUN, 22050, 64, 128);
                         	break;
                        }
                }
                return;
        }

        /* No briefing in practice mode */
        if (difficulty)
        {
	        /* Briefing */
	        for (;;)
	        {
	       		char *textptr = briefingtext;
			int y = 0;

		        getgamespeed();
		        updatemouse();
		        key = kbd_getkey();
                        checkglobalkeys();
		        if (key == KEY_ESC)
		        {
				playfx(FXCHAN_ENEMYSHOOT, SMP_SHOTGUN, 22050, 64, 128);
		        	return;
		        }
			if ((key == KEY_SPACE) || (key == KEY_ENTER) ||
				((mouseb & MOUSEB_LEFT) && (!(prevmouseb & MOUSEB_LEFT))))
			{
				playfx(FXCHAN_ENEMYSHOOT, SMP_SHOTGUN, 22050, 64, 128);
	                	break;
	                }
		        fireeffect();
		        for (;;)
		        {
		        	txt_printcenter(y, SPR_FONTS, textptr);
		        	textptr += strlen(textptr)+1;
		        	if (*textptr == '$') break;
		        	y += 20;
		        }
        	        gfx_updatepage();
        	}
	}

        /* Start main music if hiscore was playing */
        playmusic(MUSIC_MAIN);

        /* Game loop */
        getgamespeed();
        updatemouse();
        for (;;)
        {
	        getgamespeed();
                updatemouse();

                if (kbd_checkkey(musickey))
                {
                      	musicvolume ^= 64;
        	       	snd_setmusicmastervolume(FIRSTFXCHAN, musicvolume);
        	}
                if (kbd_checkkey(KEY_F12))
                {
                        screenmode++;
                        if (screenmode > GFX_DOUBLESIZE) screenmode = 0;
        	        if (!gfx_init(320,200,70,screenmode | GFX_USE3PAGES))
                	{
                                if (!gfx_init(320,200,70,screenmode | GFX_USEDIBSECTION))
                                {
                        		win_messagebox("Graphics init failed!");
                                        saveconfig();
                                	exit(0);
                                }
                	}
                }
                if (kbd_checkkey(KEY_F11))
                {
                        directsound ^= 1;
                	snd_init(mixrate, mixmode, 150, CHANNELS, directsound);
                }

                if (win_quitted)
                {
                        saveconfig();
                        exit(0);
                }
		if ((kbd_checkkey(KEY_ESC)) || (gameover > 400))
                {
			snd_stopsample(FXCHAN_BND);
			snd_stopsample(FXCHAN_LIFT);
			return;
		}

		if (victory > 200)
		{
			snd_stopsample(FXCHAN_BND);
			snd_stopsample(FXCHAN_LIFT);
			endpart();
			return;
		}

		if (kbd_checkkey(noteskey))
		{
                        int c = numbombs;
                        showinstrtime = 0;
                        while (c--)
                        {
				showinstr++;
				if (showinstr >= numbombs) showinstr = 0;
				if (bomb[showinstr].instructions)
				{
					showinstrtime = 200;
					break;
				}
			}
		}

		if (kbd_checkkey(pausekey)) paused ^= 1;
		if (kbd_checkkey(linekey)) sightline ^= 1;
  	        checkglobalkeys();
                if (!paused)
                {
	                for (; gamespeed; gamespeed--)
	                {
                                win_checkmessages();
                                if (!fastforward)
                                {
	                                flashclosetlights();
	                                makecollareas();
					moveactors();
					movelifts();
					doscroll();
					countdown();
					testvictory();
				}
				else
				{
				        if (((gametime) && (bombs)) && (!timecheat))
				        {
						gametime -= 20;
						if (gametime < 0) gametime = 0;
					}
					fastforward -= 20;
					if (fastforward < 0) fastforward = 0;
				}
				if (showinstrtime) showinstrtime--;
				if (gamemsgtime) gamemsgtime--;
				if (gameover) gameover++;
			}
		}

                if (!fastforward)
                {
			/* Draw map. Shifting for explosion when time runs out */
			map_drawlayer(0, xpos/DEC+xshift, ypos/DEC+yshift, 0, 0, 21, 14);
			map_drawlayer(1, xpos/DEC+xshift, ypos/DEC+yshift, 0, 0, 21, 14);
			drawactors();
			drawshadow();

			/* Show bomb defusing instructions? */
			if (showinstrtime) drawinstr(showinstr, 256, 8);
			/* Only a living BOFH can aim. */
	                if (actor[0].type == ACTOR_BOFH) drawsight();
                        drawscanner();
			/* Show closet? */
			drawnearcloset();
			/* Show lift floor number & lift message? */
			if (liftnumber) gfx_drawsprite(160, 135, 0x90012+liftnumber);
			if (lifthere) txt_printcenter(160, SPR_SMALLFONTS, "PRESS NUMBER KEYS FOR DESTINATION FLOOR");
		}
                else
                {
                        gfx_fillscreen(0);
                }

		printstatus();
		/* Game message */
		if ((gamemsgtime) && (gamemsg)) txt_printcenter(2, SPR_FONTS, gamemsg);
		gfx_updatepage();
	}
}

void drawscanner(void)
{
        ACTOR *aptr = &actor[0];
        int c, y;

        if ((weapon != WEAP_SCANNER) || (!ammo[WEAP_SCANNER])) return;

        for (y = 0; y < 65; y++)
        {
                gfx_line(10, y+10, 74, y+10, 0);
        }
        gfx_plot(10+32, 10+32, 7);

        for (c = 0; c < actors; c++)
        {
                int xd, yd;

                switch(aptr->type)
                {
                        case ACTOR_FISTMAN:
                        case ACTOR_PISTOLMAN:
                        case ACTOR_SHOTGUNMAN:
                        case ACTOR_UZIMAN:
                        case ACTOR_TECHNICIAN:
                        case ACTOR_LEADER:
                        case ACTOR_SADIST:
                        xd = (aptr->x/DEC/8 - actor[0].x/DEC/8);
                        yd = (aptr->y/DEC/8 - actor[0].y/DEC/8);

                        if ((xd > -33) && (xd < 33) && (yd > -33) && (yd < 33))
                        {
                                gfx_plot(10+32+xd, 10+32+yd, 18);
                        }
                        break;

                	case ACTOR_ACER:
                	case ACTOR_SYSTECH:
                	case ACTOR_POMI:
                	case ACTOR_THINKPAD:
                	case ACTOR_DOCKINGSTAT:
                	case ACTOR_LASERJET:
                	case ACTOR_DESKJET:
                	case ACTOR_BIGLASERJET:
                	case ACTOR_CDSERVER:
                	case ACTOR_SUNSERVER:
                	case ACTOR_COMPAQSERVER:
                        xd = (aptr->x/DEC/8 - actor[0].x/DEC/8);
                        yd = (aptr->y/DEC/8 - actor[0].y/DEC/8);

                        if ((xd > -33) && (xd < 33) && (yd > -33) && (yd < 33))
                        {
                                gfx_plot(10+32+xd, 10+32+yd, 34);
                        }
                        break;

                        case ACTOR_CAT5:
                        case ACTOR_BND:
                        case ACTOR_PISTOL:
                        case ACTOR_SHOTGUN:
                        case ACTOR_UZI:
                        case ACTOR_SMALLMEDIKIT:
                        case ACTOR_BIGMEDIKIT:
                        case ACTOR_INSTRUCTIONS:
                        case ACTOR_GRENADE:
                        case ACTOR_CROSSBOW:
                        case ACTOR_BAZOOKA:
                        xd = (aptr->x/DEC/8 - actor[0].x/DEC/8);
                        yd = (aptr->y/DEC/8 - actor[0].y/DEC/8);

                        if ((xd > -33) && (xd < 33) && (yd > -33) && (yd < 33))
                        {
                                gfx_plot(10+32+xd, 10+32+yd, 21);
                        }
                        break;

                        case ACTOR_BAZOOKA_PROJECTILE:
                        case ACTOR_FLYINGGRENADE:
                        case ACTOR_CHUNK:
                        case ACTOR_ARROW:
                        xd = (aptr->x/DEC/8 - actor[0].x/DEC/8);
                        yd = (aptr->y/DEC/8 - actor[0].y/DEC/8);

                        if ((xd > -33) && (xd < 33) && (yd > -33) && (yd < 33))
                        {
                                gfx_plot(10+32+xd, 10+32+yd, 23);
                        }
                        break;
                }
                aptr++;
        }
}


void movelifts(void)
{
	int c, d;
	int atlift = -1, atliftfloor = -1;
        int xb = actor[0].x >> 12;
        int yb = actor[0].y >> 12;
        int req = -1;
	liftnumber = 0;
	lifthere = 0;

        for (c = 0; c < numlifts; c++)
        {
        	for (d = 0; d < lift[c].floors; d++)
        	{
        		if ((xb == lift[c].liftfloor[d].xb) &&
        		    (yb == lift[c].liftfloor[d].yb))
        		{
                                int newdestfloor;

        			atlift = c;
        			atliftfloor = d;
        			liftnumber = 1+lift[c].firstfloor + (lift[c].floor+500)/1000-1;
        			newdestfloor = (atliftfloor+1)*1000;
        			if (lift[c].floor == lift[c].destfloor)
        			{
	        			if (newdestfloor != lift[c].destfloor)
	        			{
	                                        int sx, sy;
	        				lift[c].destfloor = newdestfloor;

	        				/* Play lift running sound */
	                                        liftsound = atlift;
	                                        sx = (xb<<12)+8*DEC;
	                                        sy = (yb<<12)+8*DEC;
					        playpositionalfx(sx, sy, FXCHAN_LIFT, SMP_LIFTSTART, 11025, 48);
	        			}
	        			else lifthere = 1;
	        		}
        		}
        	}
        }

	for (c = 0; c < numlifts; c++)
	{
		if (lift[c].floor < lift[c].destfloor)
		{
			lift[c].floor += lift[c].speed;
			if (lift[c].floor >= lift[c].destfloor)
			{
                                int x, y;
				lift[c].floor = lift[c].destfloor;
                                findnearestfloor(c, &x, &y);
                                snd_stopsample(FXCHAN_LIFT);
			        playpositionalfx(x, y, FXCHAN_LIFT, SMP_LIFTSTOP, 11025, 48);
			        liftsound = -1;
			}

		}
		if (lift[c].floor > lift[c].destfloor)
		{
			lift[c].floor -= lift[c].speed;
			if (lift[c].floor <= lift[c].destfloor)
			{
                                int x, y;
				lift[c].floor = lift[c].destfloor;
                                findnearestfloor(c, &x, &y);
                                snd_stopsample(FXCHAN_LIFT);
			        playpositionalfx(x, y, FXCHAN_LIFT, SMP_LIFTSTOP, 11025, 48);
			        liftsound = -1;
			}
		}
	}

        /* Update location of lift running sound (in case player moves) */
	if (liftsound != -1)
	{
		int x, y;
                c = liftsound;
                findnearestfloor(c, &x, &y);

		updatepositionalfx(x, y, FXCHAN_LIFT);
	}

	/* Lifts don't work after player dies */
	if (gameover) return;
	if (actor[0].type != ACTOR_BOFH) return;

	if (lifthere)
	{
        	if (kbd_checkkey(KEY_0)) req = 0;
        	if (kbd_checkkey(KEY_1)) req = 1;
        	if (kbd_checkkey(KEY_2)) req = 2;
        	if (kbd_checkkey(KEY_3)) req = 3;
        	if (kbd_checkkey(KEY_4)) req = 4;
        	if (kbd_checkkey(KEY_5)) req = 5;
        	if (kbd_checkkey(KEY_6)) req = 6;
        	if (kbd_checkkey(KEY_7)) req = 7;
        	if (kbd_checkkey(KEY_8)) req = 8;
        	if (kbd_checkkey(KEY_9)) req = 9;
        	req -= lift[atlift].firstfloor;

                if ((req >= 0) && (req < lift[atlift].floors) && (req != atliftfloor))
                {
                        int dist, sx, sy;

                	/* Move player & lift to new floor */
                	actor[0].angle = lift[atlift].angle;
                	actor[0].x = (lift[atlift].liftfloor[req].xb << 12)+8*DEC;
                	actor[0].y = (lift[atlift].liftfloor[req].yb << 12)+8*DEC;
                	actor[0].x += sintable[actor[0].angle] * 10;
                	actor[0].y -= sintable[actor[0].angle+COS] * 10;
                        xpos = actor[0].x - 160*DEC;
                        ypos = actor[0].y - 100*DEC;
                        lift[atlift].destfloor = (req+1)*1000;
			dist = abs(lift[atlift].destfloor - lift[atlift].floor);
                        /*
                         * Leave lift one "pixel" off destination so the lift
                         * will make the stop sound when player arrives
                         */
                        lift[atlift].floor = lift[atlift].destfloor+1;

                        /* Play lift running sound */
                        liftsound = atlift;
                        sx = (lift[atlift].liftfloor[req].xb << 12)+8*DEC;
                        sy = (lift[atlift].liftfloor[req].yb << 12)+8*DEC;
			playpositionalfx(sx, sy, FXCHAN_LIFT, SMP_LIFTSTART, 11025, 48);


			/* Calculate time loss */
                        fastforward = dist / lift[atlift].speed + lift[atlift].startdelay;
                        /* Don't print the number & message at new destination */
                        lifthere = 0;
                        liftnumber = 0;
                }
        }
}

void findnearestfloor(int liftnum, int *x, int *y)
{
       	int mindist = 0x7fffffff, minx = 0, miny = 0, tx, ty, tdist;
        int d;

        for (d = 0; d < lift[liftnum].floors; d++)
        {
        	tx = (lift[liftnum].liftfloor[d].xb<<12) + 8*DEC;
        	ty = (lift[liftnum].liftfloor[d].yb<<12) + 8*DEC;
        	tdist = finddist(actor[0].x, actor[0].y, tx, ty);
                if (tdist < mindist)
                {
                       	mindist = tdist;
                       	minx = tx;
                       	miny = ty;
                }
        }
        *x = minx;
        *y = miny;
}



void endpart(void)
{
        #define END_KILLBONUS 0
        #define END_HEALTHBONUS 1
        #define END_TIMEBONUS 2
        #define END_SERVERBONUS 3
        #define END_WKSTBONUS 4
        #define END_FINALTEXT 5

        int c;
        int done = 0;
        int phase = 0;
        int phasetime = -35;
        int scroll = 0;

        ACTOR *aptr;

        int hbonus = actor[0].health;
        int tbonus = gametime / (70*60);
        int kbonus = kills;
        int sbonus = 0;
        int wbonus = 0;
        int soundcount = 1;

        /* Count the servers & workstations remaining */
        aptr = &actor[0];
        for (c = 0; c < actors; c++)
        {
        	if ((aptr->type >= ACTOR_FIRSTCOMPUTER) &&
        	    (aptr->type <= ACTOR_LASTCOMPUTER))
        	{
        		if (isserverroom(aptr->x, aptr->y)) sbonus++;
        		else wbonus++;
        	}
        	aptr++;
        }
        /* If goal was to destroy computers, turn the counter around */
        if (victorybits & VB_COMPUTERS)
        {
                sbonus = beginservers-sbonus;
                wbonus = beginworkstations-wbonus;
        }


        stopmusic();

	getgamespeed();
        updatemouse();
	for (;;)
	{
		getgamespeed();
                updatemouse();
                key = kbd_getkey();

  	        checkglobalkeys();
		if (done)
		{
        		if ((key == KEY_ESC) || (key == KEY_SPACE) || (key == KEY_ENTER) ||
        			((mouseb & MOUSEB_LEFT) && (!(prevmouseb & MOUSEB_LEFT))))
        		{
        			playfx(FXCHAN_ENEMYSHOOT, SMP_SHOTGUN, 22050, 64, 128);
        			return;
        		}
		}

		for (; gamespeed; gamespeed--)
		{
			switch(phase)
			{
				case END_KILLBONUS:
				phasetime++;
				if ((kbonus) && (phasetime > 2))
				{
		        	        if (soundcount) playfx(FXCHAN_KLONK, SMP_UZI, 30000, 32, 128);
                                        soundcount ^= 1;
					phasetime = 0;
					kbonus--;
					reward(50);
				}
				break;

				case END_HEALTHBONUS:
				phasetime++;
				if ((hbonus) && (phasetime > 2))
				{
		        	        if (soundcount) playfx(FXCHAN_KLONK, SMP_UZI, 30000, 32, 128);
                                        soundcount ^= 1;
					phasetime = 0;
					hbonus--;
					reward(100);
				}
				break;

				case END_TIMEBONUS:
				phasetime++;
				if ((tbonus) && (phasetime > 2))
				{
		        	        if (soundcount) playfx(FXCHAN_KLONK, SMP_UZI, 30000, 32, 128);
                                        soundcount ^= 1;
					phasetime = 0;
					tbonus--;
					reward(200);
				}
				break;

				case END_SERVERBONUS:
				phasetime++;
				if ((sbonus) && (phasetime > 10))
				{
		        	        playfx(FXCHAN_KLONK, SMP_UZI, 30000, 32, 128);
					phasetime = 0;
					sbonus--;
					reward(3000);
				}
				break;

				case END_WKSTBONUS:
				phasetime++;
				if ((wbonus) && (phasetime > 2))
				{
		        	        if (soundcount) playfx(FXCHAN_KLONK, SMP_UZI, 30000, 32, 128);
                                        soundcount ^= 1;
					phasetime = 0;
					wbonus--;
					reward(25);
					if (wbonus)
					{
						wbonus--;
						reward(25);
					}
				}
				break;

				case END_FINALTEXT:
                                if (!done)
                                {
                                	done = 1;
                                	playmusic(MUSIC_HISCORE);
                                }
                                if (scroll < 200) scroll++;
                                break;
                        }
                        if (phasetime > 35)
                        {
                                soundcount = 1;
                        	phasetime = -35;
                        	phase++;
                        }
		}

                if (phase < END_FINALTEXT)
                {
		        gfx_fillscreen(0);
	                if (phase >= END_KILLBONUS)
	                {
	                	sprintf(textbuf, "KILL BONUS %d X 50", kbonus);
	                	txt_printcenter(20, SPR_FONTS, textbuf);
	                }
	                if (phase >= END_HEALTHBONUS)
	                {
	                	sprintf(textbuf, "HEALTH BONUS %d X 100", hbonus);
	                	txt_printcenter(50, SPR_FONTS, textbuf);
	                }

	                if (phase >= END_TIMEBONUS)
	                {
	                	sprintf(textbuf, "TIME BONUS %d X 200", tbonus);
	                	txt_printcenter(80, SPR_FONTS, textbuf);
	                }

	                if (phase >= END_SERVERBONUS)
	                {
	                	sprintf(textbuf, "SERVER BONUS %d X 3000", sbonus);
	                	txt_printcenter(110, SPR_FONTS, textbuf);
	                }

	                if (phase >= END_WKSTBONUS)
	                {
	                	sprintf(textbuf, "WORKSTATION&PRINTER BONUS %d X 25", wbonus);
	                	txt_printcenter(140, SPR_FONTS, textbuf);
	                }
	                printstatus();
		}
		else
		{
                        char *textptr = victorytext;
                        int y = 200;
			fireeffect();
			for (;;)
			{
				txt_printcenter(y-scroll, SPR_FONTS, textptr);
				textptr += strlen(textptr)+1;
				if (*textptr == '$') break;
				y += 20;
			}
                }
	        gfx_updatepage();
	}
}


void flashclosetlights(void)
{
        int c;

	if (rand() % 5) return;
	for (c = 0; c < numclosets; c++)
	{
                if (closet[c].anim)
                {
        		if (map_layerdataptr[1][closet[c].yb * map_layer[1].xsize+closet[c].xb])
			map_layerdataptr[1][closet[c].yb * map_layer[1].xsize+closet[c].xb] ^= 1;
                }
	}
}

void reward(int points)
{
        if (!cheating())
                score += points;
}


int checkhiscore(void)
{
        int place = -1;
        int ascii;
        int key;
        int c;

        loadhiscore();

        for (c = 0; c < 10; c++)
        {
        	if (score > hiscore[c].score)
        	{
        		place = c;
        		break;
        	}
        }
        if (place == -1) return 0; /* No hiscore */

	/* Shift hiscores down */
	for (c = 9; c > place; c--)
	{
		hiscore[c].score = hiscore[c-1].score;
		memcpy(hiscore[c].name, hiscore[c-1].name, NAMELENGTH);
	}
        hiscore[place].score = score;
        memset(hiscore[place].name, 0, NAMELENGTH);

        playmusic(MUSIC_HISCORE);

	getgamespeed();
        updatemouse();
        kbd_getascii();
	for (;;)
	{
		getgamespeed();
                updatemouse();

                key = kbd_getkey();
                ascii = kbd_getascii();
                ascii = toupper(ascii);
                checkglobalkeys();

                if (ascii)
                {
			if ((ascii == 27) || (ascii == 13))
			{
				playfx(FXCHAN_ENEMYSHOOT, SMP_SHOTGUN, 22050, 64, 128);
				break;
			}
	                if (ascii == 8)
	                {
	                	int pos = strlen(hiscore[place].name);
	                        if (pos)
	                        {
	                        	hiscore[place].name[pos-1] = 0;
					playfx(FXCHAN_ENEMYSHOOT, SMP_FIST1, 22050, 64, 128);
	                        }
	                }
                        if ((ascii >= 32) && (ascii < 96))
	                {
	                	int pos = strlen(hiscore[place].name);
	                	if (pos < NAMELENGTH-1)
	                	{
	                       		hiscore[place].name[pos] = ascii;
					playfx(FXCHAN_ENEMYSHOOT, SMP_FIST1, 22050, 64, 128);
	                       	}
	                }
	        }

	        fireeffect();
	        txt_printcenter(50, SPR_FONTS, "YOU ARE ONE OF THE LEGENDARY BOFHS");
	        txt_printcenter(70, SPR_FONTS, "ENTER YOUR NAME USING KEYBOARD");
                sprintf(textbuf, "%02d. %-19s %06d", c+1, hiscore[place].name, hiscore[place].score);
		txt_printcenter(110, SPR_FONTS, textbuf);
	        gfx_updatepage();
	}

        savehiscore();
        return 1;
}

void drawsight(void)
{
        int xp, yp, dx, dy, sx, sy;

        if (controlscheme == CTRL_ABSOLUTE)
        {
                xp = mouseposx * DEC;
                yp = mouseposy * DEC;
        }
        else
        {
                xp = (actor[0].x & 0xffffff00) + 60 * sintable[actor[0].angle];
                yp = (actor[0].y & 0xffffff00) - 60 * sintable[actor[0].angle+COS];
                xp -= xshift;
                xp -= yshift;
                xp -= (xpos & 0xffffff00);
                yp -= (ypos & 0xffffff00);
        }

        /* Ugly code to draw the dotted sight line. :-) */
        if (sightline)
        {

		sx = actor[0].x - (xpos & 0xffffff00);
		sy = actor[0].y - (ypos & 0xffffff00);
		dx = (xp - sx)/16;
		dy = (yp - sy)/16;


		gfx_plot((sx + dx * 2)/DEC, (sy + dy * 2)/DEC, 39);
		gfx_plot((sx + dx * 3)/DEC, (sy + dy * 3)/DEC, 39);
		gfx_plot((sx + dx * 4)/DEC, (sy + dy * 4)/DEC, 39);
		gfx_plot((sx + dx * 5)/DEC, (sy + dy * 5)/DEC, 39);
		gfx_plot((sx + dx * 6)/DEC, (sy + dy * 6)/DEC, 39);
		gfx_plot((sx + dx * 7)/DEC, (sy + dy * 7)/DEC, 39);
		gfx_plot((sx + dx * 8)/DEC, (sy + dy * 8)/DEC, 39);
		gfx_plot((sx + dx * 9)/DEC, (sy + dy * 9)/DEC, 39);
		gfx_plot((sx + dx * 10)/DEC, (sy + dy * 10)/DEC, 39);
		gfx_plot((sx + dx * 11)/DEC, (sy + dy * 11)/DEC, 39);
		gfx_plot((sx + dx * 12)/DEC, (sy + dy * 12)/DEC, 39);
		gfx_plot((sx + dx * 13)/DEC, (sy + dy * 13)/DEC, 39);
		gfx_plot((sx + dx * 14)/DEC, (sy + dy * 14)/DEC, 39);
		gfx_plot((sx + dx * 15)/DEC, (sy + dy * 15)/DEC, 39);
		gfx_plot((sx + dx * 16)/DEC, (sy + dy * 16)/DEC, 39);
	}

        xp /= DEC;
        yp /= DEC;

        gfx_drawsprite(xp, yp, SPRI_SIGHT);

}

void printstatus(void)
{
	int seconds, minutes, c;

	sprintf(textbuf, "%06d", score);
        txt_print(0, 182, SPR_FONTS, textbuf);
        sprintf(textbuf, "%03d", ammo[weapon]);
	txt_print(294, 182, SPR_FONTS, textbuf);
	if (gametime)
	{
		seconds = (gametime/70) % 60;
		minutes = (gametime/70) / 60;
		if ((gametime % 70) >= 35)
		{
			sprintf(textbuf, "%02d %02d", minutes, seconds);
		}
		else
		{
			sprintf(textbuf, "%02d:%02d", minutes, seconds);
                }
	        for (c = 181; c < 199; c++) gfx_line(139,c,139+45,c, 2);
		txt_print(140, 182, SPR_FONTS, textbuf);
	}

	sprintf(textbuf, "%03d", actor[0].health);
	gfx_drawsprite(61, 197, SPRI_ST_HEART);
	txt_print(69, 182, SPR_FONTS, textbuf);

	sprintf(textbuf, "%03d", terrorists);
	gfx_drawsprite(102, 197, SPRI_ST_TERROR);
	txt_print(110, 182, SPR_FONTS, textbuf);

	sprintf(textbuf, "%01d", bombs);
	gfx_drawsprite(192, 197, SPRI_ST_BOMB);
	txt_print(200, 182, SPR_FONTS, textbuf);

	sprintf(textbuf, "%03d", computers);
	gfx_drawsprite(215, 197, SPRI_ST_COMPU);
	txt_print(223, 182, SPR_FONTS, textbuf);

        gfx_drawsprite(271, 197, SPRI_ST_WPN(weapon));

        if (paused) txt_printcenter(90, SPR_FONTS, "PAUSED");
}

void doscroll(void)
{
        int xp,yp;
	int speedx = 0;
	int speedy = 0;
       	xp = actor[0].x & 0xffffff00;
       	yp = actor[0].y & 0xffffff00;
       	xp -= (xpos & 0xffffff00);
       	yp -= (ypos & 0xffffff00);
       	if (xp > 180*DEC) speedx = (xp-180*DEC) / 5;
       	if (xp < 140*DEC) speedx = -(140*DEC-xp) / 5;
       	if (yp > 110*DEC) speedy = (yp-110*DEC) / 5;
       	if (yp < 90*DEC) speedy = -(90*DEC-yp) / 5;
       	xpos += speedx;
       	ypos += speedy;
        if (xpos < 0) xpos = 0;
        if (ypos < 0) ypos = 0;
        if (xpos > (map_layer[0].xsize*16-320)*DEC) xpos = (map_layer[0].xsize*16-320)*DEC;
        if (ypos > (map_layer[0].ysize*16-200)*DEC) ypos = (map_layer[0].ysize*16-200)*DEC;
}

void drawshadow(void)
{
        /* Shadow areas of the screen the player can't see */

	int sx, sy, ex, ey, x, y;
	sx = xpos - 16*DEC;
	ex = sx + 22*16*DEC;
	sy = ypos - 16*DEC;
	ey = sy + 15*16*DEC;

	for (y = sy; y < ey; y += 16*DEC)
	{
		for (x = sx; x < ex; x += 16*DEC)
		{
			if (!checkblockvision(x,y))
			{
	                       	int xp = x & 0xfffff000;
	                       	int yp = y & 0xfffff000;
	                       	xp -= (xpos & 0xffffff00);
	                       	yp -= (ypos & 0xffffff00);
	                       	xp /= DEC;
	                       	yp /= DEC;
	                       	gfx_drawsprite(xp, yp, SPRI_SHADOWBLOCK);
			}
		}
	}
}

int squareroot(int x)
{
	return sqrt(x);
}

void opendoor(int xp, int yp)
{
        int blkinf, nxp, nyp;
        unsigned short *mapptr;

	xp /= DEC;
	yp /= DEC;

	blkinf = map_getblockinfo(1, xp, yp);
	if (!(blkinf & INF_DOOR)) return;
        playpositionalfx(xp*DEC, yp*DEC, FXCHAN_DOOROPEN, SMP_SWISH, 30000, 48);
        alertenemies(xp*DEC,yp*DEC, SNDDIST_DOOR);
	mapptr = &map_layerdataptr[1][map_layer[1].xsize*(yp/16)+xp/16];
	*mapptr += 2;

	/* Ultimate kluge for WC stalls */
	if (map_getblocknum(0, xp, yp) == 141) return;

        /* Check left from here */
        nxp = xp;
        nyp = yp;
        for (;;)
        {
                nxp -= 16;
		blkinf = map_getblockinfo(1, nxp, nyp);
		if (!(blkinf & INF_DOOR)) break;
		mapptr = &map_layerdataptr[1][map_layer[1].xsize*(nyp/16)+nxp/16];
		*mapptr += 2;
	}

        /* Check right from here */
        nxp = xp;
        nyp = yp;
        for (;;)
        {
                nxp += 16;
		blkinf = map_getblockinfo(1, nxp, nyp);
		if (!(blkinf & INF_DOOR)) break;
		mapptr = &map_layerdataptr[1][map_layer[1].xsize*(nyp/16)+nxp/16];
		*mapptr += 2;
	}

        /* Check up from here */
        nxp = xp;
        nyp = yp;
        for (;;)
        {
                nyp -= 16;
		blkinf = map_getblockinfo(1, nxp, nyp);
		if (!(blkinf & INF_DOOR)) break;
		mapptr = &map_layerdataptr[1][map_layer[1].xsize*(nyp/16)+nxp/16];
		*mapptr += 2;
	}

        /* Check down from here */
        nxp = xp;
        nyp = yp;
        for (;;)
        {
                nyp += 16;
		blkinf = map_getblockinfo(1, nxp, nyp);
		if (!(blkinf & INF_DOOR)) break;
		mapptr = &map_layerdataptr[1][map_layer[1].xsize*(nyp/16)+nxp/16];
		*mapptr += 2;
	}
}

void closedoor(int xp, int yp)
{
        int blkinf, nxp, nyp;
        unsigned short *mapptr;

	xp /= DEC;
	yp /= DEC;

	blkinf = map_getblockinfo(1, xp, yp);
	if (!(blkinf & INF_OPENDOOR)) return;
        if (checkblockactors(xp/16, yp/16)) return;
        playpositionalfx(xp*DEC, yp*DEC, FXCHAN_DOORCLOSE, SMP_SWISH, 30000, 48);
        alertenemies(xp*DEC,yp*DEC, SNDDIST_DOOR);
	mapptr = &map_layerdataptr[1][map_layer[1].xsize*(yp/16)+xp/16];
	*mapptr -= 2;

	/* Ultimate kluge for WC stalls */
	if (map_getblocknum(0, xp, yp) == 141) return;

        /* Check left from here */
        nxp = xp;
        nyp = yp;
        for (;;)
        {
                nxp -= 16;
		blkinf = map_getblockinfo(1, nxp, nyp);
		if (!(blkinf & INF_OPENDOOR)) break;
                if (checkblockactors(nxp/16, nyp/16)) break;
		mapptr = &map_layerdataptr[1][map_layer[1].xsize*(nyp/16)+nxp/16];
		*mapptr -= 2;
	}

        /* Check right from here */
        nxp = xp;
        nyp = yp;
        for (;;)
        {
                nxp += 16;
		blkinf = map_getblockinfo(1, nxp, nyp);
		if (!(blkinf & INF_OPENDOOR)) break;
                if (checkblockactors(nxp/16, nyp/16)) break;
		mapptr = &map_layerdataptr[1][map_layer[1].xsize*(nyp/16)+nxp/16];
		*mapptr -= 2;
	}

        /* Check up from here */
        nxp = xp;
        nyp = yp;
        for (;;)
        {
                nyp -= 16;
		blkinf = map_getblockinfo(1, nxp, nyp);
		if (!(blkinf & INF_OPENDOOR)) break;
                if (checkblockactors(nxp/16, nyp/16)) break;
		mapptr = &map_layerdataptr[1][map_layer[1].xsize*(nyp/16)+nxp/16];
		*mapptr -= 2;
	}

        /* Check down from here */
        nxp = xp;
        nyp = yp;
        for (;;)
        {
                nyp += 16;
		blkinf = map_getblockinfo(1, nxp, nyp);
		if (!(blkinf & INF_OPENDOOR)) break;
                if (checkblockactors(nxp/16, nyp/16)) break;
		mapptr = &map_layerdataptr[1][map_layer[1].xsize*(nyp/16)+nxp/16];
		*mapptr -= 2;
	}
}

void fireeffect(void)
{
        int c;


       	for (c = 32000; c < 32641; c++)
       	{
       		firebuf[c] = (rand() & 127)+64;
       	}

       	fireinnerloop();
       	blitfire();
}

void fireinnerloop(void)
{
	int c;
	int eax;
	unsigned char *fireptr = firebuf;
	for (c = 32000; c; c--)
	{
		eax = fireptr[640] + fireptr[641] + fireptr[320] + fireptr[321];
		eax >>= 2;
		eax--;
		if (eax < 64) eax = 64;
		*fireptr++ = eax;
	}
}

void blitfire(void)
{
	unsigned char *fireptr = firebuf;
	unsigned char *destptr = gfx_vscreen;
        int c;

	for (c = 0; c < 100; c++)
	{
		memcpy(destptr, fireptr, 320);
                destptr += gfx_virtualxsize;
		memcpy(destptr, fireptr, 320);
                destptr += gfx_virtualxsize;
                fireptr += 320;
        }
}

int initstuff(void)
{
	int c;

        win_openwindow("BOFH - Servers Under Siege", NULL);

        /* Set random seed */
        srand(666);

        kbd_init();

        win_setmousemode(MOUSE_ALWAYS_HIDDEN);

	if (!gfx_init(320,200,70,screenmode | GFX_USE3PAGES))
	{
                if (!gfx_init(320,200,70,screenmode | GFX_USEDIBSECTION))
                {
                	win_messagebox("Graphics init failed!");
                        return 0;
                }
	}

	for (c = 0;; c++)
	{
                char spritenamebuf[80];
		if (!spritename[c]) break;
                strcpy(spritenamebuf, DIR_DATA "/");
                strcat(spritenamebuf, spritename[c]);

		if (!gfx_loadsprites(c, spritenamebuf))
		{
			win_messagebox("Sprite load error");
			return 0;
		}
	}

	for(c = 0; c < MAX_SMP; c++)
	{
                char samplenamebuf[80];

		if (!samplename[c]) break;

                strcpy(samplenamebuf, DIR_DATA "/");
                strcat(samplenamebuf, samplename[c]);

		smp[c] = snd_loadrawsample(samplenamebuf, 0, 0, VM_ONESHOT);
		if (!smp[c])
		{
			printf("Sample load error (%s)\n", samplename[c]);
			return 0;
		}
	}

	for (c = 0; c < sizeof(repeats)/sizeof(SAMPLELOOPPOINT); c++)
        {
	        SAMPLE *smpp = smp[repeats[c].samplenum];
	        smpp->repeat = smpp->start
	                + (repeats[c].repeat
	                   << ((smpp->voicemode & VM_16BIT) != 0));
	        if (smpp->repeat >= smpp->end) smpp->repeat = smpp->start;
	        smpp->voicemode = ((smpp->voicemode & ~VM_ONESHOT)
	                           | VM_LOOP);
        }

        firebuf = malloc(32641);
        if (!firebuf)
	{
		win_messagebox("Insufficient memory!");
		return 0;
	}

	snd_init(mixrate, mixmode, 150, CHANNELS, directsound);
	snd_setmusicmastervolume(FIRSTFXCHAN, musicvolume);
	snd_setsfxmastervolume(FIRSTFXCHAN, sfxvolume);

        return 1;
}

void playfx(int chan, int fx, unsigned freq, unsigned char volume, unsigned char panning)
{
	snd_playsample(smp[fx], chan, freq, volume, panning);
}

void playpositionalfx(int x, int y, int chan, int fx, unsigned freq, int volume)
{
        int panning, voladjust, angle;
        channelextras[chan].vol = volume;
 	voladjust = finddist(actor[0].x, actor[0].y, x, y);
 	if (voladjust > 0)
 	{
 		voladjust /= 10;
 		if (voladjust > 56) voladjust = 56;
	 	angle = findangle(actor[0].x, actor[0].y, x, y);
	 	angle = angledist(actor[0].angle, angle);
	 	angle &= 0x3ff,
	 	panning = 128 + ((sintable[angle]*7)/16);
                volume -= voladjust;
	}
	else panning = 128;
        if (volume <= 0) return;
        snd_playsample(smp[fx], chan, freq, volume, panning);
        channelextras[chan].owner = 0;
}

void updatepositionalfx(int x, int y, int chan)
{
        int panning, voladjust, angle;
        int volume = channelextras[chan].vol;
        if (!snd_sndinitted) return;
 	voladjust = finddist(actor[0].x, actor[0].y, x, y);
 	if (voladjust > 0)
 	{
 		voladjust /= 10;
 		if (voladjust > 56) voladjust = 56;
	 	angle = findangle(actor[0].x, actor[0].y, x, y);
	 	angle = angledist(actor[0].angle, angle);
	 	angle &= 0x3ff,
	 	panning = 128 + ((sintable[angle]*7)/16);
                volume -= voladjust;
	}
	else panning = 128;
        if (volume < 0) volume = 0;
        snd_channel[chan].vol = volume;
        snd_channel[chan].panning = panning;
        channelextras[chan].owner = 0;
}

// ***
void killownedpositionalfx(ACTOR *aptr, int chan)
{
  if (aptr && (channelextras[chan].owner == aptr)) {
    snd_stopsample(chan);
  }
}

// ***
void updateownedpositionalfx(ACTOR *aptr, int x, int y, int chan)
{
  if (!snd_sndinitted) return;
  if (aptr && (channelextras[chan].owner == aptr)) {
    if (!(snd_channel[chan].voicemode & VM_ON)) {
      channelextras[chan].owner = 0;
    } else
    {
      int panning, voladjust, angle, volume = channelextras[chan].vol;
      voladjust = finddist(actor[0].x, actor[0].y, x, y);
      if (voladjust > 0)
      {
              voladjust /= 10;
              if (voladjust > 56) voladjust = 56;
              angle = findangle(actor[0].x, actor[0].y, x, y);
              angle = angledist(actor[0].angle, angle);
              angle &= 0x3ff,
              panning = 128 + ((sintable[angle]*7)/16);
              volume -= voladjust;
      }
      else panning = 128;
      if (volume <= 0) volume = 0;
      snd_channel[chan].vol = (volume+((int)snd_channel[chan].vol)*3)/4;
      snd_channel[chan].panning = (panning+((int)snd_channel[chan].panning)*3)/4;
    }
  }
}

void playownedpositionalfx(ACTOR *aptr, int x, int y, int chan, int fx, unsigned freq, int volume)
{
        int panning, voladjust, angle, newvolume = 0;
 	voladjust = finddist(actor[0].x, actor[0].y, x, y);
 	if (voladjust > 0)
 	{
 		voladjust /= 10;
 		if (voladjust > 56) voladjust = 56;
	 	angle = findangle(actor[0].x, actor[0].y, x, y);
	 	angle = angledist(actor[0].angle, angle);
	 	angle &= 0x3ff,
	 	panning = 128 + ((sintable[angle]*7)/16);
                newvolume = volume - voladjust;
	}
	else panning = 128;
        if (newvolume <= 0) return;
        snd_playsample(smp[fx], chan, freq, newvolume, panning);
        channelextras[chan].owner = aptr;
        channelextras[chan].vol = volume;
}


void testvictory(void)
{
        int bits = 0;

        if (!bombs) bits |= VB_BOMBS;
        if (!leaders) bits |= VB_LEADERS;
        if (!terrorists) bits |= VB_TERRORISTS;
        if (!computers) bits |= VB_COMPUTERS;

	if ((difficulty) && (!gameover))
	{
                if ((bits & victorybits) == victorybits) victory++;
	}
}

void printgamemsg(char *msg, int time)
{
	gamemsg = msg;
	gamemsgtime = time;
}

int isserverroom(int x, int y)
{
	int xb = x >> 12;
	int yb = y >> 12;

	if ((xb >= srminx) && (xb <= srmaxx) &&
	    (yb >= srminy) && (yb <= srmaxy)) return 1;
	else return 0;
}


void checkcheats(void)
{
        char ascii = kbd_getascii();

        ascii = toupper(ascii);

        if (!ascii) return;
        if (trycheatstring == -1)
        {
        	int c;
        	/* Which cheatstring will be entered */
        	for (c = 0; c < NUMCHEATS; c++)
        	{
        		if (ascii == cheatstring[c][0]-1)
        		{
                                trycheatstring = c;
                                trycheatindex = 1;
                                break;
                        }
                }
        }
        else
        {
        	if (ascii == cheatstring[trycheatstring][trycheatindex]-1)
        	{
                        /* Correct letter */
        		trycheatindex++;
        		if (trycheatindex == strlen(cheatstring[trycheatstring]))
        		{
				playfx(24, SMP_SHOTGUN, 22000, 32, 64);
				playfx(25, SMP_SHOTGUN, 22050, 32, 128);
				playfx(26, SMP_SHOTGUN, 22100, 32, 192);
        			*cheatvalue[trycheatstring] ^= 1;
        			trycheatstring = -1;
        		}
        	}
        	else
        	{
        		int c;
        		trycheatstring = -1; /* Wrong letter */
	        	/* Beginning of a new cheatstring? */
	        	for (c = 0; c < NUMCHEATS; c++)
	        	{
	        		if (ascii == cheatstring[c][0]-1)
	        		{
	                                trycheatstring = c;
	                                trycheatindex = 1;
	                                break;
        	                }
	                }
	        }
        }
}

int cheating(void)
{
        int c;
        for (c = 0; c < NUMCHEATS; ++c)
        {
                if (*cheatvalue[c])
                {
                        return 1;
                }
        }
        return 0;
}

void updatemouse(void)
{
	prevmouseb = mouseb;
	prevmovex = mousemovex;
	prevmovey = mousemovey;

        mouseb = mou_getbuttons();
        mou_getmove(&mousemovex, &mousemovey);
        mou_getpos(&mouseposx, &mouseposy);

        avgmovex = (mousemovex + prevmovex) / 2;
        avgmovey = (mousemovey + prevmovey) / 2;
}
