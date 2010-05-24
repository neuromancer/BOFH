#include "bofh.h"
#include "sincos.h"
#include "extern.h"

typedef struct
{
	int xb;
	int yb;
	int used;
} RANDOMLOCATION;

int begintime[MAX_DIFF];
int beginbombs[MAX_DIFF];
int beginterrorists[MAX_DIFF];
int beginpistolmen[MAX_DIFF];
int beginshotgunmen[MAX_DIFF];
int beginuzimen[MAX_DIFF];
int beginsadists[MAX_DIFF];
int beginleaders[MAX_DIFF];
int beginwhips[MAX_DIFF];
int begindrills[MAX_DIFF];
int begincrossbows[MAX_DIFF];
int beginpistols[MAX_DIFF];
int beginshotguns[MAX_DIFF];
int beginuzis[MAX_DIFF];
int begingrenades[MAX_DIFF];
int beginbazookas[MAX_DIFF];
int beginscanners[MAX_DIFF];
int beginsmallmedikits[MAX_DIFF];
int beginbigmedikits[MAX_DIFF];

RANDOMLOCATION randomloc[MAX_ACTOR];
int randomlocleft;
int totalrandomloc;
int instrnum;
int bofhstartx,bofhstarty,bofhstartangle;
FILE *mf = NULL; /* Mission file handle */

int openmf(char *missionfilename);
int mfreadstring(char *dest, int length);
void closemf(void);
int mfreadint(void);
void mfreadlocations(void);
void initrandomactor(int amount, int type);

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

int initgame(char *missionfilename)
{
        int c;
        ACTOR *aptr;
        char *textptr;
        int temptype, tempxb, tempyb, tempangle;

        if (!openmf(missionfilename)) return 0;

        /* Read in player start pos. */
        bofhstartx = (mfreadint() << 12)+8*DEC;
        bofhstarty = (mfreadint() << 12)+8*DEC;
        bofhstartangle = mfreadint();

        /* Read in dimensions of server room */
        srminx = mfreadint();
        srminy = mfreadint();
        srmaxx = mfreadint();
        srmaxy = mfreadint();

        /* Read in victory condition & texts */
        victorybits = mfreadint();

	textptr = briefingtext;
	for (;;)
	{
		mfreadstring(textptr, 80);
		if (*textptr == '$') break;
		textptr += strlen(textptr)+1;
	}
	textptr = victorytext;
	for (;;)
	{
		mfreadstring(textptr, 80);
		if (*textptr == '$') break;
		textptr += strlen(textptr)+1;
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
        for (c = 0; c < MAX_DIFF; c++)
        {
                beginbazookas[c] = mfreadint();
                beginbazookas[c] = 16;
        }
        for (c = 0; c < MAX_DIFF; c++)	beginscanners[c] = mfreadint();
        for (c = 0; c < MAX_DIFF; c++)	beginsmallmedikits[c] = mfreadint();
        for (c = 0; c < MAX_DIFF; c++)	beginbigmedikits[c] = mfreadint();

        /* Read in enemy attack probabilities */
        for (c = 0; c < MAX_DIFF; c++) fistpr[c] = mfreadint();
        for (c = 0; c < MAX_DIFF; c++) pistolpr[c] = mfreadint();
        for (c = 0; c < MAX_DIFF; c++) shotgunpr[c] = mfreadint();
        for (c = 0; c < MAX_DIFF; c++) uzipr[c] = mfreadint();
        for (c = 0; c < MAX_DIFF; c++) crossbowpr[c] = mfreadint();
        for (c = 0; c < MAX_DIFF; c++) flamepr[c] = mfreadint();

        /* Clear all actors */
        clearactors();

        /* Spawn player's actor */
	aptr = spawnactor(ACTOR_BOFH, bofhstartx, bofhstarty, bofhstartangle);
        if (!aptr)
        {
        	closemf();
        	return 0;
        }

	aptr->health = 100;

	xpos = bofhstartx-160*DEC;
	ypos = bofhstarty-120*DEC;
        paused = 0;
        gameover = 0;
        fastforward = 0;
        throwstrength = 0;
        scannerdelay = 0;
        showinstr = 0;
        showinstrtime = 0;
        gamemsgtime = 0;
        trapmsg = 0;
        kills = 0;
        victory = 0;
        liftsound = -1;
        speaker = NULL;

        bombs = 0;
        beginservers = 0;
        beginworkstations = 0;
        computers = 0;
        terrorists = 0;
        leaders = 0;
        instrnum = 0;

        /* Init player weapons & time */
	weapon = WEAP_FISTS;
	bnd_sound = BNDS_OFF;
	bnd_hittime = 0;
	if (difficulty == DIFF_PRACTICE)
	{
                unsigned i;
                for (i=0; i<WEAPNUM; ++i)
                        ammo[i] = maxammo[i];
		gametime = 0;
	}
	else
	{
                unsigned i;
                for (i=0; i<WEAPNUM; ++i)
                        ammo[i] = 0;
                ammo[WEAP_FISTS] = maxammo[WEAP_FISTS];
		gametime = begintime[difficulty] * 60 * 70;
	}

        /* Read in stairs */
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
        numstairs = c;

        /* Read in closets */
        for (c=0;;c++)
        {
        	int d;
        	closet[c].xb = mfreadint();
        	closet[c].yb = mfreadint();
        	closet[c].anim = mfreadint();
        	if ((!closet[c].xb) && (!closet[c].yb)) break;
        	mfreadstring(closet[c].name, 16);
        	for (d = 0; d < MAX_EQUIP; d++) closet[c].equipment[d] = mfreadint();
        }
        numclosets = c;

        /* Read in & init lifts */
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
        	lift[c].floor = ((rand()%lift[c].floors)+1)*1000;
        	lift[c].destfloor = lift[c].floor;
        }
        numlifts = c;

        /* Init stationary actors */
        for (;;)
        {
        	tempxb = mfreadint();
        	tempyb = mfreadint();
        	temptype = mfreadint();
        	tempangle = mfreadint();
        	if (!temptype) break;
        	aptr = spawnactor(temptype, (tempxb*16+8)*DEC, (tempyb*16+8)*DEC, tempangle);
        	if (aptr)
        	{
			if ((aptr->type >= ACTOR_FIRSTCOMPUTER) &&
			    (aptr->type <= ACTOR_LASTCOMPUTER))
			{
				aptr->health = comphealth[aptr->type - ACTOR_FIRSTCOMPUTER];
	           		computers++;
                                if (isserverroom(aptr->x, aptr->y)) beginservers++;
                                        else beginworkstations++;
	           	}
	           	if ((aptr->type >= ACTOR_KEYBOARD) &&
	           	    (aptr->type <= ACTOR_VCR_AND_TV))
	           	{
	           		aptr->health = 10;
	           	}
                        /* Grenades' amount will always be 3 */
                        if (aptr->type == ACTOR_GRENADE) aptr->health = 3;

                        /* Enemy-specific initialization */
                        if ((aptr->type >= ACTOR_FIRSTENEMY) && (aptr->type <= ACTOR_LASTENEMY))
                        {
                                terrorists++;
        			aptr->health = enemyhealth[aptr->type - ACTOR_FISTMAN];
        			aptr->enemymode = MODE_PATROL;
        			aptr->enemycontrol = 0;
        			aptr->enemycounter = 0;
        	                aptr->enemybored = 0;
        	                /* Technician has bomb instructions */
        			if (aptr->type == ACTOR_TECHNICIAN)
        			{
        			 	aptr->special = instrnum;
        			 	instrnum++;
        			}
        	                else
        	                {
        		                /* Others, except the sadist, can have grenades */
        	                        if ((aptr->type != ACTOR_SADIST) && (grenadecheat || ((rand() % 18) < difficulty)))
        	                        {
        	                                aptr->special = (rand() % 3)+1;
        	                        }
	                                else aptr->special = 0;
                                }
	                }
	                if (aptr->type == ACTOR_LEADER)
	                {
	                        leaders++;
                        }
                }
        }

        /* Init randomly located collectable items */
	mfreadlocations();
	initrandomactor(beginwhips[difficulty], ACTOR_CAT5);
	mfreadlocations();
	initrandomactor(begindrills[difficulty], ACTOR_BND);
	mfreadlocations();
	initrandomactor(begincrossbows[difficulty], ACTOR_CROSSBOW);
	mfreadlocations();
	initrandomactor(beginpistols[difficulty], ACTOR_PISTOL);
	mfreadlocations();
	initrandomactor(beginshotguns[difficulty], ACTOR_SHOTGUN);
	mfreadlocations();
	initrandomactor(beginuzis[difficulty], ACTOR_UZI);
	mfreadlocations();
	initrandomactor(begingrenades[difficulty], ACTOR_GRENADE);
	mfreadlocations();
	initrandomactor(beginbazookas[difficulty], ACTOR_BAZOOKA);
	mfreadlocations();
	initrandomactor(beginscanners[difficulty], ACTOR_SCANNER);
	mfreadlocations();
	initrandomactor(beginbigmedikits[difficulty], ACTOR_BIGMEDIKIT);
	initrandomactor(beginsmallmedikits[difficulty], ACTOR_SMALLMEDIKIT);

	/* Init leaders */
	mfreadlocations();
	initrandomactor(beginleaders[difficulty], ACTOR_LEADER);

	/* Init other enemies */
	mfreadlocations();
	initrandomactor(beginbombs[difficulty], ACTOR_TECHNICIAN);
	initrandomactor(beginsadists[difficulty], ACTOR_SADIST);
	initrandomactor(beginuzimen[difficulty], ACTOR_UZIMAN);
	initrandomactor(beginshotgunmen[difficulty], ACTOR_SHOTGUNMAN);
	initrandomactor(beginpistolmen[difficulty], ACTOR_PISTOLMAN);
        initrandomactor(beginterrorists[difficulty]-
                        beginbombs[difficulty]-
                        beginsadists[difficulty]-
                        beginuzimen[difficulty]-
                        beginshotgunmen[difficulty]-
                        beginpistolmen[difficulty]-
                        beginleaders[difficulty], ACTOR_FISTMAN);

        if (enemycheat)
        {
		static const int randomenemytypes[] = {
			ACTOR_FISTMAN,
			ACTOR_PISTOLMAN,
			ACTOR_SHOTGUNMAN,
			ACTOR_UZIMAN,
			ACTOR_SADIST
		};
		const size_t count = sizeof(randomenemytypes) / sizeof(randomenemytypes[1]);
		initrandomactor(randomlocleft/3, randomenemytypes[rand()%count]);
		initrandomactor(randomlocleft/2, randomenemytypes[rand()%count]);
		initrandomactor(randomlocleft/1, randomenemytypes[rand()%count]);
        }

	/* Init bombs */
        numbombs = beginbombs[difficulty];
        if (numbombs > numclosets) numbombs = numclosets;
        for (c = 0; c < numbombs; c++)
        {
                int d, e, ok;

                for (d = 0; d < 4; d++)
                {
                        ok = 0;
                        while (!ok)
                        {
	                        ok = 1;
	                	bomb[c].wireorder[d] = rand() & 3;
	                	for (e = 0; e < d; e++)
	                	{
	                		if (bomb[c].wireorder[e] == bomb[c].wireorder[d])
	                			ok = 0;
	                	}
	                }
	        }

	        ok = 0;
	        while (!ok)
	        {
                        ok = 1;
        		bomb[c].location = rand() % numclosets;
			for (d = 0; d < c; d++)
			{
				if (bomb[d].location == bomb[c].location) ok = 0;
			}
		}
                bomb[c].instructions = instrcheat;
                bomb[c].wiresleft = 4;
        }
        bombs = c;

        closemf();
        return 1;
}

void mfreadlocations(void)
{
        int c;

	for (c=0; c < MAX_ACTOR; c++)
	{
        	randomloc[c].used = 0;
        	randomloc[c].xb = mfreadint();
        	randomloc[c].yb = mfreadint();

        	if ((!randomloc[c].xb) && (!randomloc[c].yb))
        		break;
        }
        totalrandomloc = c;
        randomlocleft = c;
}

void initrandomactor(int amount, int type)
{
        ACTOR *aptr;
        int pos = 0;

	while(amount > 0)
	{
                int ok = 0;

		if (!randomlocleft) break;
                while (!ok)
                {
                 	pos = rand() % totalrandomloc;
                 	if (!randomloc[pos].used)
                 	{
                                /*
                                 * Technicians must not be spawned inside
                                 * the server room or game may be impossible
                                 * to complete
                                 */
                 		if (type == ACTOR_TECHNICIAN)
                         	{
                         		if (!isserverroom(
                         			randomloc[pos].xb<<12,
                         			randomloc[pos].yb<<12)) ok = 1;
				}
				else ok = 1;
			}
		}
                randomlocleft--;
                randomloc[pos].used = 1;
                aptr = spawnactor(type, (randomloc[pos].xb*16+8)*DEC,
                		 (randomloc[pos].yb*16+8)*DEC, rand() & 0x3ff);
                if (!aptr) break;

                /* Grenades' amount will always be 3 */
                if (aptr->type == ACTOR_GRENADE) aptr->health = 3;

                /* Enemy-specific initialization */
                if ((aptr->type >= ACTOR_FIRSTENEMY) && (aptr->type <= ACTOR_LASTENEMY))
                {
                        terrorists++;
			aptr->health = enemyhealth[aptr->type - ACTOR_FISTMAN];
			aptr->enemymode = MODE_PATROL;
			aptr->enemycontrol = 0;
			aptr->enemycounter = 0;
	                aptr->enemybored = 0;
	                /* Technician has bomb instructions */
			if (aptr->type == ACTOR_TECHNICIAN)
			{
			 	aptr->special = instrnum;
			 	instrnum++;
			}
	                else
	                {
		                /* Others, except the sadist, can have grenades */
	                        if ((aptr->type != ACTOR_SADIST) && (grenadecheat || ((rand() % 18) < difficulty)))
	                        {
	                                aptr->special = (rand() % 3)+1;
	                        }
	                        else aptr->special = 0;
	                }
	                if (aptr->type == ACTOR_LEADER)
	                {
	                        leaders++;
                        }
                }
                amount--;
        }
}

