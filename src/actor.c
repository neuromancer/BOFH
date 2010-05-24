#include "bofh.h"
#include "sincos.h"
#include "extern.h"

static void ensure_bnd_sound (enum bnd_sound bnds);
static int takes_weapon(unsigned type, unsigned amnt);
int traceoutofwall(ACTOR *aptr);

#define COLLAREASIZE 256 /* (in pixels) */
#define COLLAREAROW (MAXMAPSIZE*16/COLLAREASIZE+1)
#define COLLAREAS (COLLAREAROW*COLLAREAROW)

int ca_size[COLLAREAS];
int ca_start[COLLAREAS];
int ca_temp[COLLAREAS];
ACTOR *ca_list[MAX_ACTOR];

void makecollareas(void)
{
        int c;
        ACTOR *cptr = &actor[0];
        int list;

        /* Clear size (number of actors) of each collision area list */
        memset(ca_size, 0, sizeof ca_size);

        /* Calculate the size of each collision area list */
        for (c = 0; c < actors; c++)
        {
                /* Actor alive & has radius? */
                if (actorradius[cptr->type])
                {
                        /* Calculate area number */
                        int areanum = (cptr->y / COLLAREASIZE / DEC)*COLLAREAROW +
                          (cptr->x / COLLAREASIZE / DEC);
                        /* Paranoid check */
                        if (areanum < COLLAREAS)
                        {
                                /* Increase size of area */
                                ca_size[areanum]++;
                                cptr->areanum = areanum;
                        }
                        else
                        cptr->areanum = -1;
                }
                else
                cptr->areanum = -1; /* Not included in collision */

                cptr++;
        }
        /* Make the collision area list startpointers */
        list = 0;
        for (c = 0; c < COLLAREAS; c++)
        {
                ca_start[c] = list;
                ca_temp[c] = list;
                list += ca_size[c];
        }
        /* Now place actors into lists */
        cptr = &actor[0];
        for (c = 0; c < actors; c++)
        {
                if (cptr->areanum >= 0)
                {
                        ca_list[ca_temp[cptr->areanum]] = cptr;
                        ca_temp[cptr->areanum]++;
                }
                cptr++;
        }
}

static ACTOR *areacollision(ACTOR *aptr, int x, int y, int *prefdist, int areanum)
{
        int list;
        int c;
        int aradius;
        if (areanum < 0) return NULL;
        if (areanum >= COLLAREAS) return NULL;
        if (!ca_size[areanum]) return NULL;
        list = ca_start[areanum];
        aradius = actorradius[aptr->type];
        for (c = 0; c < ca_size[areanum]; c++)
        {
                ACTOR *cptr = ca_list[list++];
        	int radius = actorradius[cptr->type];
                /* This has been already checked in makearealists() but
                 * the situation might have changed during
                 * this frame */
                if (radius && (cptr != aptr))
                {
                      	int xdist = abs(cptr->x - x);
                       	int ydist = abs(cptr->y - y);
                        /* actorradius[] is a bit loose to make
                         * enemies easy to hit.  If aradius == 0 then
                         * aptr is a missile of some kind so respect
                         * that.  However if aradius != 0 then this is
                         * a mob and can be packed somewhat tighter.  */
                        if (aradius != 0)
                                radius = (radius + aradius) / 2;
                        if ((xdist <= radius) && (ydist <= radius))
                        {
                                if (prefdist != NULL)
                                        *prefdist = radius;
                                return cptr;
                        }
                }
        }
        return NULL;
}

ACTOR *areacollision_no_origin(ACTOR *aptr, int areanum)
{
        int list;
        int c;
        if (areanum < 0) return NULL;
        if (areanum >= COLLAREAS) return NULL;
        if (!ca_size[areanum]) return NULL;
        list = ca_start[areanum];
        for (c = 0; c < ca_size[areanum]; c++)
        {
                ACTOR *cptr = ca_list[list++];
        	int radius = actorradius[cptr->type];
                /* This has been already checked in makearealists() but
                 * the situation might have changed during
                 * this frame */
                if (radius)
                {
                      	int xdist = abs(cptr->x - aptr->x);
                       	int ydist = abs(cptr->y - aptr->y);
                        if ((aptr->origin != cptr) && (xdist <= radius) && (ydist <= radius))
                        {
                                return cptr;
                        }
                }
        }
        return NULL;
}

ACTOR *collision(ACTOR *aptr, int x, int y, int *prefdist)
{
        ACTOR *cptr;
        int areanum = (y / COLLAREASIZE / DEC)*COLLAREAROW +
                (x / COLLAREASIZE / DEC);
        if (areanum < 0) return NULL;
        if (areanum >= COLLAREAS) return NULL;
        /* Check this area & all neighbor areas */
        cptr = areacollision(aptr, x, y, prefdist, areanum);
        if (cptr) return cptr;
        cptr = areacollision(aptr, x, y, prefdist, areanum-1);
        if (cptr) return cptr;
        cptr = areacollision(aptr, x, y, prefdist, areanum+1);
        if (cptr) return cptr;
        cptr = areacollision(aptr, x, y, prefdist, areanum+COLLAREAROW-1);
        if (cptr) return cptr;
        cptr = areacollision(aptr, x, y, prefdist, areanum+COLLAREAROW);
        if (cptr) return cptr;
        cptr = areacollision(aptr, x, y, prefdist, areanum+COLLAREAROW+1);
        if (cptr) return cptr;
        cptr = areacollision(aptr, x, y, prefdist, areanum-COLLAREAROW-1);
        if (cptr) return cptr;
        cptr = areacollision(aptr, x, y, prefdist, areanum-COLLAREAROW);
        if (cptr) return cptr;
        cptr = areacollision(aptr, x, y, prefdist, areanum-COLLAREAROW+1);
        if (cptr) return cptr;
        return NULL;
}

ACTOR *collision_no_origin(ACTOR *aptr)
{
        ACTOR *cptr;
        int areanum = (aptr->y / COLLAREASIZE / DEC)*COLLAREAROW +
                (aptr->x / COLLAREASIZE / DEC);
        if (areanum < 0) return NULL;
        if (areanum >= COLLAREAS) return NULL;
        /* Check this area & all neighbor areas */
        cptr = areacollision_no_origin(aptr, areanum);
        if (cptr) return cptr;
        cptr = areacollision_no_origin(aptr, areanum-1);
        if (cptr) return cptr;
        cptr = areacollision_no_origin(aptr, areanum+1);
        if (cptr) return cptr;
        cptr = areacollision_no_origin(aptr, areanum+COLLAREAROW-1);
        if (cptr) return cptr;
        cptr = areacollision_no_origin(aptr, areanum+COLLAREAROW);
        if (cptr) return cptr;
        cptr = areacollision_no_origin(aptr, areanum+COLLAREAROW+1);
        if (cptr) return cptr;
        cptr = areacollision_no_origin(aptr, areanum-COLLAREAROW-1);
        if (cptr) return cptr;
        cptr = areacollision_no_origin(aptr, areanum-COLLAREAROW);
        if (cptr) return cptr;
        cptr = areacollision_no_origin(aptr, areanum-COLLAREAROW+1);
        if (cptr) return cptr;
        return NULL;
}

// ***
int traceoutofwall(ACTOR *aptr) {
        // Moves an actor (projectile) out of wall where it ended up at
        // high speed. The actor's speed is inspected to find its previous
        // position outside the wall.

        int maxiterations = 5, iterations;
        int blkinf, bestwallx, bestwally, bestclearx, bestcleary, newx, newy;
        if (!(aptr->speedx || aptr->speedy)) {
          // No can do!
          return 0;
        }
        bestwallx = aptr->x;
        bestwally = aptr->y;
        bestclearx = aptr->x-aptr->speedx;
        bestcleary = aptr->y-aptr->speedy;
        blkinf = map_getblockinfo(0,bestclearx/DEC,bestcleary/DEC)|map_getblockinfo(1,bestclearx/DEC,bestcleary/DEC);
        if (blkinf & INF_WALL) {
          // No can do!
          return 0;
        }
        blkinf = map_getblockinfo(0,bestwallx/DEC,bestwally/DEC)|map_getblockinfo(1,bestwallx/DEC,bestwally/DEC);
        if (!(blkinf & INF_WALL)) {
          // It wasn't inside the wall in the first place...
          return 0;
        }
        for (iterations = 0; iterations < maxiterations; iterations++) {
          newx = (bestclearx+bestwallx)/2;
          newy = (bestcleary+bestwally)/2;
          blkinf = map_getblockinfo(0,newx/DEC,newy/DEC)|map_getblockinfo(1,newx/DEC,newy/DEC);
          if (blkinf & INF_WALL) {
            bestwallx = newx;
            bestwally = newy;
          } else {
            bestclearx = newx;
            bestcleary = newy;
          }
        }
        aptr->x = bestclearx;
        aptr->y = bestcleary;
	return 1;
}


void moveactors(void)
{
        int c;
        ACTOR *aptr;
        aptr = &actor[0];

        if (!actors) return;

        for (c = 0; c < actors; c++)
        {
        	if (aptr->type)
        	{
        		switch (aptr->type)
        		{
	        		case ACTOR_BOFH:
	                        playercontrol(aptr);
	                        break;

	                        case ACTOR_MUZZLE:
	                        aptr->frame++;
	                        if (aptr->frame > 2) aptr->type = ACTOR_NONE;
	                        break;

	                        case ACTOR_BULLET:
				movebullet(aptr);
	                        break;

	                        case ACTOR_ARROW:
				movearrow(aptr);
	                        break;

                                // ***
                                case ACTOR_BAZOOKA_PROJECTILE:
                                movebazookaprojectile(aptr);
                                break;


	                        case ACTOR_FLAME:
				moveflame(aptr);
	                        break;

	                        case ACTOR_FLYINGGRENADE:
				movegrenade(aptr);
	                        break;

	                        case ACTOR_SMOKE:
	                        aptr->frame++;
	                        if (aptr->frame >= 12) aptr->type = ACTOR_NONE;
	                        break;

	                        case ACTOR_EXPLOSION:
	                        aptr->frame++;
	                        if (aptr->frame >= 20) aptr->type = ACTOR_NONE;
	                        break;

	                        case ACTOR_FISTHIT:
	                        case ACTOR_WHIPHIT:
                                case ACTOR_BNDHIT:
	                        checkhit(aptr);
	                        break;

	                        case ACTOR_CHUNK:
	                        movechunk(aptr);
	                        break;

	                        case ACTOR_SHARD:
	                        moveshard(aptr);
	                        break;

	                        case ACTOR_SPARK:
	                        aptr->frame++;
	                        if (aptr->frame >= 16) aptr->type = ACTOR_NONE;
	                        break;

                                case ACTOR_SHELL: // ***
                                moveshell(aptr);  //
                                break;            //

                                case ACTOR_SHOTGSHELL:  // ***
                                moveshell(aptr);        //
                                break;                  //

                                case ACTOR_RICOCHET:    // ***
                                movericochet(aptr);     //
                                break;                  //

	                        case ACTOR_BLOOD:
	                        moveblood(aptr);
	                        break;

	                        case ACTOR_FISTMAN:
                                if (aptr == speaker) updateownedpositionalfx(aptr, aptr->x, aptr->y, FXCHAN_SPEECH); // ***
                                enemyboredom(aptr);
	                        fistmancontrol(aptr);
	                        break;

	                        case ACTOR_PISTOLMAN:
                                if (aptr == speaker) updateownedpositionalfx(aptr, aptr->x, aptr->y, FXCHAN_SPEECH); // ***
                                enemyboredom(aptr);
                                pistolmancontrol(aptr);
				break;

	                        case ACTOR_SHOTGUNMAN:
                                if (aptr == speaker) updateownedpositionalfx(aptr, aptr->x, aptr->y, FXCHAN_SPEECH); // ***
                                enemyboredom(aptr);
                                shotgunmancontrol(aptr);
	                        break;

	                        case ACTOR_TECHNICIAN:
                                if (aptr == speaker) updateownedpositionalfx(aptr, aptr->x, aptr->y, FXCHAN_SPEECH); // ***
                                enemyboredom(aptr);
                                techniciancontrol(aptr);
	                        break;

	                        case ACTOR_UZIMAN:
                                if (aptr == speaker) updateownedpositionalfx(aptr, aptr->x, aptr->y, FXCHAN_SPEECH); // ***
                                enemyboredom(aptr);
                                uzimancontrol(aptr);
	                        break;

	                        case ACTOR_SADIST:
                                if (aptr == speaker) updateownedpositionalfx(aptr, aptr->x, aptr->y, FXCHAN_SPEECH); // ***
                                enemyboredom(aptr);
                                sadistcontrol(aptr);
	                        break;

	                        case ACTOR_LEADER:
                                if (aptr == speaker) updateownedpositionalfx(aptr, aptr->x, aptr->y, FXCHAN_SPEECH); // ***
                                enemyboredom(aptr);
                                leadercontrol(aptr);
	                        break;

	                        case ACTOR_DEADFISTMAN:
	                        case ACTOR_DEADPISTOLMAN:
	                        case ACTOR_DEADSHOTGUNMAN:
	                        case ACTOR_DEADUZIMAN:
	                        case ACTOR_DEADTECHNICIAN:
	                        case ACTOR_DEADLEADER:
	                        case ACTOR_DEADSADIST:
	                        if (aptr->frame < 20)
	                        {
                                        aptr->angle += 20;
                                        aptr->angle &= 0x3ff;
	                        	aptr->frame++;
	                        	/* if (rand() & 1) spawnblood(aptr, 1, rand() & 0x3ff); */
	                        }

	                        break;

	                        case ACTOR_DEADBOFH:
	                        if (aptr->frame < 20)
	                        {
	                        	aptr->frame++;
	                        	if (rand() & 1) spawnblood(aptr, 1, rand() & 0x3ff);
	                        }
	                        if (!gameover) gameover++;
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
                                case ACTOR_BAZOOKA:
                                case ACTOR_BAZOOKA_STRAP:
                                case ACTOR_BAZOOKA_USED:
                                case ACTOR_CROSSBOW:
                                case ACTOR_SCANNER:

	                        moveitem(aptr);
	                        break;

			        case ACTOR_BEAM:
			        if (bombs)
			        {
			        	aptr->frame++;
			        	if (!trapmsg)
			        	{
				        	if (finddist(aptr->x, aptr->y, actor[0].x, actor[0].y) < 40)
				        	{
			        			trapmsg = 1;
			        			printgamemsg("BEWARE THE TRAP!", 200);
			        		}
			        	}
			        	if (((actor[0].x & 0xfffff000) == (aptr->x & 0xfffff000)) &&
			        	    ((actor[0].y & 0xfffff000) == (aptr->y & 0xfffff000)))
			        	{
			        		if (gametime)
			        		{
			        			gametime = 0;
			        			printgamemsg("OH SHIT!", 200);
			        		}
			        	}
                                }
                                else
                                {
                                	aptr->type = ACTOR_NONE;
                                }
                                break;
	                }
                }
                aptr++;
        }
}

void drawactors(void)
{
        int c, xp, yp;

        /* First draw machines, items & corpses */

        ACTOR *aptr;
        if (!actors) return;
        aptr = &actor[actors-1];
        for (c = 0; c < actors; c++)
        {
        	if (aptr->type)
        	{
                        if ((aptr == &actor[0]) || (checkvision(aptr)))
                        {
	        		int frame;
	        		int angleturn = ((aptr->angle + 64) / 128) & 7;
	                       	xp = aptr->x & 0xffffff00;
	                       	yp = aptr->y & 0xffffff00;
	                       	xp -= (xpos & 0xffffff00);
	                       	yp -= (ypos & 0xffffff00);
	                       	xp /= DEC;
	                       	yp /= DEC;
	                       	xp -= xshift;
	                       	xp -= yshift;

	        		switch (aptr->type)
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
                                        if (aptr->frame & 8)
                                        {
			                        frame = 0x00030063 + (angleturn >> 1);
        				        gfx_drawsprite(xp, yp, frame);
                                        }
			                break;

		        		case ACTOR_DEADFISTMAN:
		        		frame = 0x00040059 + angleturn * 2 + aptr->frame / 20;
			                gfx_drawsprite(xp, yp, frame);
		                        break;

		        		case ACTOR_DEADPISTOLMAN:
		        		frame = 0x00050041 + angleturn * 2 + aptr->frame / 20;
			                gfx_drawsprite(xp, yp, frame);
		                        break;

		        		case ACTOR_DEADSHOTGUNMAN:
		        		frame = 0x00060041 + angleturn * 2 + aptr->frame / 20;
			                gfx_drawsprite(xp, yp, frame);
		                        break;

		        		case ACTOR_DEADUZIMAN:
		        		frame = 0x00070041 + angleturn * 2 + aptr->frame / 20;
			                gfx_drawsprite(xp, yp, frame);
		                        break;

		        		case ACTOR_DEADSADIST:
		        		frame = 0x000b0059 + angleturn * 2 + aptr->frame / 20;
			                gfx_drawsprite(xp, yp, frame);
		                        break;

		        		case ACTOR_DEADLEADER:
		        		frame = 0x000a0041 + angleturn * 2 + aptr->frame / 20;
			                gfx_drawsprite(xp, yp, frame);
		                        break;

		        		case ACTOR_DEADTECHNICIAN:
		        		frame = 0x00080059 + angleturn * 2 + aptr->frame / 20;
			                gfx_drawsprite(xp, yp, frame);
		                        break;

		        		case ACTOR_DEADBOFH:
		        		frame = SPRI_DEADBOFH1 + aptr->frame / 20;
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
                                        frame = SPRI_BAZOOKA_STRAP + aptr->frame;
                                        gfx_drawsprite(xp, yp, frame);
                                        break;

                                        case ACTOR_BAZOOKA_USED: // ***
                                        frame = SPRI_BAZOOKA_USED + aptr->frame;
                                        gfx_drawsprite(xp, yp, frame);
                                        break;

                                        case ACTOR_SHELL:                 // ***
                                        frame = SPRI_SHELL + aptr->frame; //
                                        gfx_drawsprite(xp, yp, frame);  //
                                        break;                            //

                                        case ACTOR_SHOTGSHELL:                 // ***
                                        frame = SPRI_SHOTGSHELL + aptr->frame; //
                                        gfx_drawsprite(xp, yp, frame);       //
                                        break;                                 //

                                        case ACTOR_ARROW: // ***
                                        {
                                          int xp2 = aptr->enemycontrol & 0xffffff00;
                                          int yp2 = aptr->enemycounter & 0xffffff00;
                                          xp2 -= (xpos & 0xffffff00);
                                          yp2 -= (ypos & 0xffffff00);
                                          xp2 /= DEC;
                                          yp2 /= DEC;
	  	                       	xp2 -= xshift;
		                       	yp2 -= yshift;
                                          gfx_line(xp, yp, xp2, yp2, 6);
                                        }
                                        break;

                                        case ACTOR_RICOCHET: // ***
                                        if (aptr->special) {
                                          int xp2 = aptr->enemycontrol & 0xffffff00;
                                          int yp2 = aptr->enemycounter & 0xffffff00;
                                          xp2 -= (xpos & 0xffffff00);
                                          yp2 -= (ypos & 0xffffff00);
                                          xp2 /= DEC;
                                          yp2 /= DEC;
	  	                       	xp2 -= xshift;
		                       	yp2 -= yshift;
                                          gfx_line(xp, yp, xp2, yp2, 7);
                                        } else {
                                          frame = SPRI_RICOCHET + aptr->frame;
                                          gfx_drawsprite(xp, yp, frame);
                                        }
                                        break;

                                        // ***
                                        case ACTOR_BULLET:
                                        if (aptr->special && (aptr->frame > 3)) {
                                          int xp2 = aptr->enemycontrol & 0xffffff00;
                                          int yp2 = aptr->enemycounter & 0xffffff00;
                                          xp2 -= (xpos & 0xffffff00);
                                          yp2 -= (ypos & 0xffffff00);
                                          xp2 /= DEC;
                                          yp2 /= DEC;
	  	                       	xp2 -= xshift;
		                       	yp2 -= yshift;
                                          if (aptr->frame == 4) gfx_line(xp, yp, xp2, yp2, 5);
                                          else if (aptr->frame == 5) gfx_line(xp, yp, xp2, yp2, 6);
                                          else gfx_line(xp, yp, xp2, yp2, 7);
                                        }
                                        break;

                        	}
	                }
                }
                aptr--;
        }

        /* Then all the rest */

	aptr = &actor[actors-1];
        for (c = 0; c < actors; c++)
        {
        	if (aptr->type)
        	{
                        if ((aptr == &actor[0]) || (checkvision(aptr)))
                        {
       	        		int frame;
	        		int angleturn = ((aptr->angle + 64) / 128) & 7;
	                       	xp = aptr->x & 0xffffff00;
	                       	yp = aptr->y & 0xffffff00;
	                       	xp -= (xpos & 0xffffff00);
	                       	yp -= (ypos & 0xffffff00);
	                       	xp /= DEC;
	                       	yp /= DEC;
	                       	xp -= xshift;
	                       	xp -= yshift;

	        		switch (aptr->type)
	        		{
		        		case ACTOR_BOFH:
		        		frame = 0x00010001 + angleturn * 11 + aptr->frame / 32;
		        		if (aptr->attack)
		          		  frame = 0x00010009 + angleturn * 11 + aptr->attack / 3;
			                gfx_drawsprite(xp, yp, frame);
			                if ((aptr->attack) && (weapon == WEAP_CAT5))
			                {
			                	/* The Demon's Whip */
	                                        frame = 0x00020001 + angleturn * 3 + (8 - aptr->attack) / 3;
				                gfx_drawsprite(xp, yp, frame);
					}
		                        break;

		                        case ACTOR_MUZZLE:
		                        frame = 0x00020019 + angleturn;
			                gfx_drawsprite(xp, yp, frame);
			                break;

			                case ACTOR_SMOKE:
			                frame = 0x00020021 + aptr->frame / 4;
			                gfx_drawsprite(xp, yp, frame);
			                break;

			                case ACTOR_EXPLOSION:
		                        frame = 0x00030041 + aptr->frame / 5;
			                gfx_drawsprite(xp, yp, frame);
			                break;

			                case ACTOR_CHUNK:
		                        frame = 0x00030045 + aptr->frame;
			                gfx_drawsprite(xp, yp, frame);
			                break;

			                case ACTOR_SPARK:
		                        frame = 0x0003004d + aptr->frame / 4;
			                gfx_drawsprite(xp, yp, frame);
			                break;

			                case ACTOR_BLOOD:
		                        frame = 0x00030051 + aptr->frame / 4;
			                gfx_drawsprite(xp, yp, frame);
			                break;

		                        case ACTOR_FLAME:
		        		frame = SPRI_FLAME + aptr->frame / 5;
			                gfx_drawsprite(xp, yp, frame);
		        		break;

		                        case ACTOR_SHARD:
		        		frame = SPRI_SHARD + aptr->frame;
			                gfx_drawsprite(xp, yp, frame);
		        		break;

		                        case ACTOR_FLYINGGRENADE:
		        		frame = SPRI_GRENADE + aptr->frame / 10;
			                gfx_drawsprite(xp, yp, frame);
		        		break;

                                        // ***
                                        case ACTOR_BAZOOKA_PROJECTILE:
                                        frame = SPRI_BAZOOKA_PROJECTILE + angleturn;
			                gfx_drawsprite(xp, yp, frame);
                                        if (aptr->health % 2) {
                                          frame = 0x00020019 + ((4+angleturn)%8);
                                          gfx_drawsprite(xp, yp, frame);
                                        }
                                        break;


		        		case ACTOR_FISTMAN:
		        		frame = 0x00040001 + angleturn * 11 + aptr->frame / 32;
		        		if (aptr->attack)
		          		  frame = 0x00040009 + angleturn * 11 + aptr->attack / 3;
			                gfx_drawsprite(xp, yp, frame);
		                        break;

		        		case ACTOR_PISTOLMAN:
		        		frame = 0x00050001 + angleturn * 8 + aptr->frame / 32;
			                gfx_drawsprite(xp, yp, frame);
		                        break;

		        		case ACTOR_SHOTGUNMAN:
		        		frame = 0x00060001 + angleturn * 8 + aptr->frame / 32;
			                gfx_drawsprite(xp, yp, frame);
		                        break;

		        		case ACTOR_UZIMAN:
		        		frame = 0x00070001 + angleturn * 8 + aptr->frame / 32;
			                gfx_drawsprite(xp, yp, frame);
		                        break;

		        		case ACTOR_SADIST:
		        		frame = 0x000b0001 + angleturn * 11 + aptr->frame / 32;
		        		if (aptr->attack)
                                        {
                                                int attack = aptr->attack;
                                                if (attack > 5) attack = 5;
		          		  frame = 0x000b0009 + angleturn * 11 + attack / 3;
                                        }
			                gfx_drawsprite(xp, yp, frame);
		                        break;

		        		case ACTOR_TECHNICIAN:
		        		frame = 0x00080001 + angleturn * 11 + aptr->frame / 32;
		        		if (aptr->attack)
		          		  frame = 0x00080009 + angleturn * 11 + aptr->attack / 3;
			                gfx_drawsprite(xp, yp, frame);
		                        break;

		        		case ACTOR_LEADER:
		        		frame = 0x000a0001 + angleturn * 8 + aptr->frame / 32;
			                gfx_drawsprite(xp, yp, frame);
		                        break;
                        	}
	                }
                }
                aptr--;
        }
}

int finddist(int sx, int sy, int ex, int ey)
{
        int dx, dy;
        sx /= DEC;
        sy /= DEC;
        ex /= DEC;
        ey /= DEC;
	dx = ex - sx;
	dy = ey - sy;

        return squareroot(dx*dx+dy*dy);
}

int findangle(int sx, int sy, int ex, int ey)
{
	int dx, dy, adx, ady, index, angle;

        sx /= DEC;
        sy /= DEC;
        ex /= DEC;
        ey /= DEC;

	dx = ex - sx;
	dy = ey - sy;
	if ((!dx) && (!dy)) return 0;
	adx = abs(dx);
	ady = abs(dy);
	if (dx < 0)
	{
		if (dy < 0)
		{
			if (ady > adx)
			{
				index = (adx*1024)/ady;
				angle = 0x400-atantable[index];
			}
			else
			{
				index = (ady*1024)/adx;
				angle = 0x300+atantable[index];
			}
		}
		else
		{
			if (ady > adx)
			{
				index = (adx*1024)/ady;
				angle = 0x200+atantable[index];
			}
			else
			{
				index = (ady*1024)/adx;
				angle = 0x300-atantable[index];
			}
		}
	}
	else
	{
                if (dy < 0)
                {
			if (ady > adx)
			{
				index = (adx*1024)/ady;
				angle = atantable[index];
			}
			else
			{
				index = (ady*1024)/adx;
				angle = 0x100-atantable[index];
			}
		}
		else
		{
			if (ady > adx)
			{
				index = (adx*1024)/ady;
				angle = 0x200-atantable[index];
			}
			else
			{
				index = (ady*1024)/adx;
				angle = 0x100+atantable[index];
			}
		}
	}
	angle &= 0x3ff;
	return angle;
}

int angledist(int srcangle, int destangle)
{
	int dist = destangle - srcangle;
	if (dist > 512) dist = -(1024-dist);
	if (dist < -512) dist = (1024+dist);
	return dist;
}

int checkvision(ACTOR *aptr)
{
	ACTOR *pptr = &actor[0];
        int sx, sy, ex, ey, dx, dy, adx, ady, blkinf;

	adx = abs(aptr->x - pptr->x);
	ady = abs(aptr->y - pptr->y);

	if ((adx > 300*DEC) || (ady > 300*DEC)) return 0;
	sx = (pptr->x & 0xfffff000);
	sy = (pptr->y & 0xfffff000);
	ex = (aptr->x & 0xfffff000);
	ey = (aptr->y & 0xfffff000);
	dx = ex - sx;
	dy = ey - sy;
	if ((!dx) && (!dy)) return 1; /* In the same block */
	adx = abs(dx);
	ady = abs(dy);
	if (dx < 0)
	{
		sx += 15*DEC;
		ex += 15*DEC;
	}
	if (dy < 0)
	{
		sy += 15*DEC;
		ey += 15*DEC;
	}

	if (adx > ady)
	{
		dy /= (adx/(16*DEC));
		if (dx > 0) dx = 16*DEC; else dx = -16*DEC;
        	while (sx != ex)
        	{
			sx += dx;
			sy += dy;
			if (sx != ex)
			{
				blkinf = map_getblockinfo(1, sx/DEC, sy/DEC);
				if (blkinf & INF_BLOCKVISION) return 0;
			}
		}
	}
	else
	{
		dx /= (ady/(16*DEC));
		if (dy > 0) dy = 16*DEC; else dy = -16*DEC;
        	while (sy != ey)
        	{
			sx += dx;
			sy += dy;
			if (sy != ey)
			{
				blkinf = map_getblockinfo(1, sx/DEC, sy/DEC);
				if (blkinf & INF_BLOCKVISION) return 0;
			}
		}
	}
	return 1;
}

int checkblockactors(int xb, int yb)
{
        int xl = xb<<12;
        int yl = yb<<12;
        int xh = ((xb+1)<<12)-1;
        int yh = ((yb+1)<<12)-1;
        ACTOR *aptr = &actor[0];
        int c;

        /* Check for any actors in that block (for door closing) */
        for (c = 0; c < actors; c++)
        {
                if (aptr->type)
                {
                        if ((aptr->x >= xl) && (aptr->y >= yl) && (aptr->x < xh) && (aptr->y < yh))
                        {
                                return 1; /* Actor found */
                        }
                }
                aptr++;
        }
        return 0; /* Actor not found */
}


int checkwalls(ACTOR *aptr)
{
	ACTOR *pptr = &actor[0];
        int sx, sy, ex, ey, dx, dy, adx, ady, blkinf;

	adx = abs(aptr->x - pptr->x);
	ady = abs(aptr->y - pptr->y);

	if ((adx > 200*DEC) || (ady > 200*DEC)) return 0;
	sx = (pptr->x & 0xfffff000);
	sy = (pptr->y & 0xfffff000);
	ex = (aptr->x & 0xfffff000);
	ey = (aptr->y & 0xfffff000);
        sx += 8*DEC;
        sy += 8*DEC;
        ex += 8*DEC;
        ey += 8*DEC;
	dx = ex - sx;
	dy = ey - sy;
	if ((!dx) && (!dy)) return 1; /* In the same block */
	adx = abs(dx);
	ady = abs(dy);

	if (adx > ady)
	{
		dy /= (adx/(4*DEC));
		if (dx > 0) dx = 4*DEC; else dx = -4*DEC;
        	while (sx != ex)
        	{
			blkinf = map_getblockinfo(1, sx/DEC, sy/DEC);
			if (blkinf & INF_WALL) return 0;
			sx += dx;
			sy += dy;
		}
	}
	else
	{
		dx /= (ady/(4*DEC));
		if (dy > 0) dy = 4*DEC; else dy = -4*DEC;
        	while (sy != ey)
        	{
			blkinf = map_getblockinfo(1, sx/DEC, sy/DEC);
			if (blkinf & INF_WALL) return 0;
			sx += dx;
			sy += dy;
		}
	}
	return 1;
}

int checkblockvision(int ex, int ey)
{
        int sx, sy, dx, dy, adx, ady, blkinf;

        sx = actor[0].x & 0xfffff000;
        sy = actor[0].y & 0xfffff000;

	ex = ex & 0xfffff000;
	ey = ey & 0xfffff000;
	dx = ex - sx;
	dy = ey - sy;
	if ((!dx) && (!dy)) return 1; /* In the same block */
	adx = abs(dx);
	ady = abs(dy);
	if (dx < 0)
	{
		sx += 15*DEC;
		ex += 15*DEC;
	}
	if (dy < 0)
	{
		sy += 15*DEC;
		ey += 15*DEC;
	}

	if (adx > ady)
	{
		dy /= (adx/(16*DEC));
		if (dx > 0) dx = 16*DEC; else dx = -16*DEC;
        	while (sx != ex)
        	{
			sx += dx;
			sy += dy;
			if (sx != ex)
			{
				blkinf = map_getblockinfo(1, sx/DEC, sy/DEC);
				if (blkinf & INF_BLOCKVISION) return 0;
			}
		}
	}
	else
	{
		dx /= (ady/(16*DEC));
		if (dy > 0) dy = 16*DEC; else dy = -16*DEC;
        	while (sy != ey)
        	{
			sx += dx;
			sy += dy;
			if (sy != ey)
			{
				blkinf = map_getblockinfo(1, sx/DEC, sy/DEC);
				if (blkinf & INF_BLOCKVISION) return 0;
			}
		}
	}
	return 1;
}

void playercontrol(ACTOR *aptr)
{
        int want_walk = 0;      /* back<0<ahead */
        int want_turn = 0;      /* CCW<0<CW */
        int want_strafe = 0;    /* left<0<right */

        want_walk -= avgmovey * 8;
        if (win_keytable[KEY_UP]    || win_keytable[KEY_KP8] || win_keytable[KEY_KP5]) want_walk += 16;
        if (win_keytable[KEY_DOWN]  || win_keytable[KEY_KP2]) want_walk -= 16;
        if (win_keytable[KEY_LEFT]  || win_keytable[KEY_KP4]) want_turn -= 16;
        if (win_keytable[KEY_RIGHT] || win_keytable[KEY_KP6]) want_turn += 16;
        if (win_keytable[KEY_END]   || win_keytable[KEY_KP1]) want_strafe -= 16;
        if (win_keytable[KEY_PGDN]  || win_keytable[KEY_KP3]) want_strafe += 16;
        if (win_keytable[KEY_ALT]) {      /* Strafe mode */
                want_strafe += want_turn;
                want_turn = 0;
        }
        if ((win_keytable[KEY_LEFTSHIFT])||(win_keytable[KEY_RIGHTSHIFT])) {    /* Slo-Mo */
                want_walk   /= 2;
                want_strafe /= 2;
        }

        /* Don't let the player gain extra speed by moving diagonally.  */
	{
                const int maxspeed = 16;
		int speed = squareroot(want_walk   * want_walk +
                                       want_strafe * want_strafe);
                if (speed > maxspeed) {
                        want_walk   = want_walk   * maxspeed / speed;
                        want_strafe = want_strafe * maxspeed / speed;
                }
        }

        /* Penalize backward movement.
        if (want_walk < 0)
                want_walk -= want_walk / 3; */

	aptr->angularspeed = aptr->angularspeed * 9 / 10;
        aptr->angularspeed += want_turn / 8;
	if (aptr->angularspeed > 13) aptr->angularspeed = 13;
	if (aptr->angularspeed < -13) aptr->angularspeed = -13;

        if (aptr->attack)
        {
                if (weapon == WEAP_BND) {
                        want_walk   = want_walk   * 3 / 5;
                        want_strafe = want_strafe * 3 / 5;
                } else {
                        want_walk   = 0;
                        want_strafe = 0;
                }
        }
        if (!speedcheat)
        {
	        aptr->speedx += (sintable[aptr->angle    ] * want_walk   / 64);
	        aptr->speedy -= (sintable[aptr->angle+COS] * want_walk   / 64);
	        aptr->speedx += (sintable[aptr->angle+COS] * want_strafe / 64);
	        aptr->speedy += (sintable[aptr->angle    ] * want_strafe / 64);
	}
	else
        {
	        aptr->speedx += (sintable[aptr->angle    ] * want_walk   / 32);
	        aptr->speedy -= (sintable[aptr->angle+COS] * want_walk   / 32);
	        aptr->speedx += (sintable[aptr->angle+COS] * want_strafe / 32);
	        aptr->speedy += (sintable[aptr->angle    ] * want_strafe / 32);
	}

        /* Friction */
        if (aptr->speedx)
        {
	        if (aptr->speedx > 0)
	        {
			aptr->speedx -= abs(aptr->speedx/6)+1;
			if (aptr->speedx < 0) aptr->speedx = 0;
		}
		else
	        {
			aptr->speedx += abs(aptr->speedx/6)+1;
			if (aptr->speedx > 0) aptr->speedx = 0;
		}
	}
        if (aptr->speedy)
        {
	        if (aptr->speedy > 0)
	        {
			aptr->speedy -= abs(aptr->speedy/6)+1;
			if (aptr->speedy < 0) aptr->speedy = 0;
		}
		else
	        {
			aptr->speedy += abs(aptr->speedy/6)+1;
			if (aptr->speedy > 0) aptr->speedy = 0;
		}
	}

	aptr->angle += aptr->angularspeed;
	aptr->angle += avgmovex/2;
	aptr->angle &= 0x3ff;
	{
		int speed = squareroot(aptr->speedx * aptr->speedx +
                                       aptr->speedy * aptr->speedy);
		aptr->frame += speed / 40;
                aptr->frame &= 0xff;
        }
	domove(aptr);
	playerattack(aptr);
}

void playerattack(ACTOR *aptr)
{
        /* Changing weapon */
        /* Mandatory delay between attacks, depends on weapon.
         * No weapon changing during the delay *** */
        if (aptr->attackdelay) {
          aptr->attackdelay--;
        } else {
          if ((kbd_checkkey(KEY_ENTER))
              || ((mouseb & MOUSEB_RIGHT) && (!(prevmouseb & MOUSEB_RIGHT))))
          {
                  int oldweapon = weapon;
                  aptr->attack = 0;
                  do {
                          weapon++;
                          if (weapon >= WEAPNUM) weapon = WEAP_FISTS;
                  } while (!ammo[weapon]);
                  if (weapon != oldweapon) playfx(FXCHAN_FIST, SMP_FIST1, 22050, 48, 128);
          }
          /* Direct change of weapon with function keys */
          if (kbd_checkkey(KEY_F1)) weapon = WEAP_FISTS;
          if (kbd_checkkey(KEY_F2) && ammo[WEAP_CAT5]) weapon = WEAP_CAT5;
          if (kbd_checkkey(KEY_F3) && ammo[WEAP_BND]) weapon = WEAP_BND;
          if (kbd_checkkey(KEY_F4) && ammo[WEAP_PISTOL]) weapon = WEAP_PISTOL;
          if (kbd_checkkey(KEY_F5) && ammo[WEAP_SHOTGUN]) weapon = WEAP_SHOTGUN;
          if (kbd_checkkey(KEY_F6) && ammo[WEAP_UZI]) weapon = WEAP_UZI;
          if (kbd_checkkey(KEY_F7) && ammo[WEAP_GRENADE]) weapon = WEAP_GRENADE;
          if (kbd_checkkey(KEY_F8) && ammo[WEAP_BAZOOKA]) weapon = WEAP_BAZOOKA;
          if (kbd_checkkey(KEY_F9) && ammo[WEAP_CROSSBOW]) weapon = WEAP_CROSSBOW;
          if (kbd_checkkey(KEY_F10) && ammo[WEAP_SCANNER]) weapon = WEAP_SCANNER;
        }

        /* Scanner battery handling */
        if ((weapon == WEAP_SCANNER) && (!ammocheat) && (difficulty != DIFF_PRACTICE))
        {
                scannerdelay++;
                if (scannerdelay >= attackdelaytbl[weapon])
                {
                        scannerdelay = 0;
                        ammo[weapon]--;
                }
        }

        /* Shotgun load */
        if (weapon == WEAP_SHOTGUN) {
          if (aptr->attackdelay == 20) playfx(FXCHAN_GUNLOAD, SMP_GUNLOAD, 22050, 16, 128);
          else if (aptr->attackdelay == 10) spawnshotgshell(aptr);
        }

        /* Bazooka handling *** !!! */
        if (weapon == WEAP_BAZOOKA) {
          if (aptr->attackdelay == 65) {
            playpositionalfx(aptr->x, aptr->y, FXCHAN_GUNLOAD, SMP_BAZOOKALOAD, 20000, 32);
          } else if (aptr->attackdelay == 50) {
            spawnbazookaprojectile(aptr);
          } else if (aptr->attackdelay == 1) {
            ACTOR *sptr = spawnitem(aptr, ACTOR_BAZOOKA_USED);
            if (sptr) {
              sptr->frame = rand()%4;
              playpositionalfx(aptr->x, aptr->y, FXCHAN_FIST, SMP_SWISH, 22050, 64); // !!! ***
            }
          }
        }

        /* Check for initiating attack */
        if ((!aptr->attackdelay) && ((win_keytable[KEY_CTRL]) || (win_keytable[KEY_SPACE]) || (mouseb & MOUSEB_LEFT)) && ammo[weapon]) // ***
	{
		switch(weapon)
		{
                	case WEAP_FISTS:
                	case WEAP_CAT5:
			aptr->attack = 9;
                        aptr->attackdelay = attackdelaytbl[weapon];
                        playpositionalfx(aptr->x, aptr->y, FXCHAN_PLRSHOOT, SMP_SWISH, 22050, 64);
			break;

                        case WEAP_CROSSBOW:
                        aptr->attack = 5+1;
                        break;

                        case WEAP_BND:
			if ((difficulty != DIFF_PRACTICE) && (!ammocheat)) ammo[weapon]--;
                        aptr->attackdelay = attackdelaytbl[weapon];
                        aptr->attack = 5+1;
                        break;

			case WEAP_PISTOL:
                        playpositionalfx(aptr->x, aptr->y, FXCHAN_PLRSHOOT, SMP_PISTOL, 22050, 64);
                        alertenemies(aptr->x, aptr->y, SNDDIST_GUN);
   			aptr->attackdelay = attackdelaytbl[weapon];
			if ((difficulty != DIFF_PRACTICE) && (!ammocheat)) ammo[weapon]--;
                        spawnbullet(aptr, 10, 0, 1);
                        spawnshell(aptr);
                        break;

			case WEAP_SHOTGUN:
                        playpositionalfx(aptr->x, aptr->y, FXCHAN_PLRSHOOT, SMP_SHOTGUN, 22050, 64);
                        alertenemies(aptr->x, aptr->y, SNDDIST_GUN);
   			aptr->attackdelay = attackdelaytbl[weapon];
			if ((difficulty != DIFF_PRACTICE) && (!ammocheat)) ammo[weapon]--;
                        spawnbullet(aptr, 20, 0, 0);
                        spawnbullet(aptr, 20, 0, 0);
                        spawnbullet(aptr, 20, 0, 0);
                        break;

			case WEAP_UZI:
                        playpositionalfx(aptr->x, aptr->y, FXCHAN_PLRSHOOT, SMP_UZI, 22050, 64);
                        alertenemies(aptr->x, aptr->y, SNDDIST_GUN);
   			aptr->attackdelay = attackdelaytbl[weapon];
			if ((difficulty != DIFF_PRACTICE) && (!ammocheat)) ammo[weapon]--;
                        spawnbullet(aptr, 20, 1, 1);
                        spawnshell(aptr);
                        break;

                        case WEAP_GRENADE:
                        if (!aptr->attack)
                        {
                                throwstrength = DEC / 4;
                        }
                        else
                        {
                                throwstrength += DEC / 16;
                                if (throwstrength > 3*DEC) throwstrength = 3*DEC;
                        }
                        aptr->attack = 9;
                        break;

                        case WEAP_BAZOOKA: // ***
                        {
                          ACTOR *sptr = spawnitem(aptr, ACTOR_BAZOOKA_STRAP);
                          if (sptr) {
                            sptr->frame = rand()%4;
                            aptr->attackdelay = attackdelaytbl[weapon];
                            if ((difficulty != DIFF_PRACTICE) && (!ammocheat)) ammo[weapon]--;
                            playpositionalfx(aptr->x, aptr->y, FXCHAN_FIST, SMP_SWISH, 22050, 64); // !!! ***
                          }
                        }
                        break;

		}
	}
        /* Out of ammo? */
        if ((!ammo[weapon]) && (!aptr->attackdelay)) // ***
	{
                aptr->attack = 0;
                /* Find the best weapon which has ammo.  This
                   terminates because fists always have ammo.  */
                weapon = WEAP_UZI; /* Grenades & bazooka too dangerous for auto-select */
                while (!ammo[weapon])
                        --weapon;
	}
	if (aptr->attack)
	{
		aptr->attack--;
		aptr->frame = 0;
                if (aptr->attack == 4)
                {
                        if (weapon == WEAP_CROSSBOW)
                        {
               			if ((difficulty != DIFF_PRACTICE) && (!ammocheat)) ammo[weapon]--;
                                spawnarrow(aptr, 5);
                                playpositionalfx(aptr->x, aptr->y, FXCHAN_PLRSHOOT, SMP_CROSSBOW, 22050, 64);
                                aptr->attackdelay = attackdelaytbl[weapon];
                        }
                }

		if (aptr->attack == 5)
		{
			ACTOR *sptr;
			int blkinf;
                        switch(weapon)
                        {
                                case WEAP_GRENADE:
                                if (spawngrenade(aptr, 20, throwstrength))
                                {
                			if ((difficulty != DIFF_PRACTICE) && (!ammocheat)) ammo[weapon]--;
                                        playpositionalfx(aptr->x, aptr->y, FXCHAN_PLRSHOOT, SMP_SWISH, 22050, 64);
                                        aptr->attackdelay = attackdelaytbl[weapon];
                                }
                                break;

                        	case WEAP_FISTS:
				sptr = spawnactor(ACTOR_FISTHIT, aptr->x + sintable[aptr->angle]*6, aptr->y - sintable[aptr->angle+COS]*6, aptr->angle);
				if (!sptr) break;
			        sptr->origin = aptr;
                                /* Can't hit behind walls */
				blkinf = map_getblockinfo(0, sptr->x/DEC, sptr->y/DEC) | map_getblockinfo(1, sptr->x/DEC, sptr->y/DEC);
                                if (blkinf & INF_WALL)
                                {
                                        playpositionalfx(sptr->x, sptr->y, FXCHAN_FIST, SMP_FIST1+(rand()%3), 22050, 64);
                                	sptr->type = ACTOR_NONE;
                                	break;
                                }
                                break;

                                case WEAP_BND:
                                /* Make the BNDHIT's angle opposite
                                   from that of the player, so that
                                   blood spills towards him.  */
                                alertenemies(aptr->x, aptr->y, SNDDIST_DRILL);
                                sptr = spawnactor(ACTOR_BNDHIT,
                                                  aptr->x + sintable[aptr->angle]*8,
                                                  aptr->y - sintable[aptr->angle+COS]*8,
                                                  aptr->angle ^ 512);
                                if (!sptr) break;
                                sptr->origin = aptr;
                                /* Can't drill behind walls */
                                blkinf = (map_getblockinfo(0, sptr->x/DEC, sptr->y/DEC) |
                                          map_getblockinfo(1, sptr->x/DEC, sptr->y/DEC));
                                if (blkinf & INF_WALL)
                                {
                                        bnd_hittime = BND_MIN_HITTIME;
                                        switch (rand() % 10)
                                        {
                                                case 3: case 5:
                                                sptr->type = ACTOR_SMOKE;
                                                break;
                                                case 7:
                                                sptr->type = ACTOR_SPARK;
                                                break;
                                                default:
                                                sptr->type = ACTOR_NONE;
                                                break;
                                        }
                                        break;
                                }
                                break;

                        	case WEAP_CAT5:
                                {
                                        int dist;
                                        for (dist = 8; dist <= 24; dist += 8)
                                        {
                                                sptr = spawnactor(ACTOR_WHIPHIT,
                                                                  aptr->x + sintable[aptr->angle]*dist,
                                                                  aptr->y - sintable[aptr->angle+COS]*dist,
                                                                  aptr->angle);
                                                if (!sptr) break;
                                                sptr->origin = aptr;
                                                /* Can't whip behind walls */
                                                blkinf = map_getblockinfo(0, sptr->x/DEC, sptr->y/DEC) | map_getblockinfo(1, sptr->x/DEC, sptr->y/DEC);
                                                if (blkinf & INF_WALL)
                                                {
                                                        playpositionalfx(sptr->x, sptr->y, FXCHAN_FIST, SMP_FIST1+(rand()%3), 22050, 64);
                                                        sptr->type = ACTOR_NONE;
                                                        break;
                                                }
                                        }
                                }
                                break;
			}
		}
	}
        if (bnd_hittime > 0) {
                --bnd_hittime;
                ensure_bnd_sound (bnd_hittime ? BNDS_HIT : BNDS_RUN);
        } else if (weapon == WEAP_BND && aptr->attack)
                ensure_bnd_sound (BNDS_ON);
        else
                ensure_bnd_sound (BNDS_OFF);
}


static void ensure_bnd_sound (enum bnd_sound bnds)
{
        int freq = 22050;
        static const int samplenums[] = {
                SMP_BND_OFF,
                SMP_BND_ON,
                SMP_BND_HIT,
                SMP_BND_RUN
        };
        if (ammo[WEAP_BND] < 256) {
          freq = 22050-16*(256-ammo[WEAP_BND]);
          if (snd_sndinitted) snd_channel[FXCHAN_BND].freq = freq;
        }
        if (bnds == bnd_sound)
                return;
        if (bnds == BNDS_ON && bnd_sound == BNDS_RUN)
                return;
        bnd_sound = bnds;
        playfx (FXCHAN_BND, samplenums[bnds], freq, 80, 128);
}

void breakglass(ACTOR *aptr, int x, int y)
{
        int d;
	int blknum = map_getblocknum(1, x/DEC, y/DEC);
        short *mapptr;
	int xb = x >> 12;
	int yb = y >> 12;
	if (blknum < 421)
	{
		playpositionalfx(x, y, FXCHAN_GLASS1+(rand()&1), SMP_GLASS, 22050, 64);
		alertenemies(x, y, SNDDIST_GLASS);
		mapptr = &map_layerdataptr[1][map_layer[1].xsize*yb+xb];
		*mapptr += 420;
		for (d = 0; d < NUM_SHARDS; d++)
		{
			spawnshard(aptr);
		}
        }
}

void structuredamage(int x, int y)
{
	int blknum;
        short *mapptr;
	int xb = x >> 12;
	int yb = y >> 12;
	blknum = map_getblocknum(0, x/DEC, y/DEC);
	if ((blknum) && (blknum < firstdamagedblock))
	{
		mapptr = &map_layerdataptr[0][map_layer[0].xsize*yb+xb];
		*mapptr += firstdamagedblock-1;
        }
	blknum = map_getblocknum(1, x/DEC, y/DEC);
	if ((blknum) && (blknum < firstdamagedblock))
	{
		mapptr = &map_layerdataptr[1][map_layer[1].xsize*yb+xb];
		*mapptr += firstdamagedblock-1;
        }
}



void spawnchunk(ACTOR *aptr, int damage)
{
	ACTOR *sptr;
	int speed;
	sptr = spawnactor(ACTOR_CHUNK, aptr->x, aptr->y, (rand() & 0x3ff));
	if (!sptr) return;
 	speed = (rand() % 3) + 3;
 	sptr->origin = aptr->origin;
	sptr->speedx = (sintable[sptr->angle]*speed);
	sptr->speedy = -(sintable[sptr->angle+COS]*speed);
	sptr->frame = rand() & 7;
	sptr->health = (rand() % 40) + 30;
	sptr->special = damage;
}

void spawnshard(ACTOR *aptr)
{
	ACTOR *sptr;
	int speed;
	int angle = (aptr->angle + (rand()&255) - 128) & 0x3ff;
	sptr = spawnactor(ACTOR_SHARD, aptr->x+((rand()&7)-4)*DEC, aptr->y+((rand()&7)-4)*DEC, angle);
	if (!sptr) return;
 	speed = (rand() % 1) + 2;
 	sptr->origin = aptr;
	sptr->speedx = (sintable[sptr->angle]*speed);
	sptr->speedy = -(sintable[sptr->angle+COS]*speed);
	sptr->frame = rand() & 7;
	sptr->health = (rand() % 40) + 10;
}

void spawnshell(ACTOR *aptr) // ***
{
  ACTOR *sptr;
  int speed;
  int angle = ((rand() % 100) + 0x100 - 50 + aptr->angle)& 0x3ff;
  sptr = spawnactor(ACTOR_SHELL,
                    aptr->x + sintable[(aptr->angle+40) & 0x3ff]*6,
                    aptr->y + -sintable[((aptr->angle+40) & 0x3ff)+COS]*6,
                    0);
  if (!sptr) return;
  speed = (rand() % 40) + 20;
  sptr->origin = aptr;
  sptr->speedx = (sintable[angle]*speed)/20 + aptr->speedx;
  sptr->speedy = -(sintable[angle+COS]*speed)/20 + aptr->speedy;
  sptr->frame = rand() % 4;
  sptr->health = 70;
  sptr->special = 5+(rand()%20);
}

void spawnshotgshell(ACTOR *aptr) // ***
{
  ACTOR *sptr;
  int speed;
  int angle = ((rand() % 100) + 0x100 - 50 + aptr->angle)& 0x3ff;
  sptr = spawnactor(ACTOR_SHOTGSHELL,
                    aptr->x + sintable[(aptr->angle+40) & 0x3ff]*6,
                    aptr->y + -sintable[((aptr->angle+40) & 0x3ff)+COS]*6,
                    0);
  if (!sptr) return;
  speed = (rand() % 40) + 20;
  sptr->origin = aptr;
  sptr->speedx = (sintable[angle]*speed)/20 + aptr->speedx;
  sptr->speedy = -(sintable[angle+COS]*speed)/20 + aptr->speedy;
  sptr->frame = rand() % 12;
  sptr->health = 70;
  sptr->special = 2+(rand()%10);
}

// ***
void spawnbazookaprojectile(ACTOR *aptr)
{
  ACTOR *sptr;
  int x, y, blkinf;
  x = aptr->x + sintable[(aptr->angle+0x100)&0x3ff]*3;
  y = aptr->y - sintable[((aptr->angle+0x100)&0x3ff)+COS]*3;
  blkinf = map_getblockinfo(0, x/DEC, y/DEC) | map_getblockinfo(1, x/DEC, y/DEC);
  if (blkinf & INF_WALL) {
    // Shoot left-handed instead?
    x = aptr->x + sintable[(aptr->angle-0x100)&0x3ff]*3;
    y = aptr->y - sintable[((aptr->angle-0x100)&0x3ff)+COS]*3;
    blkinf = map_getblockinfo(0, x/DEC, y/DEC) | map_getblockinfo(1, x/DEC, y/DEC);
    if (blkinf & INF_WALL) return; // Don't fire...
  }
  sptr = spawnactor(ACTOR_BAZOOKA_PROJECTILE, x, y, aptr->angle);
  if (!sptr) return;
  sptr->origin = aptr;
  sptr->speedx = aptr->speedx;
  sptr->speedy = aptr->speedy;
  sptr->health = 300;
  sptr->special = 0;
  sptr->angularspeed = 0;
  playownedpositionalfx(sptr, sptr->x, sptr->y, FXCHAN_PLRSHOOT, SMP_BAZOOKAFIRE, 22050, 64);
  alertenemies(sptr->x, sptr->y, SNDDIST_GUN);

  // Some shit that bursts out from the rear..
  {
    int count;
    for (count = 0; count < 10; count++) {
      ACTOR *rptr;
      rptr = spawnactor(ACTOR_RICOCHET, x, y, 0);
      if (rptr) {
        int speed = rand()%300+100;
        int variance = rand()%0x80;
        variance *= variance;
        variance /= 0x100;
        if (rand()%2) variance = -variance;
        rptr->frame = rand()%8;
        rptr->special = 0;
        rptr->angle = (aptr->angle+0x200+variance)&0x3ff;
        rptr->speedx = (sintable[rptr->angle]*speed/256);
        rptr->speedy = -(sintable[rptr->angle+COS]*speed/256);
        rptr->health = rand()%40+20;
        rptr->enemycontrol = aptr->x;  // Position buffer with weird
        rptr->enemycounter = aptr->y;  // slot names!
      }
    }
  }
}

// ***
void movebazookaprojectile(ACTOR *aptr)
{
        ACTOR *cptr;
        int blkinf;

        updateownedpositionalfx(aptr, aptr->x, aptr->y, FXCHAN_PLRSHOOT);

        /* Collision */
        cptr = collision_no_origin(aptr);
        if (cptr)
        {
                // EXPLODE!
                killownedpositionalfx(aptr, FXCHAN_PLRSHOOT);
                grenadeexplode(aptr, 1);
                return;
        }

        if (!(aptr->health % 4)) {
          // Smoke
          ACTOR *rptr;
          spawnactor(ACTOR_SMOKE, aptr->x-sintable[aptr->angle]*10, aptr->y+sintable[aptr->angle+COS]*10, 0);
          // Some shit that bursts out from the rear..
          rptr = spawnactor(ACTOR_RICOCHET, aptr->x, aptr->y, 0);
          if (rptr) {
            int speed = rand()%300+100;
            int variance = rand()%0x80;
            variance *= variance;
            variance /= 0x100;
            if (rand()%2) variance = -variance;
            rptr->frame = rand()%8;
            rptr->special = 0;
            rptr->angle = (aptr->angle+0x200+variance)&0x3ff;
            rptr->speedx = (sintable[rptr->angle]*speed/256);
            rptr->speedy = -(sintable[rptr->angle+COS]*speed/256);
            rptr->health = rand()%40+20;
          }
        }

        blkinf = map_getblockinfo(0, (aptr->x+aptr->speedx)/DEC, (aptr->y+aptr->speedy)/DEC) | map_getblockinfo(1, (aptr->x+aptr->speedx)/DEC, (aptr->y+aptr->speedy)/DEC);
        if (blkinf & INF_GLASS)
        {
          // Cripple and break glass!
          aptr->special = 1;
          aptr->x += aptr->speedx;
          aptr->y += aptr->speedy;
          aptr->speedx /= 2; // Slow down a bit and change direction
          aptr->speedy /= 2;
          aptr->angle += (rand() % 0x80) - 0x40;
          aptr->angle &= 0x3ff;
          breakglass(aptr, aptr->x+aptr->speedx, aptr->y+aptr->speedy);
        } else {
          int xcollide = 0, ycollide = 0, newx, newy, newspeedx = aptr->speedx, newspeedy = aptr->speedy, impact;
          if (aptr->speedx)
          {
                  newx = aptr->x + aptr->speedx;
                  blkinf = map_getblockinfo(0, newx/DEC, aptr->y/DEC) | map_getblockinfo(1, newx/DEC, aptr->y/DEC);
                  if (blkinf & INF_WALL) xcollide = 1;
          }
          if (aptr->speedy)
          {
                  newy = aptr->y + aptr->speedy;
                  blkinf = map_getblockinfo(0, aptr->x/DEC, newy/DEC) | map_getblockinfo(1, aptr->x/DEC, newy/DEC);
                  if (blkinf & INF_WALL) ycollide = 1;
          }
          if (xcollide)
          {
                  newspeedx = -aptr->speedx / 8;
                  aptr->angle = (-aptr->angle+(rand()%0x80)-0x40)&0x3ff;
                  playpositionalfx(aptr->x, aptr->y, FXCHAN_KLONK, SMP_FIST1, 21000+(rand()%2000),30+abs(aptr->speedx/DEC)*10);
          }
          else aptr->x += aptr->speedx;
          if (ycollide)
          {
                  newspeedy = -aptr->speedy / 8;
                  aptr->angle = (0x200-aptr->angle+(rand()%0x80)-0x40)&0x3ff;
                  playpositionalfx(aptr->x, aptr->y, FXCHAN_KLONK, SMP_FIST1, 21000+(rand()%2000),30+abs(aptr->speedy/DEC)*10);
          }
          else aptr->y += aptr->speedy;
          if (ycollide || xcollide) {
            aptr->health -= 45;
            impact = (aptr->speedx-newspeedx)*(aptr->speedx-newspeedx)+
                     (aptr->speedy-newspeedy)*(aptr->speedy-newspeedy);
            if ((aptr->health <= 0) || (impact > 300000)) {
              // EXPLODE!
              traceoutofwall(aptr);
              killownedpositionalfx(aptr, FXCHAN_PLRSHOOT);
              grenadeexplode(aptr, 1);
              return;
            } else if (impact > 200000) {
              // Cripple!
              aptr->special = 1;
            }
            aptr->speedx = newspeedx;
            aptr->speedy = newspeedy;
          }
        }
        aptr->speedx += (sintable[aptr->angle])/8;
        aptr->speedy += -(sintable[aptr->angle+COS])/8;
        aptr->speedx *= 0.99;
        aptr->speedy *= 0.99;
        aptr->angle += aptr->angularspeed;
        aptr->angle &= 0x3ff;
        if (aptr->special) aptr->angularspeed += (rand()%3)-1;
        if (aptr->angularspeed < -0x10) aptr->angularspeed = -0x10;
        if (aptr->angularspeed > 0x10) aptr->angularspeed = 0x10;
        aptr->health--;
        if (aptr->health == 0) {
          // EXPLODE!
          traceoutofwall(aptr);
          killownedpositionalfx(aptr, FXCHAN_PLRSHOOT);
          grenadeexplode(aptr, 1);
          return;
        }
}


void spawnbullet(ACTOR *aptr, int inaccuracy, int illuminated, int mayricoche) // ***
{
	ACTOR *sptr;
	spawnactor(ACTOR_MUZZLE, aptr->x + sintable[aptr->angle]*4, aptr->y - sintable[aptr->angle+COS]*4, aptr->angle);
	sptr = spawnactor(ACTOR_BULLET, aptr->x + sintable[aptr->angle]*3, aptr->y - sintable[aptr->angle+COS]*3, aptr->angle);
        if (!sptr) return;
        sptr->origin = aptr;
        sptr->angle -= inaccuracy/2;
        sptr->angle += (rand() % inaccuracy);
        sptr->angle &= 0x3ff;
        sptr->speedx = sintable[sptr->angle]*8;
        sptr->speedy = -sintable[sptr->angle+COS]*8;
        sptr->special = illuminated;   // ***
        sptr->enemymode = mayricoche;  //
        sptr->enemycontrol = sptr->x;  // Position buffer with weird
        sptr->enemycounter = sptr->y;  // slot names!
}

void spawnarrow(ACTOR *aptr, int inaccuracy)
{
	ACTOR *sptr;
	sptr = spawnactor(ACTOR_ARROW, aptr->x + sintable[aptr->angle]*5 +
	  sintable[aptr->angle+COS]*3, aptr->y - sintable[aptr->angle+COS]*5 + sintable[aptr->angle]*3, aptr->angle);
        if (!sptr) return;
        sptr->origin = aptr;
        sptr->angle -= inaccuracy/2;
        sptr->angle += (rand() % inaccuracy);
        sptr->angle &= 0x3ff;
        sptr->speedx = sintable[sptr->angle]*5;
        sptr->speedy = -sintable[sptr->angle+COS]*5;
        sptr->special = 1;
        sptr->enemymode = 1;
        sptr->enemycontrol = sptr->x;  // Position buffer with weird
        sptr->enemycounter = sptr->y;  // slot names!
}

int spawngrenade(ACTOR *aptr, int inaccuracy, int speed)
{
	ACTOR *sptr;
        int blkinf;

	blkinf = map_getblockinfo(0, aptr->x,aptr->y)||
                map_getblockinfo(1, aptr->x,aptr->y);
        if (blkinf & INF_WALL) return 0;
	sptr = spawnactor(ACTOR_FLYINGGRENADE, aptr->x, aptr->y, aptr->angle);
        if (!sptr) return 0;
        sptr->origin = aptr;
        sptr->angle -= inaccuracy/2;
        sptr->angle += (rand() % inaccuracy);
        sptr->angle &= 0x3ff;
        sptr->speedx = (sintable[sptr->angle]*speed)/DEC;
        sptr->speedy = (-sintable[sptr->angle+COS]*speed)/DEC;
        sptr->health = 140; /* Time until it explodes */
        return 1;
}

void spawnflame(ACTOR *aptr, int inaccuracy)
{
	ACTOR *sptr;
	sptr = spawnactor(ACTOR_FLAME, aptr->x + sintable[aptr->angle]*6, aptr->y - sintable[aptr->angle+COS]*6, aptr->angle);
        if (!sptr) return;
        sptr->origin = aptr;
        sptr->angle -= inaccuracy/2;
        sptr->angle += (rand() % inaccuracy);
        sptr->angle &= 0x3ff;
        sptr->speedx = (sintable[sptr->angle]*5)/2;
        sptr->speedy = (-sintable[sptr->angle+COS]*5)/2;
        sptr->health = DMG_FLAME;
}

void moveblood(ACTOR *aptr)
{
       	aptr->frame++;
       	if (aptr->frame > 16)
       	{
       		aptr->frame = 16;
	}
       	aptr->x += aptr->speedx;
       	aptr->y += aptr->speedy;
	aptr->health--;
	if (aptr->health <= 0) aptr->type = ACTOR_NONE;
}

void shellsound(ACTOR *aptr) {
  int channel = rand() % 3;
  switch (channel) {
    case 0 :
      playpositionalfx(aptr->x, aptr->y, FXCHAN_KLONK, SMP_SHELL1, 18000+(rand()%5000),30+abs(aptr->speedx/DEC)*40);
      break;
    case 1 :
      playpositionalfx(aptr->x, aptr->y, FXCHAN_MULTI1, SMP_SHELL1, 18000+(rand()%5000),30+abs(aptr->speedx/DEC)*40);
      break;
    case 2 :
      playpositionalfx(aptr->x, aptr->y, FXCHAN_MULTI2, SMP_SHELL2, 18000+(rand()%5000),30+abs(aptr->speedx/DEC)*40);
      break;
  }
}

void moveshell(ACTOR *aptr)
{
        int oldx = aptr->x, oldy = aptr->y;
        int newx, newy;
        int blkinf;
        int maxframes = 0;
        int bounceblkinf;

        // ***
        if (aptr->special) {
          bounceblkinf = INF_WALL;
        } else {
          bounceblkinf = INF_OBSTACLE | INF_WALL;
        }

        blkinf = map_getblockinfo(0, aptr->x/DEC, aptr->y/DEC) | map_getblockinfo(1, aptr->x/DEC, aptr->y/DEC);
        // ***
        if (blkinf & bounceblkinf) {
          if (blkinf & INF_WALL) {
            traceoutofwall(aptr);
            blkinf = map_getblockinfo(0, aptr->x/DEC, aptr->y/DEC) | map_getblockinfo(1, aptr->x/DEC, aptr->y/DEC);
            if (blkinf & INF_WALL) {
              shellsound(aptr);
              aptr->type = ACTOR_NONE;
              return;
            }
          }
          bounceblkinf = INF_WALL;
        }

        newx = aptr->x + aptr->speedx;
        blkinf = map_getblockinfo(0, newx/DEC, aptr->y/DEC) | map_getblockinfo(1, newx/DEC, aptr->y/DEC);
        if (blkinf & bounceblkinf) // ***
        {
        	aptr->speedx = -aptr->speedx / 2;
                playpositionalfx(aptr->x, aptr->y, FXCHAN_KLONK, SMP_KLONK, 21000+(rand()%2000),8+abs(aptr->speedx/DEC)*6);
        }
        else aptr->x = newx;

        newy = aptr->y + aptr->speedy;
        blkinf = map_getblockinfo(0, aptr->x/DEC, newy/DEC) | map_getblockinfo(1, aptr->x/DEC, newy/DEC);
        if (blkinf & bounceblkinf)
        {
        	aptr->speedy = -aptr->speedy / 2;
                playpositionalfx(aptr->x, aptr->y, FXCHAN_KLONK, SMP_KLONK, 21000+(rand()%2000),8+abs(aptr->speedx/DEC)*6);
        }
        else aptr->y = newy;

        if (aptr->type == ACTOR_SHELL) {
          aptr->speedx -= aptr->speedx >> 4;
          if (!(aptr->speedx >> 4)) aptr->speedx = 0;
          aptr->speedy -= aptr->speedy >> 4;
          if (!(aptr->speedy >> 4)) aptr->speedy = 0;
          maxframes = 4;
        } else if (aptr->type == ACTOR_SHOTGSHELL) {
          aptr->speedx -= aptr->speedx >> 3;
          if (!(aptr->speedx >> 3)) aptr->speedx = 0;
          aptr->speedy -= aptr->speedy >> 3;
          if (!(aptr->speedy >> 3)) aptr->speedy = 0;
          maxframes = 12;
        }
        if ((oldx/DEC != newx/DEC) || (oldy/DEC != newy/DEC)) {
          if (aptr->frame == 0) aptr->frame = maxframes-1;
          else aptr->frame--;
        }

        // ***
        if (aptr->special == 1) shellsound(aptr);
        if (aptr->special) aptr->special--;

        aptr->health--;
        // Desaparece el casquillo //if (aptr->health <= 0) aptr->type = ACTOR_NONE;
}

void movericochet(ACTOR *aptr)
{
        int newx, newy;
        int blkinf;

        updateownedpositionalfx(aptr, aptr->x, aptr->y, FXCHAN_RICOCHET); // ***

        aptr->enemycontrol = aptr->x;  // Position buffer with weird
        aptr->enemycounter = aptr->y;  // slot names!

        newx = aptr->x + aptr->speedx;
        blkinf = map_getblockinfo(0, newx/DEC, aptr->y/DEC) | map_getblockinfo(1, newx/DEC, aptr->y/DEC);
        if (blkinf & INF_WALL)
        {
        	aptr->speedx = -aptr->speedx / 2;
                if (aptr->health < 10) playpositionalfx(aptr->x, aptr->y, FXCHAN_RICOCHET, SMP_KLONK, 21000+(rand()%2000),8+abs(aptr->speedx/DEC)*6);
        }
        else aptr->x = newx;

        newy = aptr->y + aptr->speedy;
        blkinf = map_getblockinfo(0, aptr->x/DEC, newy/DEC) | map_getblockinfo(1, aptr->x/DEC, newy/DEC);
        if (blkinf & INF_WALL)
        {
        	aptr->speedy = -aptr->speedy / 2;
                if (aptr->health < 10) playpositionalfx(aptr->x, aptr->y, FXCHAN_RICOCHET, SMP_KLONK, 21000+(rand()%2000),8+abs(aptr->speedx/DEC)*6);
        }
        else aptr->y = newy;

        aptr->frame++;
        aptr->frame %= 8;

        aptr->health--;
        if (aptr->health <= 0) aptr->type = ACTOR_NONE;
}




void movechunk(ACTOR *aptr)
{
        ACTOR *cptr;
        int newx, newy;
        int blkinf;

        newx = aptr->x + aptr->speedx;
        blkinf = map_getblockinfo(0, newx/DEC, aptr->y/DEC) | map_getblockinfo(1, newx/DEC, aptr->y/DEC);
        /* High-energy chunks can break glass */
        if ((blkinf & INF_GLASS) && (aptr->special > DMG_CHUNK))
        {
               	breakglass(aptr, newx, aptr->y);
        }
        /* Very high-energy chunks can cause structure damage */
        if (aptr->special >= DMG_BOMBCHUNK)
        {
        	if (aptr->health >= 50)
        	{
        	 	if ((rand()%25) < (aptr->health-50))
        	 	{
        	 		structuredamage(newx, aptr->y);
        	 	}
        	}
        }
        if (blkinf & INF_WALL)
        {
        	aptr->speedx = -aptr->speedx / 2;
        	playpositionalfx(aptr->x, aptr->y, FXCHAN_KLONK, SMP_KLONK, 21000+(rand()%2000),8+abs(aptr->speedx/DEC)*6);
        }
        else aptr->x = newx;

        newy = aptr->y + aptr->speedy;
        blkinf = map_getblockinfo(0, aptr->x/DEC, newy/DEC) | map_getblockinfo(1, aptr->x/DEC, newy/DEC);
        /* High-energy chunks can break glass */
        if ((blkinf & INF_GLASS) && (aptr->special > DMG_CHUNK))
        {
               	breakglass(aptr, aptr->x, newy);
        }
        /* Very high-energy chunks can cause structure damage */
        if (aptr->special >= DMG_BOMBCHUNK)
        {
        	if (aptr->health >= 50)
        	{
        	 	if ((rand()%25) < (aptr->health-50))
        	 	{
        	 		structuredamage(aptr->x, newy);
        	 	}
        	}
        }
        if (blkinf & INF_WALL)
        {
        	aptr->speedy = -aptr->speedy / 2;
        	playpositionalfx(aptr->x, aptr->y, FXCHAN_KLONK, SMP_KLONK, 21000+(rand()%2000),8+abs(aptr->speedx/DEC)*6);
        }
        else aptr->y = newy;

        /* Chunk collision */
        cptr = collision(aptr, aptr->x, aptr->y, NULL);
        if (cptr)
        {
		punish(cptr, aptr, aptr->special);
		aptr->type = ACTOR_NONE;
	        return;
        }

        if ((rand() % 250) < aptr->health)
        {
        	spawnactor(ACTOR_SPARK, aptr->x, aptr->y, 0);
        }
        aptr->health--;
        if (aptr->health <= 0) aptr->type = ACTOR_NONE;
}

void moveshard(ACTOR *aptr)
{
        ACTOR *cptr;
        int newx, newy;
        int blkinf;

        newx = aptr->x + aptr->speedx;
        blkinf = map_getblockinfo(0, newx/DEC, aptr->y/DEC) | map_getblockinfo(1, newx/DEC, aptr->y/DEC);
        if (blkinf & INF_WALL)
        {
        	aptr->speedx = -aptr->speedx / 2;
        	playpositionalfx(aptr->x, aptr->y, FXCHAN_KLONK, SMP_KLONK, 21000+(rand()%2000),8+abs(aptr->speedx/DEC)*6);
        }
        else aptr->x = newx;

        newy = aptr->y + aptr->speedy;
        blkinf = map_getblockinfo(0, aptr->x/DEC, newy/DEC) | map_getblockinfo(1, aptr->x/DEC, newy/DEC);
        if (blkinf & INF_WALL)
        {
        	aptr->speedy = -aptr->speedy / 2;
        	playpositionalfx(aptr->x, aptr->y, FXCHAN_KLONK, SMP_KLONK, 21000+(rand()%2000),8+abs(aptr->speedx/DEC)*6);
        }
        else aptr->y = newy;

        /* Shard collision */
        cptr = collision(aptr, aptr->x, aptr->y, NULL);
        if (cptr)
        {
	        punish(cptr, aptr, DMG_SHARD);
	        aptr->type = ACTOR_NONE;
	        return;
        }
        aptr->health--;
        if (aptr->health <= 0) aptr->type = ACTOR_NONE;
}

void moveitem(ACTOR *aptr)
{
        int newx, newy;
        int blkinf;
        int radius;

        if ((aptr->speedx) || (aptr->speedy))
        {
                /* Item movement */
	        if (aptr->speedx)
	        {
		        newx = aptr->x + aptr->speedx;
		        blkinf = map_getblockinfo(0, newx/DEC, aptr->y/DEC) | map_getblockinfo(1, newx/DEC, aptr->y/DEC);
		        if (blkinf & (INF_WALL | INF_OBSTACLE))
		        {
		        	aptr->speedx = -aptr->speedx / 2;
		        	playpositionalfx(aptr->x, aptr->y, FXCHAN_KLONK, SMP_KLONK, 21000+(rand()%2000),8+abs(aptr->speedx/DEC)*6);
		        }
	        	else aptr->x = newx;
	        }
	        if (aptr->speedy)
	        {
		        newy = aptr->y + aptr->speedy;
		        blkinf = map_getblockinfo(0, aptr->x/DEC, newy/DEC) | map_getblockinfo(1, aptr->x/DEC, newy/DEC);
		        if (blkinf & (INF_WALL | INF_OBSTACLE))
		        {
		        	aptr->speedy = -aptr->speedy / 2;
		        	playpositionalfx(aptr->x, aptr->y, FXCHAN_KLONK, SMP_KLONK, 21000+(rand()%2000),8+abs(aptr->speedx/DEC)*6);
		        }
		        else aptr->y = newy;
		}

		checkstairs(aptr);

	        /* Friction */
	        if (aptr->speedx)
	        {
		        if (aptr->speedx > 0)
		        {
				aptr->speedx -= abs(aptr->speedx/10)+1;
				if (aptr->speedx < 0) aptr->speedx = 0;
			}
			else
		        {
				aptr->speedx += abs(aptr->speedx/10)+1;
				if (aptr->speedx > 0) aptr->speedx = 0;
			}
		}
	        if (aptr->speedy)
	        {
		        if (aptr->speedy > 0)
		        {
				aptr->speedy -= abs(aptr->speedy/10)+1;
				if (aptr->speedy < 0) aptr->speedy = 0;
			}
			else
		        {
				aptr->speedy += abs(aptr->speedy/10)+1;
				if (aptr->speedy > 0) aptr->speedy = 0;
			}
		}
        }
        else
        {
		/* Item stationary: check collision */
	  	radius = actorradius[actor[0].type];
	  	if (radius)
	  	{
	                int taken = 0;
	  		int xdist = abs((aptr->x - actor[0].x));
	  		int ydist = abs((aptr->y - actor[0].y));
	  		if ((xdist <= radius) && (ydist <= radius))
	  		{
	  			switch(aptr->type)
	  			{
	  				case ACTOR_CAT5:
                                        taken = takes_weapon(WEAP_CAT5, 999);
                                        break;

                                        case ACTOR_BND:
                                        taken = takes_weapon(WEAP_BND, 999);
                                        break;

                                        case ACTOR_CROSSBOW:
                                        taken = takes_weapon(WEAP_CROSSBOW, 10);
                                        break;

                                        case ACTOR_SCANNER:
                                        taken = takes_weapon(WEAP_SCANNER, 999);
                                        break;

	  				case ACTOR_PISTOL:
                                        taken = takes_weapon(WEAP_PISTOL, 20);
                                        break;

	  				case ACTOR_SHOTGUN:
                                        taken = takes_weapon(WEAP_SHOTGUN, 10);
                                        break;

	  				case ACTOR_UZI:
                                        taken = takes_weapon(WEAP_UZI, 30);
                                        break;

                                        case ACTOR_GRENADE:
                                        taken = takes_weapon(WEAP_GRENADE, aptr->health);
                                        break;

                                        // ***
                                        case ACTOR_BAZOOKA:
                                        taken = takes_weapon(WEAP_BAZOOKA, 1);
                                        break;


	  				case ACTOR_SMALLMEDIKIT:
	  				if (actor[0].health < 100)
	  				{
		  				actor[0].health += 10;
		  				if (actor[0].health > 100) actor[0].health = 100;
		  				taken = 1;
		  			}
	  				break;

	  				case ACTOR_BIGMEDIKIT:
	  				if (actor[0].health < 100)
	  				{
		  				actor[0].health += 20;
		  				if (actor[0].health > 100) actor[0].health = 100;
		  				taken = 1;
		  			}
		  			break;

		  			case ACTOR_INSTRUCTIONS:
                                        bomb[aptr->health].instructions = 1;
                                        showinstr = aptr->health;
                                        showinstrtime = 200;
                                        reward(1000);
                                        taken = 1;
                                        break;
	  			}
	                        if (taken)
	                        {
		  			aptr->type = ACTOR_NONE;
			        	playfx(FXCHAN_FIST, SMP_FIST1, 16000,64,128);
			        }
	                }
	        }
	}
}

void movegrenade(ACTOR *aptr)
{
        int newx, newy;
        int blkinf;
        int xcollide = 0;
        int ycollide = 0;

        /* If grenade is somehow trapped, it explodes */
        blkinf = map_getblockinfo(0, aptr->x/DEC, aptr->y/DEC) | map_getblockinfo(1, aptr->x/DEC, aptr->y/DEC);
        if (blkinf & INF_WALL) aptr->health = 0;

        /* Collision to walls (always) */
        if (aptr->speedx)
        {
                newx = aptr->x + aptr->speedx;
                blkinf = map_getblockinfo(0, newx/DEC, aptr->y/DEC) | map_getblockinfo(1, newx/DEC, aptr->y/DEC);
                if (blkinf & INF_WALL) xcollide = 1;
        }
        if (aptr->speedy)
        {
                newy = aptr->y + aptr->speedy;
                blkinf = map_getblockinfo(0, aptr->x/DEC, newy/DEC) | map_getblockinfo(1, aptr->x/DEC, newy/DEC);
                if (blkinf & INF_WALL) ycollide = 1;
        }

        /* Collision to obstacles */
        if (aptr->frame >= 40)
        {
                /* If on top of an obstacle, ignore collision */
                blkinf = map_getblockinfo(0, aptr->x/DEC, aptr->y/DEC) | map_getblockinfo(1, aptr->x/DEC, aptr->y/DEC);
                if (!(blkinf & INF_OBSTACLE))
                {
                        if (aptr->speedx)
                        {
                                newx = aptr->x + aptr->speedx;
                                blkinf = map_getblockinfo(0, newx/DEC, aptr->y/DEC) | map_getblockinfo(1, newx/DEC, aptr->y/DEC);
                                if (blkinf & INF_OBSTACLE) xcollide = 1;
                        }
                        if (aptr->speedy)
                        {
                                newy = aptr->y + aptr->speedy;
                                blkinf = map_getblockinfo(0, aptr->x/DEC, newy/DEC) | map_getblockinfo(1, aptr->x/DEC, newy/DEC);
                                if (blkinf & INF_OBSTACLE) ycollide = 1;
                        }
                }
        }
        if (xcollide)
        {
               	aptr->speedx = -aptr->speedx / 2;
                playpositionalfx(aptr->x, aptr->y, FXCHAN_KLONK, SMP_FIST1, 21000+(rand()%2000),30+abs(aptr->speedx/DEC)*10);
        }
       	else aptr->x += aptr->speedx;
        if (ycollide)
        {
               	aptr->speedy = -aptr->speedy / 2;
                playpositionalfx(aptr->x, aptr->y, FXCHAN_KLONK, SMP_FIST1, 21000+(rand()%2000),30+abs(aptr->speedy/DEC)*10);
        }
       	else aptr->y += aptr->speedy;


        /* Friction */
        if (aptr->frame >= 40)
        {
                if (aptr->speedx)
                {
        	        if (aptr->speedx > 0)
        	        {
        			aptr->speedx -= abs(aptr->speedx/30)+1;
        			if (aptr->speedx < 0) aptr->speedx = 0;
        		}
        		else
        	        {
        			aptr->speedx += abs(aptr->speedx/30)+1;
        			if (aptr->speedx > 0) aptr->speedx = 0;
        		}
        	}
                if (aptr->speedy)
                {
        	        if (aptr->speedy > 0)
        	        {
        			aptr->speedy -= abs(aptr->speedy/30)+1;
        			if (aptr->speedy < 0) aptr->speedy = 0;
        		}
        		else
        	        {
        			aptr->speedy += abs(aptr->speedy/30)+1;
        			if (aptr->speedy > 0) aptr->speedy = 0;
        		}
        	}
        }
        else
       /* Flying animation */
       aptr->frame++;
       /* Explosion */
       if (aptr->health) aptr->health--;
       else
       {
         // ***
         grenadeexplode(aptr, 0); // ***
       }
}

// ***
void grenadeexplode(ACTOR *aptr, int extraflavor)
{
  /* *** START copied straight from Cadaver's sources END *** */
  int c;
  int pieces = NUM_GRENADEPIECES;
  playpositionalfx(aptr->x, aptr->y, FXCHAN_EXPLODE, SMP_EXPLODE, 20000+(rand()&2047),64);
  alertenemies(aptr->x, aptr->y, SNDDIST_EXPLOSION);
  aptr->type = ACTOR_EXPLOSION;
  aptr->frame = 0;

  /* If there's a door, open it */
  opendoor(aptr->x, aptr->y);
  /* Open also doors next to it */
  opendoor(aptr->x+16*DEC, aptr->y);
  opendoor(aptr->x-16*DEC, aptr->y);
  opendoor(aptr->x, aptr->y+16*DEC);
  opendoor(aptr->x, aptr->y-16*DEC);
  opendoor(aptr->x+16*DEC, aptr->y+16*DEC);
  opendoor(aptr->x-16*DEC, aptr->y+16*DEC);
  opendoor(aptr->x+16*DEC, aptr->y-16*DEC);
  opendoor(aptr->x-16*DEC, aptr->y-16*DEC);

  /* Spawn many chunks */
  for (c = 0; c < pieces; c++)
  {
          spawnchunk(aptr, DMG_BOMBCHUNK);
  }
  /* *** END copied straight from Cadaver's sources END *** */

  // *** Some extra visual flavor
  if (extraflavor) for (c = 0; c < pieces/2; c++)
  {
    int speed;
    ACTOR *rptr;
    rptr = spawnactor(ACTOR_RICOCHET, aptr->x, aptr->y, 0);
    if (rptr) {
      rptr->special = 1;
      rptr->angle = rand()&0x3ff; // ***
      speed = rand()%2000+400;
      rptr->speedx = (sintable[rptr->angle]*speed/256);
      rptr->speedy = -(sintable[rptr->angle+COS]*speed/256);
      rptr->health = rand()%20+20;
      rptr->enemycontrol = aptr->x;  // Position buffer with weird
      rptr->enemycounter = aptr->y;  // slot names!
    }
  }
}

static int
takes_weapon(unsigned type, unsigned amount)
{
        int old_ammo = ammo[type];
        int new_ammo = ammo[type] + amount;
        if (new_ammo > maxammo[type])
                new_ammo = maxammo[type];
        if (new_ammo <= old_ammo)
                return 0;       /* don't take it */

        ammo[type] = new_ammo;
        if (type < WEAP_GRENADE) /* Grenades and above too dangerous *** */
        {
                if (weapon < type && old_ammo == 0)
                        weapon = type;
        }
        return 1;
}

ACTOR *spawnitem(ACTOR *aptr, int type)
{
	ACTOR *sptr;
        int speed;

	sptr = spawnactor(type, aptr->x, aptr->y, rand() & 0x3ff);
	if (!sptr) return NULL;
	speed = DEC + (rand() % (3*DEC));
	sptr->speedx = (sintable[sptr->angle]*speed/DEC);
	sptr->speedy = -(sintable[sptr->angle+COS]*speed/DEC);
	return sptr;
}

void movebullet(ACTOR *aptr)
{
        ACTOR *cptr;
        int blkinf;

        aptr->frame++;
        if (aptr->frame > 70)
        {
        	aptr->type = ACTOR_NONE;
        	return;
        }

        blkinf = map_getblockinfo(0, aptr->x/DEC, aptr->y/DEC) | map_getblockinfo(1, aptr->x/DEC, aptr->y/DEC);
        if (blkinf & INF_WALL)
        {
                if (blkinf & INF_GLASS)
                {
                	if (rand() & 1) breakglass(aptr, aptr->x, aptr->y);
                }
                // *** Lots of changes in following lines
                traceoutofwall(aptr);
                if ((aptr->enemymode == 0) || (rand()%100 < 50)) {
                  // No ricochet!
                  aptr->type = ACTOR_SMOKE;
                  aptr->frame = 0;
                }
                else
                {
                  // Ricochet!
                  int speed;
                  ACTOR *sptr = spawnactor(ACTOR_SMOKE, aptr->x, aptr->y, 0);
                  sptr->frame = 0;
                  aptr->type = ACTOR_RICOCHET;
                  aptr->frame = 0;
                  aptr->angle = rand()&0x3ff; // ***
                  speed = rand()%500;
                  aptr->speedx /= 2;
                  aptr->speedy /= 2;
                  aptr->speedx += (sintable[aptr->angle]*speed/256);
                  aptr->speedy += -(sintable[aptr->angle+COS]*speed/256);
                  aptr->health = 20;
                  playownedpositionalfx(aptr, aptr->x, aptr->y, FXCHAN_RICOCHET, SMP_RICOCHET1+rand()%2, 18000+(rand()%5000),15+rand()%20);
                }
        	return;
        }
        aptr->enemycontrol = aptr->x;  // Position buffer with weird
        aptr->enemycounter = aptr->y;  // slot names!
        aptr->x += aptr->speedx;
        aptr->y += aptr->speedy;

        /* Bullet collision */
        cptr = collision_no_origin(aptr);
        if (cptr)
        {
		punish(cptr, aptr, DMG_BULLET);
		aptr->frame = 0;
		aptr->type = ACTOR_SMOKE;
	        return;
        }
}

void movearrow(ACTOR *aptr)
{
        ACTOR *cptr;
        int blkinf;

        aptr->frame++;
        if (aptr->frame > 120)
        {
        	aptr->type = ACTOR_NONE;
        	return;
        }

        blkinf = map_getblockinfo(0, aptr->x/DEC, aptr->y/DEC) | map_getblockinfo(1, aptr->x/DEC, aptr->y/DEC);
        if (blkinf & INF_WALL)
        {
                if (blkinf & INF_GLASS)
                {
                	if (rand() & 1) breakglass(aptr, aptr->x, aptr->y);
                }
                if (!traceoutofwall(aptr)) aptr->enemymode = 0;
                if (!aptr->enemymode)
                {
                  // No ricochet!
                  aptr->type = ACTOR_SMOKE;
                  aptr->frame = 0;
                }
                else
                {
                  // Ricochet!
                  int speed;
                  ACTOR *sptr = spawnactor(ACTOR_SMOKE, aptr->x, aptr->y, 0);
                  sptr->frame = 0;
                  aptr->type = ACTOR_RICOCHET;
                  aptr->frame = 0;
                  aptr->angle = rand()&0x3ff; // ***
                  speed = rand()%500;
                  aptr->speedx /= 4;
                  aptr->speedy /= 4;
                  aptr->speedx += (sintable[aptr->angle]*speed/256);
                  aptr->speedy += -(sintable[aptr->angle+COS]*speed/256);
                  aptr->health = 15;
                }
        	return;
        }
        aptr->enemycontrol = aptr->x;  // Position buffer with weird
        aptr->enemycounter = aptr->y;  // slot names!
        aptr->x += aptr->speedx;
        aptr->y += aptr->speedy;

	if (aptr->health > 0)
	{
		ACTOR *bptr = spawnactor(ACTOR_BLOOD, aptr->x, aptr->y, aptr->angle);
		if (bptr)
		{
			bptr->health = 25;
			bptr->speedx = rand() % DEC/5 - DEC/10;
			bptr->speedy = rand() % DEC/5 - DEC/10;
			aptr->health--;
		}
	}

        /* Arrow collision */
        cptr = collision_no_origin(aptr);
        if (cptr)
        {
                int health = cptr->health;
                int friction = 60 + rand() % 200;

                playpositionalfx(aptr->x, aptr->y, FXCHAN_FIST, SMP_ARROWHIT, 22050, 64);
		punish(cptr, aptr, DMG_ARROW);

		if (cptr->type >= ACTOR_FIRSTDEADENEMY && cptr->type <= ACTOR_LASTDEADENEMY && aptr->frame + friction < 120)
		{
			/* Killed and penetrated the enemy.  Spew blood.  */
			aptr->health += health;
			aptr->frame += friction;
		}
		else
		{
			aptr->frame = 0;
			aptr->type = ACTOR_NONE;
			return;
		}
        }
}

void moveflame(ACTOR *aptr)
{
        ACTOR *cptr;
        int blkinf;

        aptr->frame++;
        if (aptr->frame >= 5*8)
        {
        	aptr->type = ACTOR_NONE;
        	return;
        }
        aptr->x += aptr->speedx;
        aptr->y += aptr->speedy;

        if (aptr->health)
        {
                /* Flame collision */
                cptr = collision_no_origin(aptr);
                if ((cptr) && (cptr->type != ACTOR_LEADER))
                {
        		punish(cptr, aptr, aptr->health);
        		aptr->health = 0;
                }
        }
        blkinf = map_getblockinfo(0, aptr->x/DEC, aptr->y/DEC) | map_getblockinfo(1, aptr->x/DEC, aptr->y/DEC);
        if (blkinf & INF_WALL)
        {
                aptr->type = ACTOR_NONE;
                return;
        }
}

void checkhit(ACTOR *aptr)
{
        ACTOR *cptr;
        cptr = collision_no_origin(aptr);
        if (cptr)
        {
              	switch (aptr->type)
	        {
	                case ACTOR_FISTHIT:
	                playpositionalfx(aptr->x, aptr->y, FXCHAN_FIST, SMP_FIST1+(rand()%3), 22050, 64);
	                punish(cptr, aptr, DMG_FIST);
	                break;

	                case ACTOR_WHIPHIT:
	                playpositionalfx(aptr->x, aptr->y, FXCHAN_FIST, SMP_FIST1+(rand()%3), 22050, 64);
	                punish(cptr, aptr, DMG_WHIP);
	                break;

	                case ACTOR_BNDHIT:
	                bnd_hittime = BND_MIN_HITTIME;
	                punish(cptr, aptr, DMG_BND);
	                break;
	        }
        }
	aptr->type = ACTOR_NONE;
}

void punish(ACTOR *cptr, ACTOR *aptr, int damage)
{
        ACTOR *sptr;
        int blood = 1;
        if (aptr->type == ACTOR_WHIPHIT) blood = 2;
        if (aptr->type == ACTOR_CHUNK) blood = 3;
        if (aptr->type == ACTOR_SHARD) blood = 4;
        if (aptr->type == ACTOR_BULLET) blood = 4;
        if (aptr->type == ACTOR_ARROW) blood = 4;

        /* Arrows do more damage to humans than to equipment */
        if (aptr->type == ACTOR_ARROW)
        {
                switch(cptr->type)
                {
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
                        case ACTOR_KEYBOARD:
                        case ACTOR_MULTITRACK:
                        case ACTOR_VCR_AND_TV:
                        damage /= 3;
                        break;
                }
        }

        if ((cptr != &actor[0]) || (!lifecheat)) cptr->health -= damage;

 	if (cptr->health <= 0)
 	{
                int c;

 		cptr->health = 0;
                /* If speaker died while speaking, cut sample */
                if (cptr == speaker)
                {
                 	speaker = NULL;
                 	snd_stopsample(FXCHAN_SPEECH);
                }

 		switch(cptr->type)
 		{
                        case ACTOR_KEYBOARD:
                        case ACTOR_MULTITRACK:
                        case ACTOR_VCR_AND_TV:
  			playpositionalfx(cptr->x, cptr->y, FXCHAN_EXPLODE, SMP_EXPLODE, 22050, 64);
                        alertenemies(cptr->x, cptr->y, SNDDIST_EXPLOSION/2);
                        for (c = 0; c < 10; c++)
                        {
                        	spawnchunk(cptr, DMG_CHUNK);
                        }
                 	cptr->type = ACTOR_EXPLOSION;
                 	cptr->frame = 0;
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
  			playpositionalfx(cptr->x, cptr->y, FXCHAN_EXPLODE, SMP_EXPLODE, 22050, 64);
                        alertenemies(cptr->x, cptr->y, SNDDIST_EXPLOSION/2);
                        for (c = 0; c < comphealth[cptr->type - ACTOR_ACER]; c++)
                        {
                               	spawnchunk(cptr, DMG_CHUNK);
                        }
                 	cptr->type = ACTOR_EXPLOSION;
                 	cptr->frame = 0;
                 	computers--;
                 	break;

         		case ACTOR_FISTMAN:
         		case ACTOR_PISTOLMAN:
         		case ACTOR_SHOTGUNMAN:
         		case ACTOR_UZIMAN:
         		case ACTOR_SADIST:
  			playpositionalfx(cptr->x, cptr->y, FXCHAN_DEATH, SMP_DIE1+(rand()%3), 11025, 64);
                        alertenemies(cptr->x, cptr->y, SNDDIST_DEATHGRUNT);
  			spawnblood(cptr, blood*3, aptr->angle);
  			if (deadenemyitem[cptr->type - ACTOR_FIRSTENEMY]) spawnitem(cptr, deadenemyitem[cptr->type - ACTOR_FIRSTENEMY]);
                        dropgrenades(cptr);
  			spawnmedikit(cptr);
  			closedoor(cptr->x, cptr->y);
  			if (aptr->origin == &actor[0]) reward(enemyscore[cptr->type - ACTOR_FIRSTENEMY]);
  			cptr->type = deadenemytype[cptr->type - ACTOR_FIRSTENEMY];
  			cptr->frame = 0;
  			terrorists--;
  			kills++;
			break;

         		case ACTOR_LEADER:
  			playpositionalfx(cptr->x, cptr->y, FXCHAN_DEATH, SMP_DIE1+(rand()%3), 11025, 64);
                        alertenemies(cptr->x, cptr->y, SNDDIST_DEATHGRUNT);
  			spawnblood(cptr, blood*5, aptr->angle);
                        dropgrenades(cptr);
  			closedoor(cptr->x, cptr->y);
  			if (aptr->origin == &actor[0]) reward(enemyscore[cptr->type - ACTOR_FIRSTENEMY]);
  			cptr->type = deadenemytype[cptr->type - ACTOR_FIRSTENEMY];
  			cptr->frame = 0;
  			terrorists--;
  			leaders--;
  			kills++;
			break;

         		case ACTOR_TECHNICIAN:
  			if (aptr->origin == &actor[0]) reward(enemyscore[cptr->type - ACTOR_FIRSTENEMY]);
  			playpositionalfx(cptr->x, cptr->y, FXCHAN_DEATH, SMP_DIE1+(rand()%3), 11025, 64);
                        alertenemies(cptr->x, cptr->y, SNDDIST_DEATHGRUNT);
  			spawnblood(cptr, blood*3, aptr->angle);
  			closedoor(cptr->x, cptr->y);
  			cptr->type = ACTOR_DEADTECHNICIAN;
  			cptr->frame = 0;
  			sptr = spawnitem(cptr, ACTOR_INSTRUCTIONS);
  			/* Instructions for which bomb? */
  			if (sptr)
  			{
  				sptr->health = cptr->special;
  			}
  			else
                        {
                        	/* If no free actor for instructions, turn the
                        	 * corpse into instructions */
                        	cptr->type = ACTOR_INSTRUCTIONS;
                        	cptr->health = cptr->special;
                        	cptr->frame = 0;
                        	cptr->angle = 0;
                        	cptr->speedx = 0;
                        	cptr->speedy = 0;
                        }
  			terrorists--;
  			kills++;
			break;

         		case ACTOR_BOFH:
  			playpositionalfx(cptr->x, cptr->y, FXCHAN_DEATH, SMP_DIE1+(rand()%3), 11025, 64);
  			spawnblood(cptr, blood*5, aptr->angle);
                        ensure_bnd_sound(BNDS_OFF);
                        allpatrol();
  			cptr->type = ACTOR_DEADBOFH;
  			cptr->frame = 0;
			break;
                }
        }
        else
        {
         	switch(cptr->type)
         	{
         		case ACTOR_FISTMAN:
         		case ACTOR_PISTOLMAN:
         		case ACTOR_SHOTGUNMAN:
         		case ACTOR_UZIMAN:
         		case ACTOR_SADIST:
         		case ACTOR_TECHNICIAN:
         		case ACTOR_LEADER:
  			playpositionalfx(cptr->x, cptr->y, FXCHAN_HIT, SMP_HIT1+(rand()%6), 11025, 64);
                        alertenemies(cptr->x, cptr->y, SNDDIST_DAMAGEGRUNT);
                        if (aptr->type != ACTOR_BNDHIT) {
                                cptr->speedx += sintable[aptr->angle];
                                cptr->speedy -= sintable[aptr->angle+COS];
                        }
  			spawnblood(cptr, blood, aptr->angle);
  			attackplayer_notaunt(cptr);
			break;

         		case ACTOR_BOFH:
  			playpositionalfx(cptr->x, cptr->y, FXCHAN_HIT, SMP_HIT1+(rand()%6), 11025, 64);
                        alertenemies(cptr->x, cptr->y, SNDDIST_DAMAGEGRUNT);
                        if (aptr->type != ACTOR_BNDHIT) {
                                cptr->speedx += sintable[aptr->angle];
                                cptr->speedy -= sintable[aptr->angle+COS];
                        }
  			spawnblood(cptr, blood, aptr->angle);
			break;
		}
        }
}

void spawnmedikit(ACTOR *aptr)
{
	int type = ACTOR_NONE;
	int decision = rand() & 15;
	if (lifecheat) return;
	switch (decision)
	{
		case 0:
		type = ACTOR_BIGMEDIKIT;
		break;

		case 1:
		case 2:
		case 3:
		case 4:
		type = ACTOR_SMALLMEDIKIT;
		break;
	}
	if (type) spawnitem(aptr, type);
}


void spawnblood(ACTOR *cptr, int number, int angle)
{
        ACTOR *bptr;
        int c;
        int speed;

        for (c = 0; c < number; c++)
        {
		bptr = spawnactor(ACTOR_BLOOD, cptr->x, cptr->y, (angle + (rand() & 0xff) - 0x80) & 0x3ff);
 	       	if (!bptr) break;
 	       	speed = (rand() & 0x7f) + 0x80;
        	bptr->speedx = sintable[bptr->angle] * speed / DEC;
        	bptr->speedy = -sintable[bptr->angle + COS] * speed / DEC;
        	bptr->health = 25;
        }
}

void clearactors(void)
{
	ACTOR *aptr;
	int c;

        aptr = &actor[0];
        for (c = 0; c < MAX_ACTOR; c++)
        {
        	aptr->type = ACTOR_NONE;
        	aptr++;
        }
        actors = 0;
}


ACTOR *spawnactor(int type, int x, int y, int angle)
{
	ACTOR *aptr = &actor[0];
	int c;

	if (!actors) actors = 1;

	for (c = 0; c < actors; c++)
	{
		if (!aptr->type)
		{
			aptr->type = type;
			aptr->x = x;
			aptr->y = y;
			aptr->angle = angle;
			aptr->angularspeed = 0;
			aptr->frame = 0;
			aptr->health = 0;
			aptr->speedx = 0;
			aptr->speedy = 0;
			aptr->attack = 0;
                        aptr->origin = NULL;
			aptr->attackdelay = 0;
			return aptr;
		}
		aptr++;
                if ((c >= actors-1) && (actors < MAX_ACTOR)) actors++;
	}
	return NULL;
}


int domove(ACTOR *aptr)
{
	int newx;
	int newy;
        unsigned char blkinf;
        ACTOR *cptr;
        int prefdist;

        /* Limit maximum speed to 4 so actors can't fall into
         * staircases etc.
         */
        if (aptr->speedx < -4*DEC) aptr->speedx = -4*DEC;
        if (aptr->speedy < -4*DEC) aptr->speedy = -4*DEC;
        if (aptr->speedx > 4*DEC) aptr->speedx = 4*DEC;
        if (aptr->speedy > 4*DEC) aptr->speedy = 4*DEC;
	newx = aptr->x + aptr->speedx;
	newy = aptr->y + aptr->speedy;

        cptr = collision(aptr, newx, newy, &prefdist);
        if (cptr)
        {
                int impetusx = (cptr->x - newx);
                int impetusy = (cptr->y - newy);
                int dist;
                const int maximpetus = DEC/4;

                dist = abs(impetusx);
                if (dist < abs(impetusy))
                        dist = abs(impetusy);

                if (dist == 0) {
                        impetusx = rand() % (2*maximpetus) - maximpetus;
                        impetusy = rand() % (2*maximpetus) - maximpetus;
                } else {
                        impetusx = impetusx * (prefdist - dist) / dist;
                        impetusy = impetusy * (prefdist - dist) / dist;
                }

                if (impetusx >  maximpetus) impetusx =  maximpetus;
                if (impetusx < -maximpetus) impetusx = -maximpetus;
                if (impetusy >  maximpetus) impetusy =  maximpetus;
                if (impetusy < -maximpetus) impetusy = -maximpetus;
                
                aptr->speedx -= impetusx;
                aptr->speedy -= impetusy;
                cptr->speedx += impetusx;
                cptr->speedy += impetusy;

                newx = aptr->x + aptr->speedx;
                newy = aptr->y + aptr->speedy;
        }

	blkinf = map_getblockinfo(0, newx/DEC, newy/DEC) | map_getblockinfo(1, newx/DEC, newy/DEC);

	/* Check for a door and open it */
	if (blkinf & INF_DOOR)
	{
		opendoor(newx, newy);
	}

        /* No movement? */
        if ((!aptr->speedx) && (!aptr->speedy)) return 1;


        /* Check if move possible */
	if (!(blkinf & (INF_OBSTACLE | INF_WALL)))
	{
                int oldx = aptr->x;
                int oldy = aptr->y;
		aptr->x = newx;
		aptr->y = newy;
		checkdoorclose(aptr, oldx, oldy);
		checkstairs(aptr);
		return 1;
	}

	/* Now try motion only in X-direction */
	if (aptr->speedx)
	{
		newx = aptr->x + aptr->speedx;
		newy = aptr->y;
		blkinf = map_getblockinfo(0, newx/DEC, newy/DEC) | map_getblockinfo(1, newx/DEC, newy/DEC);
	        blkinf &= 3; /* Leave obstacle bits */
	        if (!blkinf)
	        {
                        int oldx = aptr->x;
                        int oldy = aptr->y;
        		aptr->x = newx;
        		aptr->y = newy;
        		checkdoorclose(aptr, oldx, oldy);
			checkstairs(aptr);
	        	return 0;
	        }
	}
	/* Now try motion only in Y-direction */
	if (aptr->speedy)
	{
		newx = aptr->x;
		newy = aptr->y + aptr->speedy;
		blkinf = map_getblockinfo(0, newx/DEC, newy/DEC) | map_getblockinfo(1, newx/DEC, newy/DEC);
	        blkinf &= 3; /* Leave obstacle bits */
	        if (!blkinf)
	        {
                        int oldx = aptr->x;
                        int oldy = aptr->y;
        		aptr->x = newx;
        		aptr->y = newy;
        		checkdoorclose(aptr, oldx, oldy);
			checkstairs(aptr);
	        	return 0;
	        }
	}
	return 0;
}

int checkstairs(ACTOR *aptr)
{
        int c;
        int blkinf = map_getblockinfo(0, aptr->x/DEC, aptr->y/DEC) | map_getblockinfo(1, aptr->x/DEC, aptr->y/DEC);

        /* Going up/down stairs */
        if (blkinf == INF_STAIRS)
        {
                int nxb = aptr->x >> 12;
                int nyb = aptr->y >> 12;
                for (c = 0; c < numstairs; c++)
                {
                        const STAIREND *src  = &stairs[c].src;
                        const STAIREND *dest = &stairs[c].dest;
                        int end;
                        for (end = 0; end < 2; ++end)
                        {
                                const STAIREND *swap;
                                if (nxb == src->xb && nyb == src->yb)
                                {
                                        aptr->x = ((dest->xb<<12) + 8*DEC
                                                   + 16*sintable[dest->angle]);
                                        aptr->y = ((dest->yb<<12) + 8*DEC
                                                   - 16*sintable[dest->angle+COS]);
                                        aptr->angle += dest->angle - src->angle;
                                        aptr->angle ^= 0x200;
                                        aptr->angle &= 0x3FF;
                                        aptr->speedx = 0;
                                        aptr->speedy = 0;

                                        if (aptr == &actor[0])
                                        {
	                                /*
	                                 * If it is the player, move "camera" and enable
	                                 * fastforward mode which will lose 10 seconds
	                                 * (inspired by Die Hard on NES :-))
	                                 */
	                                        xpos = aptr->x - 160*DEC;
	                                        ypos = aptr->y - 100*DEC;
	                                        fastforward = 10 * 70;
	                                }

                                        return 1;
                                }
                                swap = src;
                                src = dest;
                                dest = swap;
                        }
		}
	}
	return 0;
}

int domove_nodoors(ACTOR *aptr)
{
	int newx;
	int newy;
        unsigned char blkinf;

        if (aptr->speedx < -4*DEC) aptr->speedx = -4*DEC;
        if (aptr->speedy < -4*DEC) aptr->speedy = -4*DEC;
        if (aptr->speedx > 4*DEC) aptr->speedx = 4*DEC;
        if (aptr->speedy > 4*DEC) aptr->speedy = 4*DEC;
	newx = aptr->x + aptr->speedx;
	newy = aptr->y + aptr->speedy;
	blkinf = map_getblockinfo(0, newx/DEC, newy/DEC) | map_getblockinfo(1, newx/DEC, newy/DEC);

        /* Check if move possible */
	if (!(blkinf & (INF_OBSTACLE | INF_WALL)))
	{
                int oldx = aptr->x;
                int oldy = aptr->y;
        	aptr->x = newx;
        	aptr->y = newy;
        	checkdoorclose(aptr, oldx, oldy);
		checkstairs(aptr);
		return 1;
	}

	/* Now try motion only in X-direction */
	if (aptr->speedx)
	{
		newx = aptr->x + aptr->speedx;
		newy = aptr->y;
		blkinf = map_getblockinfo(0, newx/DEC, newy/DEC) | map_getblockinfo(1, newx/DEC, newy/DEC);
	        blkinf &= 3; /* Leave obstacle bits */
	        if (!blkinf)
	        {
                        int oldx = aptr->x;
                        int oldy = aptr->y;
                	aptr->x = newx;
                	aptr->y = newy;
                	checkdoorclose(aptr, oldx, oldy);
        		checkstairs(aptr);
	        	return 0;
	        }
	}
	/* Now try motion only in Y-direction */
	if (aptr->speedy)
	{
		newx = aptr->x;
		newy = aptr->y + aptr->speedy;
		blkinf = map_getblockinfo(0, newx/DEC, newy/DEC) | map_getblockinfo(1, newx/DEC, newy/DEC);
	        blkinf &= 3; /* Leave obstacle bits */
	        if (!blkinf)
	        {
                        int oldx = aptr->x;
                        int oldy = aptr->y;
                	aptr->x = newx;
                	aptr->y = newy;
                	checkdoorclose(aptr, oldx, oldy);
        		checkstairs(aptr);
	        	return 0;
	        }
	}
	return 0;
}

void checkdoorclose(ACTOR *aptr, int oldx, int oldy)
{
	int blkinf1, blkinf2;

	blkinf1 = map_getblockinfo(1, aptr->x/DEC, aptr->y/DEC);
	blkinf2 = map_getblockinfo(1, oldx/DEC, oldy/DEC);

	/* If previous position has an open door and the new hasn't,
	 * close the door.
	 */
	if ((blkinf2 & INF_OPENDOOR) && (!(blkinf1 & INF_OPENDOOR)))
	{
		closedoor(oldx, oldy);
	}
}


