#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <signal.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "bme.h"
#include "samples.h"
#include "endian.h"

/* The makefile can override these if desired.  */
#ifndef DIR_DATA
# define DIR_DATA "data"
# define DIR_MISSIONS "missions"
# define DIR_USERCFG "."
# define DIR_SCORE "."
#endif

#define MUSIC_MAIN 0
#define MUSIC_HISCORE 1

#define NAMELENGTH 20
#define MAXBRIEFINGLENGTH 4096

#define MAXMAPSIZE 520

#define MAXMISSIONS 512

#define VB_BOMBS 1
#define VB_LEADERS 2
#define VB_TERRORISTS 4
#define VB_COMPUTERS 8

#define MAX_ACTOR 2048
#define MAX_SMP 64
#define MAX_BOMB 64
#define MAX_CLOSET 64
#define MAX_LIFT 64
#define MAX_STAIRS 256
#define MAX_DIFF 5
#define MAX_EQUIP 9
#define MAX_FLOOR 10
#define MAXFRAMESKIP 5

#define WIRE_NONE -1
#define WIRE_RED 0
#define WIRE_GREEN 1
#define WIRE_BLUE 2
#define WIRE_YELLOW 3

#define CB_LEFT 1
#define CB_RIGHT 2
#define CB_FORWARD 4
#define CB_BACKWARD 8
#define CB_STRAFELEFT 16
#define CB_STRAFERIGHT 32

#define MODE_PATROL 1
#define MODE_ATTACK 2

#define INF_OBSTACLE 1
#define INF_WALL 2
#define INF_DOOR 4
#define INF_OPENDOOR 8
#define INF_STAIRS 16
#define INF_GLASS 32
#define INF_BLOCKVISION 128

/* Autoselection assumes that bigger numbers mean better weapons.  */
#define WEAP_FISTS   0
#define WEAP_CAT5    1
#define WEAP_BND     2
#define WEAP_CROSSBOW 3
#define WEAP_PISTOL  4
#define WEAP_SHOTGUN 5
#define WEAP_UZI     6
#define WEAP_GRENADE 7
#define WEAP_BAZOOKA 8
#define WEAP_SCANNER 9
#define WEAPNUM      10

#define FIRSTFXCHAN 14
#define FXCHAN_PLRSHOOT 14
#define FXCHAN_ENEMYSHOOT 15
#define FXCHAN_FIST 16
#define FXCHAN_HIT 17
#define FXCHAN_DEATH 18
#define FXCHAN_EXPLODE 19
#define FXCHAN_KLONK 20
#define FXCHAN_DOOROPEN 21
#define FXCHAN_DOORCLOSE 22
#define FXCHAN_SPEECH 23
#define FXCHAN_BND 24
#define FXCHAN_GUNLOAD 25
#define FXCHAN_MULTI1 26
#define FXCHAN_MULTI2 27
#define FXCHAN_RICOCHET 28
#define FXCHAN_GLASS1 29
#define FXCHAN_GLASS2 30
#define FXCHAN_LIFT 31

#define DEC 256
#define DSHIFT 8
#define COS 256

#define SPR_FONTS 0
#define SPR_PLAYER 1
#define SPR_WEAPON 2
#define SPR_MACHINE 3
#define SPR_FISTMAN 4
#define SPR_GUNMAN 5
#define SPR_SHOTGMAN 6
#define SPR_UZIMAN 7
#define SPR_TECH 8
#define SPR_CLOSET 9
#define SPR_LEADER 10
#define SPR_SADIST 11
#define SPR_TITLE 12
#define SPR_SMALLFONTS 13
#define SPR_BLACKFONTS 14

#define SPRI_DEADBOFH1 ((SPR_PLAYER << 16) + 0x59)
#define SPRI_DEADBOFH2 ((SPR_PLAYER << 16) + 0x5A)
#define SPRI_SIGHT     ((SPR_PLAYER << 16) + 0x5B)
#define SPRI_ST_WPN(x) ((SPR_PLAYER << 16) + 0x5C + x)
#define SPRI_ST_HEART  ((SPR_PLAYER << 16) + 0x66)
#define SPRI_ST_BOMB   ((SPR_PLAYER << 16) + 0x67)
#define SPRI_ST_TERROR ((SPR_PLAYER << 16) + 0x68)
#define SPRI_ST_COMPU  ((SPR_PLAYER << 16) + 0x69)
#define SPRI_CAT5      ((SPR_PLAYER << 16) + 0x6A)
#define SPRI_BND       ((SPR_PLAYER << 16) + 0x6B)
#define SPRI_PISTOL    ((SPR_PLAYER << 16) + 0x6C)
#define SPRI_SHOTGUN   ((SPR_PLAYER << 16) + 0x6D)
#define SPRI_UZI       ((SPR_PLAYER << 16) + 0x6E)
#define SPRI_SMALLMEDI ((SPR_PLAYER << 16) + 0x6F)
#define SPRI_BIGMEDI   ((SPR_PLAYER << 16) + 0x70)
#define SPRI_INSTRUCT  ((SPR_PLAYER << 16) + 0x71)
// ***
#define SPRI_BAZOOKA   ((SPR_PLAYER << 16) + 0x72)
#define SPRI_CROSSBOW  ((SPR_PLAYER << 16) + 0x73)
#define SPRI_SCANNER   ((SPR_PLAYER << 16) + 0x74)
#define SPRI_SHELL      ((SPR_WEAPON << 16) + 36)
#define SPRI_SHOTGSHELL ((SPR_WEAPON << 16) + 40)
#define SPRI_RICOCHET   ((SPR_WEAPON << 16) + 52)
#define SPRI_FLAME      ((SPR_WEAPON << 16) + 60)
#define SPRI_SHARD      ((SPR_WEAPON << 16) + 68)
#define SPRI_GRENADE    ((SPR_WEAPON << 16) + 76)
// ***
#define SPRI_BAZOOKA_PROJECTILE ((SPR_WEAPON << 16) + 81)
#define SPRI_BAZOOKA_STRAP      ((SPR_WEAPON << 16) + 89)
#define SPRI_BAZOOKA_USED       ((SPR_WEAPON << 16) + 93)

#define SPRI_SHADOWBLOCK        ((SPR_WEAPON << 16) + 97)

#define TITLE_PRESENTS 0
#define TITLE_LOGO 1
#define TITLE_HISCORE 2
#define TITLE_CREDITS 3
#define TITLE_CONTROLS 4
#define TITLE_RESOLUTION 5

#define DIFF_PRACTICE 0
#define DIFF_EASY 1
#define DIFF_MEDIUM 2
#define DIFF_HARD 3
#define DIFF_INSANE 4

#define MOUSEB_LEFT 1
#define MOUSEB_RIGHT 2

#define DMG_CHUNK   1
#define DMG_FIST    2
#define DMG_FLAME   2
#define DMG_SHARD   2
#define DMG_BND     2           /* but it's fast */
#define DMG_WHIP    3
#define DMG_BULLET  5
#define DMG_BOMBCHUNK 5
#define DMG_ARROW   10

/*
 * Time in which the enemy gives up the hunt, if hasn't
 * seen the player at all
 */
#define GIVEUPTIME 60*70

/* Maximum distances for attacks */
#define SADIST_MAXDIST 160
#define LEADER_MAXDIST 140
#define UZIMAN_MAXDIST 100
#define SHOTGUN_MAXDIST 120
#define PISTOL_MAXDIST 80
#define GRENADE_MINDIST 70

/* How far different sounds will alert enemies */
#define SNDDIST_EXPLOSION 600
#define SNDDIST_GUN 300
#define SNDDIST_DRILL 130
#define SNDDIST_GLASS 100
#define SNDDIST_SPEECH 75
#define SNDDIST_DAMAGEGRUNT 50
#define SNDDIST_DEATHGRUNT 30
#define SNDDIST_DOOR 10

/* How far away enemies movement is processed */
#define VERYFAR 700

#define ACTOR_NONE 0
#define ACTOR_FIRSTACTOR ACTOR_BOFH
#define ACTOR_BOFH 1
#define ACTOR_MUZZLE 2
#define ACTOR_BULLET 3
#define ACTOR_SMOKE 4
#define ACTOR_FIRSTCOMPUTER ACTOR_ACER
#define ACTOR_ACER 5
#define ACTOR_POMI 6
#define ACTOR_SYSTECH 7
#define ACTOR_THINKPAD 8
#define ACTOR_DOCKINGSTAT 9
#define ACTOR_LASERJET 10
#define ACTOR_DESKJET 11
#define ACTOR_BIGLASERJET 12
#define ACTOR_CDSERVER 13
#define ACTOR_SUNSERVER 14
#define ACTOR_COMPAQSERVER 15
#define ACTOR_LASTCOMPUTER ACTOR_COMPAQSERVER
#define ACTOR_EXPLOSION 16
#define ACTOR_FISTHIT 17
#define ACTOR_WHIPHIT 18
#define ACTOR_CHUNK 19
#define ACTOR_SPARK 20
#define ACTOR_BLOOD 21
#define ACTOR_FIRSTENEMY ACTOR_FISTMAN
#define ACTOR_FISTMAN 22
#define ACTOR_PISTOLMAN 23
#define ACTOR_SHOTGUNMAN 24
#define ACTOR_UZIMAN 25
#define ACTOR_TECHNICIAN 26
#define ACTOR_SADIST 27
#define ACTOR_LEADER 28
#define ACTOR_LASTENEMY ACTOR_LEADER
#define ACTOR_FIRSTDEADENEMY ACTOR_DEADFISTMAN
#define ACTOR_DEADFISTMAN 29
#define ACTOR_DEADBOFH 30
#define ACTOR_DEADPISTOLMAN 31
#define ACTOR_DEADSHOTGUNMAN 32
#define ACTOR_DEADUZIMAN 33
#define ACTOR_DEADTECHNICIAN 34
#define ACTOR_DEADLEADER 35
#define ACTOR_DEADSADIST 36
#define ACTOR_LASTDEADENEMY ACTOR_DEADSADIST
#define ACTOR_CAT5 37
#define ACTOR_PISTOL 38
#define ACTOR_SHOTGUN 39
#define ACTOR_UZI 40
#define ACTOR_SMALLMEDIKIT 41
#define ACTOR_BIGMEDIKIT 42
#define ACTOR_INSTRUCTIONS 43
#define ACTOR_BND 44
#define ACTOR_BNDHIT 45
// ***
#define ACTOR_BAZOOKA 46
#define ACTOR_KEYBOARD 47
#define ACTOR_MULTITRACK 48
#define ACTOR_VCR_AND_TV 49
#define ACTOR_SHELL 50
#define ACTOR_SHOTGSHELL 51
#define ACTOR_RICOCHET 52
#define ACTOR_BEAM 53
#define ACTOR_FLAME 54
#define ACTOR_SHARD 55
#define ACTOR_FLYINGGRENADE 56
#define ACTOR_GRENADE 57
// ***
#define ACTOR_BAZOOKA_PROJECTILE 58
#define ACTOR_BAZOOKA_STRAP 59
#define ACTOR_BAZOOKA_USED 60
#define ACTOR_CROSSBOW 61
#define ACTOR_ARROW 62
#define ACTOR_SCANNER 63
#define ACTOR_LASTACTOR ACTOR_SCANNER

#define D_U 0
#define D_UR 128
#define D_R 256
#define D_DR 384
#define D_D 512
#define D_DL 640
#define D_L 768
#define D_UL 896

#define NUM_SHARDS 10
#define NUM_GRENADEPIECES 35

#define EQUIP_NONE 0
#define EQUIP_RIMA 1
#define EQUIP_HP_HUB 2
#define EQUIP_AT_T_HUB 3
#define EQUIP_CISCO_SWITCH 4
#define EQUIP_RJ45RIMA 5

typedef struct
{
	int samplenum;
	int repeat;
} SAMPLELOOPPOINT;


typedef struct
{
	char name[NAMELENGTH];
	int score;
} HISCORE_ENTRY;

typedef struct
{
	int xb;
	int yb;
        int anim;
        char name[16];
	int equipment[MAX_EQUIP];
} CLOSET;

typedef struct
{
	int location;
	int wireorder[4];
	int wiresleft;
	int instructions;
} BOMB;

typedef struct
{
	int x;
	int y;
	int angle;
	int angularspeed;
	int speedx;
	int speedy;
	int health;
	int frame;
	int type;
	int attack;
	int attackdelay;
	void *origin;
	int enemymode;
	int enemycontrol;
	int enemycounter;
        int enemybored;
	int special; /* Number of grenades for enemies */
        int areanum; /* For collision detection */
} ACTOR;

typedef struct
{
        int xb;
        int yb;
        int angle;
} STAIREND;

typedef struct
{
        STAIREND src, dest;
} STAIRS;

typedef struct
{
	int xb;
	int yb;
} LIFTFLOOR;

typedef struct
{
	int floor;
	int destfloor;
	int floors;
	int speed;
	int startdelay;
	int firstfloor;
	int angle;
	LIFTFLOOR liftfloor[MAX_FLOOR];
} LIFT;

#define BND_MIN_HITTIME 10
enum bnd_sound {
        BNDS_OFF,
        BNDS_ON,
        BNDS_HIT,
        BNDS_RUN
};


int bofhmain(void);
void playmusic(int tune);
void stopmusic(void);
void checkglobalkeys(void);
void titlescreen(void);
void loadhiscore(void);
void savehiscore(void);
void loadconfig(void);
void saveconfig(void);
int initstuff(void);
int menu(void);
int restartdialog(void);
int selectdifficulty(void);
int optionsmenu(void);
int keydialog(void);
void showsplash(void);
void testvictory(void);
void endpart(void);
void printstatus(void);
void doscroll(void);
void drawshadow(void);
void fireeffect(void);
void playercontrol(ACTOR *aptr);
void playerattack(ACTOR *aptr);
void fistmancontrol(ACTOR *aptr);
void fistmanattack(ACTOR *aptr);
void pistolmancontrol(ACTOR *aptr);
void pistolmanattack(ACTOR *aptr);
void shotgunmancontrol(ACTOR *aptr);
void shotgunmanattack(ACTOR *aptr);
void sadistcontrol(ACTOR *aptr);
void sadistattack(ACTOR *aptr);
void uzimancontrol(ACTOR *aptr);
void uzimanattack(ACTOR *aptr);
void leadercontrol(ACTOR *aptr);
void leaderattack(ACTOR *aptr);
void techniciancontrol(ACTOR *aptr);
void technicianattack(ACTOR *aptr);
void enemyboredom(ACTOR *aptr);
void opendoor(int xp, int yp);
void closedoor(int xp, int yp);
int initgame(char *missionfilename);
void drawactors(void);
void drawsight(void);
void drawinstr(int num, int x, int y);
void drawnearcloset(void);
void drawcloset(int num);
void drawscanner(void);
void countdown(void);
void alertenemies(int x, int y, int maxdist);
int checkblockactors(int xb, int yb);
int checkvision(ACTOR *aptr);
int checkwalls(ACTOR *aptr);
int checkblockvision(int xb, int yb);
void spawnbazookaprojectile(ACTOR *aptr);
void spawnmedikit(ACTOR *aptr);
ACTOR *spawnitem(ACTOR *aptr, int type);
void moveitem(ACTOR *aptr);
void spawnbullet(ACTOR *aptr, int inaccuracy, int illuminated, int mayricoche);
void spawnarrow(ACTOR *aptr, int inaccuracy);
void grenadeexplode(ACTOR *aptr, int extraflavor);
int spawngrenade(ACTOR *aptr, int inaccuracy, int speed);
void dropgrenades(ACTOR *aptr);
void grenadeattack(ACTOR *aptr);
void spawnshell(ACTOR *aptr);
void spawnshotgshell(ACTOR *aptr);
void spawnflame(ACTOR *aptr, int inaccuracy);
void movegrenade(ACTOR *aptr);
void movearrow(ACTOR *aptr);
void moveflame(ACTOR *aptr);
void movebullet(ACTOR *aptr);
void movericochet(ACTOR *aptr);
void moveshell(ACTOR *aptr);
void moveshard(ACTOR *aptr);
void movebazookaprojectile(ACTOR *aptr);
void moveblood(ACTOR *aptr);
void flashclosetlights(void);
void spawnblood(ACTOR *cptr, int number, int angle);
void detectplayer(ACTOR *aptr);
void attackplayer(ACTOR *aptr);
void attackplayer_notaunt(ACTOR *aptr);
void allpatrol(void);
void allattack(void);
void structuredamage(int x, int y);
void breakglass(ACTOR *aptr, int x, int y);
void spawnshard(ACTOR *aptr);
void spawnchunk(ACTOR *aptr, int damage);
void movechunk(ACTOR *aptr);
void moveactors(void);
void movelifts(void);
void findnearestfloor(int liftnum, int *x, int *y);
int findangle(int sx, int sy, int ex, int ey);
int finddist(int sx, int sy, int ex, int ey);
void makecollareas(void);
ACTOR *collision(ACTOR *aptr, int x, int y, int *prefdist);
ACTOR *collision_no_origin(ACTOR *aptr);
int angledist(int srcangle, int destangle);
int domove(ACTOR *aptr);
void getgamespeed(void);
int checkstairs(ACTOR *aptr);
int domove_nodoors(ACTOR *aptr);
int isserverroom(int x, int y);
void checkdoorclose(ACTOR *aptr, int oldx, int oldy);
void clearactors(void);
ACTOR *spawnactor(int type, int x, int y, int angle);
void checkhit(ACTOR *aptr);
void punish(ACTOR *cptr, ACTOR *aptr, int damage);
void game(char *missionname);
int cheating(void);
void reward(int points);
int checkhiscore(void);
void checkcheats(void);
int initmouse(void);
void updatemouse(void);
void printgamemsg(char *msg, int time);
void printtext(int x, int y, unsigned spritefile, char *string);
void printtext_center(int y, unsigned spritefile, char *string);
void playfx(int chan, int fx, unsigned freq, unsigned char volume, unsigned char panning);
void playpositionalfx(int x, int y, int chan, int fx, unsigned freq, int volume);
void updatepositionalfx(int x, int y, int chan);
// ***
void playownedpositionalfx(ACTOR *aptr, int x, int y, int chan, int fx, unsigned freq, int volume);
void updateownedpositionalfx(ACTOR *aptr, int x, int y, int chan);
void killownedpositionalfx(ACTOR *aptr, int chan);
int squareroot(int x);
void fireinnerloop(void);
void blitfire(void);
