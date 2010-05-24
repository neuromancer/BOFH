#include "bofh.h"
#include "sincos.h"
#include "extern.h"

void grenadeattack(ACTOR *aptr)
{
        int a, ad, d;
        if (!aptr->special) return; /* No more grenades */
        if (aptr->attackdelay) return;

        a = findangle(aptr->x, aptr->y, actor[0].x, actor[0].y);
        ad = abs(angledist(aptr->angle, a));
        d = finddist(aptr->x, aptr->y, actor[0].x, actor[0].y);

        /* Long-distance attack */
        if ((rand()%80)<difficulty)
        {
        	if ((d > GRENADE_MINDIST) && (ad < 30) && (checkwalls(aptr)))
                {
                        if (d > 200) d = 200;
                        if (spawngrenade(aptr, 20, d*3))
                        {
                		playpositionalfx(aptr->x, aptr->y, FXCHAN_ENEMYSHOOT, SMP_SWISH, 22050, 64);
                                aptr->attackdelay = 9;
                                aptr->special--;
                                aptr->enemycounter = 100; /* Don't follow player for a while */
                                aptr->angle ^= 0x200; /* Turn around */
                        }
                        return;
                }
        }
}

void dropgrenades(ACTOR *aptr)
{
        ACTOR *sptr;
        if (!aptr->special) return; /* No grenades */
        sptr = spawnitem(aptr, ACTOR_GRENADE);
        if (!sptr) return;
        sptr->health = aptr->special; /* Number of grenades */
        aptr->special = 0;
}

void fistmanattack(ACTOR *aptr)
{
        if (aptr->enemymode != MODE_ATTACK) return;

        if (aptr->attackdelay) aptr->attackdelay--;

        grenadeattack(aptr);

        if (finddist(aptr->x, aptr->y, actor[0].x, actor[0].y) <= 10)
        {
		if ((actor[0].health) && (!aptr->attackdelay) && ((rand()&63) >= fistpr[difficulty]))
		{
			aptr->attack = 9;
			playpositionalfx(aptr->x, aptr->y, FXCHAN_ENEMYSHOOT, SMP_SWISH, 22050, 64);
   			aptr->attackdelay = attackdelaytbl[WEAP_FISTS]*2;
		}
        }

	if (aptr->attack)
	{
		aptr->attack--;
		aptr->frame = 0;
		if (aptr->attack == 5)
		{
			ACTOR *sptr;
			int blkinf;

			sptr = spawnactor(ACTOR_FISTHIT, aptr->x + sintable[aptr->angle]*6, aptr->y - sintable[aptr->angle+COS]*6, aptr->angle);
			if (sptr)
			{
				sptr->origin = aptr;
	                        /* Can't hit behind walls */
				blkinf = map_getblockinfo(0, sptr->x/DEC, sptr->y/DEC) | map_getblockinfo(1, sptr->x/DEC, sptr->y/DEC);
	                        if (blkinf & INF_WALL)
	                        {
	                        	playpositionalfx(sptr->x, sptr->y, FXCHAN_FIST, SMP_FIST1+(rand()%3), 22050, 64);
	                        	sptr->type = ACTOR_NONE;
	                        }
	                }
                }
        }
}

static void enemyphysrotaccel(ACTOR *aptr, int limit)
{
        aptr->angularspeed = aptr->angularspeed * 9 / 10;
        if (aptr->enemycontrol & CB_LEFT) aptr->angularspeed -= 2;
        if (aptr->enemycontrol & CB_RIGHT) aptr->angularspeed += 2;

        if (aptr->angularspeed > limit) aptr->angularspeed = limit;
        if (aptr->angularspeed < -limit) aptr->angularspeed = -limit;
}

static void enemyphystransaccel(ACTOR *aptr, int divisor)
{
        int strafe = 0, ahead = 0;
        int si, co;

        if (!aptr->enemycontrol) return;

        if (aptr->enemycontrol & CB_FORWARD) ahead += 256;
        if (aptr->enemycontrol & CB_BACKWARD) ahead -= 256;
        if (aptr->enemycontrol & CB_STRAFELEFT) strafe -= 256;
        if (aptr->enemycontrol & CB_STRAFERIGHT) strafe += 256;
        if (strafe && ahead) { strafe = strafe * 12 / 17; ahead = ahead * 12 / 17; }

        ahead /= divisor;
        strafe /= divisor;

        si = sintable[aptr->angle];
        co = sintable[aptr->angle + COS];

        aptr->speedx += (si * ahead + co * strafe) / 256;
        aptr->speedy -= (co * ahead - si * strafe) / 256;
}

/* Kitkavoima */
static void enemyphystransfrict(ACTOR *aptr)
{
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
}

static void enemyphysrotmove(ACTOR *aptr)
{
        aptr->angle += aptr->angularspeed;
        aptr->angle &= 0x3ff;
}

static void enemyanimate(ACTOR *aptr)
{
        int speed = squareroot(aptr->speedx*aptr->speedx+aptr->speedy*aptr->speedy);
        aptr->frame += speed / 40;
        aptr->frame &= 0xff;
}

static int enemyveryfar(ACTOR *aptr)
{
	int adx, ady;

        adx = abs(actor[0].x - aptr->x);
        ady = abs(actor[0].y - aptr->y);

        /* If enemy is very far away, do nothing */
        return (adx >= VERYFAR*DEC) || (ady >= VERYFAR*DEC);
}

static void enemyaiifmodepatrol(ACTOR *aptr)
{
        if (aptr->enemymode == MODE_PATROL)
        {
                if (aptr->attack) aptr->attack = 0;
                if (!domove_nodoors(aptr)) aptr->enemycontrol = CB_RIGHT+CB_FORWARD;
                if ((rand() & 63) < 2) aptr->enemycontrol = CB_FORWARD;
                if ((rand() & 63) == 3) aptr->enemycontrol = CB_LEFT;
                if ((rand() & 63) == 4) aptr->enemycontrol = CB_RIGHT;
                if ((rand() & 63) == 5) aptr->enemycontrol = 0;
		detectplayer(aptr);
	}
}

void fistmancontrol(ACTOR *aptr)
{
        if (enemyveryfar(aptr)) return;

        enemyphysrotaccel(aptr, 20);
	if (!aptr->attack)
                enemyphystransaccel(aptr, 4);
        enemyphystransfrict(aptr);
        enemyphysrotmove(aptr);

        enemyanimate(aptr);

        enemyaiifmodepatrol(aptr);
        if (aptr->enemymode == MODE_ATTACK)
        {
                int a, da;

                /* Unobstructed movement? */
		if (domove(aptr))
		{
                        /* "forget player" counter? */
                	if (!aptr->enemycounter)
                	{
                                /* Hunt player */
                                if (finddist(aptr->x, aptr->y, actor[0].x, actor[0].y) > 10)
                                {
			                aptr->enemycontrol = CB_FORWARD;
			                a = findangle(aptr->x, aptr->y, actor[0].x, actor[0].y);
					da = angledist(aptr->angle, a);
					if (da < 0) aptr->enemycontrol |= CB_LEFT;
					if (da > 0) aptr->enemycontrol |= CB_RIGHT;
				}
				else
				{
					aptr->enemycontrol = 0;
			                a = findangle(aptr->x, aptr->y, actor[0].x, actor[0].y);
					da = angledist(aptr->angle, a);
					if (da < 0) aptr->enemycontrol |= CB_LEFT;
					if (da > 0) aptr->enemycontrol |= CB_RIGHT;
				}

			}
			else aptr->enemycontrol = CB_FORWARD;
		}
		else
		{
			/* Forget player for half second, try to find way out */
			if (!aptr->enemycounter)
			{
                                if (!(rand() & 15))
                                {
					aptr->enemycounter = 35;
					if (rand() & 1)
			                aptr->enemycontrol = CB_RIGHT | CB_FORWARD;
			                else
			                aptr->enemycontrol = CB_LEFT | CB_FORWARD;
				}
				else
				{
		                        /* Hunt player */
					aptr->enemycontrol = CB_FORWARD;
					a = findangle(aptr->x, aptr->y, actor[0].x, actor[0].y);
					da = angledist(aptr->angle, a);
					if (da < 0) aptr->enemycontrol |= CB_LEFT;
					if (da > 0) aptr->enemycontrol |= CB_RIGHT;
				}
			}
			else
			{
                                if (!(rand() & 15))
                                {
					if (rand() & 1)
			                aptr->enemycontrol = CB_RIGHT | CB_FORWARD;
			                else
			                aptr->enemycontrol = CB_LEFT | CB_FORWARD;
				}
			}
                }
                if (aptr->enemycounter) aptr->enemycounter--;
        }
	fistmanattack(aptr);
}

void technicianattack(ACTOR *aptr)
{
        if (aptr->enemymode != MODE_ATTACK) return;

        if (aptr->attackdelay) aptr->attackdelay--;

        if (finddist(aptr->x, aptr->y, actor[0].x, actor[0].y) <= 10)
        {
		if ((actor[0].health) && (!aptr->attackdelay) && ((rand()&63) >= fistpr[difficulty]))
		{
			aptr->attack = 9;
			playpositionalfx(aptr->x, aptr->y, FXCHAN_ENEMYSHOOT, SMP_SWISH, 22050, 64);
   			aptr->attackdelay = attackdelaytbl[WEAP_FISTS]*2;
		}
        }

	if (aptr->attack)
	{
		aptr->attack--;
		aptr->frame = 0;
		if (aptr->attack == 5)
		{
			ACTOR *sptr;
			int blkinf;

			sptr = spawnactor(ACTOR_FISTHIT, aptr->x + sintable[aptr->angle]*6, aptr->y - sintable[aptr->angle+COS]*6, aptr->angle);
			if (sptr)
			{
				sptr->origin = aptr;
	                        /* Can't hit behind walls */
				blkinf = map_getblockinfo(0, sptr->x/DEC, sptr->y/DEC) | map_getblockinfo(1, sptr->x/DEC, sptr->y/DEC);
	                        if (blkinf & INF_WALL)
	                        {
	                        	playpositionalfx(sptr->x, sptr->y, FXCHAN_FIST, SMP_FIST1+(rand()%3), 22050, 64);
	                        	sptr->type = ACTOR_NONE;
	                        }
	                }
                }
        }
}

void techniciancontrol(ACTOR *aptr)
{
        if (enemyveryfar(aptr)) return;

        enemyphysrotaccel(aptr, 20);
	if (!aptr->attack)
                enemyphystransaccel(aptr, 4);
        enemyphystransfrict(aptr);
        enemyphysrotmove(aptr);

        enemyanimate(aptr);

        enemyaiifmodepatrol(aptr);
        if (aptr->enemymode == MODE_ATTACK)
        {
                int a, da;

                /* Unobstructed movement? */
		if (domove(aptr))
		{
                        /* "forget player" counter? */
                	if (!aptr->enemycounter)
                	{
                                /* Run away from player */
                                if (finddist(aptr->x, aptr->y, actor[0].x, actor[0].y) > 30)
                                {
			                aptr->enemycontrol = CB_FORWARD;
			                a = findangle(aptr->x, aptr->y, actor[0].x, actor[0].y);
					da = angledist(aptr->angle, a ^ 0x200);
					if (da < 0) aptr->enemycontrol |= CB_LEFT;
					if (da > 0) aptr->enemycontrol |= CB_RIGHT;
				}
				else
				{
			                aptr->enemycontrol = CB_FORWARD;
			                a = findangle(aptr->x, aptr->y, actor[0].x, actor[0].y);
					da = angledist(aptr->angle, a);
					if (da < 0) aptr->enemycontrol |= CB_LEFT;
					if (da > 0) aptr->enemycontrol |= CB_RIGHT;
				}

			}
			else aptr->enemycontrol = CB_FORWARD;
		}
		else
		{
			/* Forget player for half second, try to find way out */
			if (!aptr->enemycounter)
			{
                                if (!(rand() & 3))
                                {
					aptr->enemycounter = 35;
					if (rand() & 1)
			                aptr->enemycontrol = CB_RIGHT | CB_FORWARD;
			                else
			                aptr->enemycontrol = CB_LEFT | CB_FORWARD;
				}
				else
				{
		                        /* Run away */
					aptr->enemycontrol = CB_FORWARD;
					a = findangle(aptr->x, aptr->y, actor[0].x, actor[0].y);
					da = angledist(aptr->angle, a ^ 0x200);
					if (da < 0) aptr->enemycontrol |= CB_LEFT;
					if (da > 0) aptr->enemycontrol |= CB_RIGHT;
				}
			}
			else
			{
                                if (!(rand() & 15))
                                {
					if (rand() & 1)
			                aptr->enemycontrol = CB_RIGHT | CB_FORWARD;
			                else
			                aptr->enemycontrol = CB_LEFT | CB_FORWARD;
				}
			}
                }
                if (aptr->enemycounter) aptr->enemycounter--;
        }
	technicianattack(aptr);
}


void pistolmancontrol(ACTOR *aptr)
{
        if (enemyveryfar(aptr)) return;

        enemyphysrotaccel(aptr, 20);
	if (!aptr->attack)
                enemyphystransaccel(aptr, 4);
        enemyphystransfrict(aptr);
        enemyphysrotmove(aptr);

        enemyanimate(aptr);

        enemyaiifmodepatrol(aptr);
        if (aptr->enemymode == MODE_ATTACK)
        {
                int a, da;

                /* Unobstructed movement? */
		if (domove(aptr))
		{
                        /* "forget player" counter? */
                	if (!aptr->enemycounter)
                	{
                                /* Hunt player */
			        if (finddist(aptr->x, aptr->y, actor[0].x, actor[0].y) > 20) aptr->enemycontrol = CB_FORWARD;
			        else aptr->enemycontrol = 0;
			       	a = findangle(aptr->x, aptr->y, actor[0].x, actor[0].y);
			        da = angledist(aptr->angle, a);
			        if (da < 0) aptr->enemycontrol |= CB_LEFT;
			        if (da > 0) aptr->enemycontrol |= CB_RIGHT;
			}
			else aptr->enemycontrol = CB_FORWARD;
		}
		else
		{
			/* Forget player for half second, try to find way out */
			if (!aptr->enemycounter)
			{
                                if (!(rand() & 15))
                                {
					aptr->enemycounter = 35;
					if (rand() & 1)
			                aptr->enemycontrol = CB_RIGHT | CB_FORWARD;
			                else
			                aptr->enemycontrol = CB_LEFT | CB_FORWARD;
				}
				else
				{
		                        /* Hunt player */
					aptr->enemycontrol = CB_FORWARD;
					a = findangle(aptr->x, aptr->y, actor[0].x, actor[0].y);
					da = angledist(aptr->angle, a);
					if (da < 0) aptr->enemycontrol |= CB_LEFT;
					if (da > 0) aptr->enemycontrol |= CB_RIGHT;
				}
			}
			else
			{
                                if (!(rand() & 15))
                                {
					if (rand() & 1)
			                aptr->enemycontrol = CB_RIGHT | CB_FORWARD;
			                else
			                aptr->enemycontrol = CB_LEFT | CB_FORWARD;
				}
			}
                }
                if (aptr->enemycounter) aptr->enemycounter--;
        }
        pistolmanattack(aptr);
}

void pistolmanattack(ACTOR *aptr)
{
        int a, ad;
        if (aptr->attackdelay) aptr->attackdelay--;
        if (aptr->enemymode != MODE_ATTACK) return;
	if (!checkvision(aptr)) return;
        grenadeattack(aptr);

        a = findangle(aptr->x, aptr->y, actor[0].x, actor[0].y);
        ad = angledist(aptr->angle, a);
	if (abs(ad) > 40) return;

	if ((!aptr->attackdelay) && (finddist(aptr->x, aptr->y, actor[0].x, actor[0].y) < PISTOL_MAXDIST) && ((rand() & 63) >= pistolpr[difficulty]))
        {
		playpositionalfx(aptr->x, aptr->y, FXCHAN_ENEMYSHOOT, SMP_PISTOL, 22050, 64);
                alertenemies(aptr->x, aptr->y, SNDDIST_GUN);
   		aptr->attackdelay = attackdelaytbl[WEAP_PISTOL];
                spawnbullet(aptr, 30, 0, 1); // ***
                spawnshell(aptr); // ***
	}
}

void shotgunmancontrol(ACTOR *aptr)
{
        if (enemyveryfar(aptr)) return;

        enemyphysrotaccel(aptr, 15);
	if (!aptr->attack)
                enemyphystransaccel(aptr, 5);
        enemyphystransfrict(aptr);
        enemyphysrotmove(aptr);

        enemyanimate(aptr);

        enemyaiifmodepatrol(aptr);
        if (aptr->enemymode == MODE_ATTACK)
        {
                int a, da;

                /* Unobstructed movement? */
		if (domove(aptr))
		{
                        /* "forget player" counter? */
                	if (!aptr->enemycounter)
                	{
                                /* Hunt player */
			        if (finddist(aptr->x, aptr->y, actor[0].x, actor[0].y) > 30) aptr->enemycontrol = CB_FORWARD;
			        else aptr->enemycontrol = 0;
			       	a = findangle(aptr->x, aptr->y, actor[0].x, actor[0].y);
			        da = angledist(aptr->angle, a);
			        if (da < 0) aptr->enemycontrol |= CB_LEFT;
			        if (da > 0) aptr->enemycontrol |= CB_RIGHT;
			}
			else aptr->enemycontrol = CB_FORWARD;
		}
		else
		{
			/* Forget player for half second, try to find way out */
			if (!aptr->enemycounter)
			{
                                if (!(rand() & 15))
                                {
					aptr->enemycounter = 35;
					if (rand() & 1)
			                aptr->enemycontrol = CB_RIGHT | CB_FORWARD;
			                else
			                aptr->enemycontrol = CB_LEFT | CB_FORWARD;
				}
				else
				{
		                        /* Hunt player */
					aptr->enemycontrol = CB_FORWARD;
					a = findangle(aptr->x, aptr->y, actor[0].x, actor[0].y);
					da = angledist(aptr->angle, a);
					if (da < 0) aptr->enemycontrol |= CB_LEFT;
					if (da > 0) aptr->enemycontrol |= CB_RIGHT;
				}
			}
			else
			{
                                if (!(rand() & 15))
                                {
					if (rand() & 1)
			                aptr->enemycontrol = CB_RIGHT | CB_FORWARD;
			                else
			                aptr->enemycontrol = CB_LEFT | CB_FORWARD;
				}
			}
                }
                if (aptr->enemycounter) aptr->enemycounter--;
        }
        shotgunmanattack(aptr);
}

void shotgunmanattack(ACTOR *aptr)
{
        int a, ad;

        /* Shotgun load *** */
        if (aptr->attackdelay == 20) playpositionalfx(aptr->x, aptr->y, FXCHAN_GUNLOAD, SMP_GUNLOAD, 22050, 16);
        else if (aptr->attackdelay == 10) spawnshotgshell(aptr);

        if (aptr->attackdelay) aptr->attackdelay--;
        if (aptr->enemymode != MODE_ATTACK) return;
	if (!checkvision(aptr)) return;

        grenadeattack(aptr);

        a = findangle(aptr->x, aptr->y, actor[0].x, actor[0].y);
        ad = angledist(aptr->angle, a);
	if (abs(ad) > 80) return;

	if ((!aptr->attackdelay) && (finddist(aptr->x, aptr->y, actor[0].x, actor[0].y) < SHOTGUN_MAXDIST) && ((rand() & 63) >= shotgunpr[difficulty]))
        {
		playpositionalfx(aptr->x, aptr->y, FXCHAN_ENEMYSHOOT, SMP_SHOTGUN, 22050, 64);
                alertenemies(aptr->x, aptr->y, SNDDIST_GUN);
   		aptr->attackdelay = attackdelaytbl[WEAP_SHOTGUN];
                spawnbullet(aptr, 30, 0, 0); // ***
                spawnbullet(aptr, 30, 0, 0); //
                spawnbullet(aptr, 30, 0, 0); //
	}
}

void uzimancontrol(ACTOR *aptr)
{
        if (enemyveryfar(aptr)) return;

        enemyphysrotaccel(aptr, 15);
	if (!aptr->attack)
                enemyphystransaccel(aptr, 5);
        enemyphystransfrict(aptr);
        enemyphysrotmove(aptr);

        enemyanimate(aptr);

        enemyaiifmodepatrol(aptr);
        if (aptr->enemymode == MODE_ATTACK)
        {
                int a, da;

                /* Unobstructed movement? */
		if (domove(aptr))
		{
                        /* "forget player" counter? */
                	if (!aptr->enemycounter)
                	{
                                /* Hunt player */
			        if (finddist(aptr->x, aptr->y, actor[0].x, actor[0].y) > 20) aptr->enemycontrol = CB_FORWARD;
			        else aptr->enemycontrol = 0;
			       	a = findangle(aptr->x, aptr->y, actor[0].x, actor[0].y);
			        da = angledist(aptr->angle, a);
			        if (da < 0) aptr->enemycontrol |= CB_LEFT;
			        if (da > 0) aptr->enemycontrol |= CB_RIGHT;
			}
			else aptr->enemycontrol = CB_FORWARD;
		}
		else
		{
			/* Forget player for half second, try to find way out */
			if (!aptr->enemycounter)
			{
                                if (!(rand() & 15))
                                {
					aptr->enemycounter = 35;
					if (rand() & 1)
			                aptr->enemycontrol = CB_RIGHT | CB_FORWARD;
			                else
			                aptr->enemycontrol = CB_LEFT | CB_FORWARD;
				}
				else
				{
		                        /* Hunt player */
					aptr->enemycontrol = CB_FORWARD;
					a = findangle(aptr->x, aptr->y, actor[0].x, actor[0].y);
					da = angledist(aptr->angle, a);
					if (da < 0) aptr->enemycontrol |= CB_LEFT;
					if (da > 0) aptr->enemycontrol |= CB_RIGHT;
				}
			}
			else
			{
                                if (!(rand() & 15))
                                {
					if (rand() & 1)
			                aptr->enemycontrol = CB_RIGHT | CB_FORWARD;
			                else
			                aptr->enemycontrol = CB_LEFT | CB_FORWARD;
				}
			}
                }
                if (aptr->enemycounter) aptr->enemycounter--;
        }
        uzimanattack(aptr);
}

void uzimanattack(ACTOR *aptr)
{
        int a, ad;
        if (aptr->attackdelay) aptr->attackdelay--;

        if (aptr->enemymode != MODE_ATTACK)
        {
        	aptr->attack = 0;
        	return;
        }

        grenadeattack(aptr);

        if ((!aptr->attackdelay) && (aptr->attack))
	{
		aptr->attack--;
		playpositionalfx(aptr->x, aptr->y, FXCHAN_ENEMYSHOOT, SMP_UZI, 22050, 64);
                alertenemies(aptr->x, aptr->y, SNDDIST_GUN);
   		aptr->attackdelay = attackdelaytbl[WEAP_UZI];
                spawnbullet(aptr, 40, 1, 1); // ***
	}
	if (!checkvision(aptr)) return;
        a = findangle(aptr->x, aptr->y, actor[0].x, actor[0].y);
        ad = angledist(aptr->angle, a);
	if (abs(ad) > 80) return;

	if ((!aptr->attack) && (!aptr->attackdelay) && (finddist(aptr->x, aptr->y, actor[0].x, actor[0].y) < UZIMAN_MAXDIST) && ((rand() & 63) >= uzipr[difficulty]))
        {
        	aptr->attack = 1 + (rand()%5);
        }
}

void leadercontrol(ACTOR *aptr)
{
        if (enemyveryfar(aptr)) return;

        enemyphysrotaccel(aptr, 15);
        enemyphystransaccel(aptr, 6);
        enemyphystransfrict(aptr);
        enemyphysrotmove(aptr);

        enemyanimate(aptr);

        enemyaiifmodepatrol(aptr);
        if (aptr->enemymode == MODE_ATTACK)
        {
                int a, da;

                /* Unobstructed movement? */
		if (domove(aptr))
		{
                        /* "forget player" counter? */
                	if (!aptr->enemycounter)
                	{
                                /* Hunt player */
			        if (finddist(aptr->x, aptr->y, actor[0].x, actor[0].y) > 20) aptr->enemycontrol = CB_FORWARD;
			        else aptr->enemycontrol = 0;
			       	a = findangle(aptr->x, aptr->y, actor[0].x, actor[0].y);
			        da = angledist(aptr->angle, a);
			        if (da < 0) aptr->enemycontrol |= CB_LEFT;
			        if (da > 0) aptr->enemycontrol |= CB_RIGHT;
			}
			else aptr->enemycontrol = CB_FORWARD;
		}
		else
		{
			/* Forget player for half second, try to find way out */
			if (!aptr->enemycounter)
			{
                                if (!(rand() & 15))
                                {
					aptr->enemycounter = 35;
					if (rand() & 1)
			                aptr->enemycontrol = CB_RIGHT | CB_FORWARD;
			                else
			                aptr->enemycontrol = CB_LEFT | CB_FORWARD;
				}
				else
				{
		                        /* Hunt player */
					aptr->enemycontrol = CB_FORWARD;
					a = findangle(aptr->x, aptr->y, actor[0].x, actor[0].y);
					da = angledist(aptr->angle, a);
					if (da < 0) aptr->enemycontrol |= CB_LEFT;
					if (da > 0) aptr->enemycontrol |= CB_RIGHT;
				}
			}
			else
			{
                                if (!(rand() & 15))
                                {
					if (rand() & 1)
			                aptr->enemycontrol = CB_RIGHT | CB_FORWARD;
			                else
			                aptr->enemycontrol = CB_LEFT | CB_FORWARD;
				}
			}
                }
                if (aptr->enemycounter) aptr->enemycounter--;
        }
        leaderattack(aptr);
}

void leaderattack(ACTOR *aptr)
{
        int a, ad;
        if (aptr->attackdelay) aptr->attackdelay--;

        if (aptr->enemymode != MODE_ATTACK)
        {
        	aptr->attack = 0;
        	return;
        }

        if ((!aptr->attackdelay) && (aptr->attack))
	{
		aptr->attack--;
		playpositionalfx(aptr->x, aptr->y, FXCHAN_ENEMYSHOOT, SMP_EXPLODE, 20000, 24+(rand()%5));
                alertenemies(aptr->x, aptr->y, SNDDIST_GUN/2);
   		aptr->attackdelay = 5;
		spawnflame(aptr, 5); // ***
	}
	if (!checkvision(aptr)) return;
        a = findangle(aptr->x, aptr->y, actor[0].x, actor[0].y);
        ad = angledist(aptr->angle, a);
	if (abs(ad) > 80) return;

	if ((!aptr->attack) && (!aptr->attackdelay) && (finddist(aptr->x, aptr->y, actor[0].x, actor[0].y) < LEADER_MAXDIST) && ((rand() & 63) >= flamepr[difficulty]))
        {
        	aptr->attack = 10 + (rand()%10);
        }
}

void sadistcontrol(ACTOR *aptr)
{
        if (enemyveryfar(aptr)) return;

        enemyphysrotaccel(aptr, 15);
	if (!aptr->attack)
                enemyphystransaccel(aptr, 5);
        enemyphystransfrict(aptr);
        enemyphysrotmove(aptr);

        enemyanimate(aptr);

        enemyaiifmodepatrol(aptr);
        if (aptr->enemymode == MODE_ATTACK)
        {
                int dist, a, da;

                /* Unobstructed movement? */
		if (domove(aptr))
		{
                        /* "forget player" counter? */
                	if (!aptr->enemycounter)
                	{
                                /* Hunt player */
                                aptr->enemycontrol = 0;
			       	a = findangle(aptr->x, aptr->y, actor[0].x, actor[0].y);
			        da = angledist(aptr->angle, a);
			        if (da < 0) aptr->enemycontrol |= CB_LEFT;
			        if (da > 0) aptr->enemycontrol |= CB_RIGHT;
				if (!checkvision(aptr)) aptr->enemycontrol |= CB_FORWARD;
                                else {
                                        da = angledist(actor[0].angle, a);
                                        dist = finddist(aptr->x, aptr->y, actor[0].x, actor[0].y);
					if (512 - abs(da) - rand() % 40 < difficulty * (196 - dist) / 4) {
                                                if (da > 0) aptr->enemycontrol |= CB_STRAFERIGHT;
                                                if (da < 0) aptr->enemycontrol |= CB_STRAFELEFT;
                                                if (dist > 15 * difficulty) aptr->enemycontrol |= CB_BACKWARD;
                                                else if (dist > 10) aptr->enemycontrol |= CB_FORWARD;
                                        } else {
                                                if (dist < 30) aptr->enemycontrol |= CB_BACKWARD;
                                                if (dist > 120) aptr->enemycontrol |= CB_FORWARD;
                                        }
                                }
			}
			else aptr->enemycontrol = CB_FORWARD;
		}
		else
		{
			/* Forget player for half second, try to find way out */
			if (!aptr->enemycounter)
			{
                                if (!(rand() & 15))
                                {
					aptr->enemycounter = 35;
					if (rand() & 1)
			                aptr->enemycontrol = CB_RIGHT | CB_FORWARD;
			                else
			                aptr->enemycontrol = CB_LEFT | CB_FORWARD;
				}
				else
				{
		                        /* Hunt player */
					aptr->enemycontrol = CB_FORWARD;
					a = findangle(aptr->x, aptr->y, actor[0].x, actor[0].y);
					da = angledist(aptr->angle, a);
					if (da < 0) aptr->enemycontrol |= CB_LEFT;
					if (da > 0) aptr->enemycontrol |= CB_RIGHT;
				}
			}
			else
			{
                                if (!(rand() & 15))
                                {
					if (rand() & 1)
			                aptr->enemycontrol = CB_RIGHT | CB_FORWARD;
			                else
			                aptr->enemycontrol = CB_LEFT | CB_FORWARD;
				}
			}
                }
                if (aptr->enemycounter) aptr->enemycounter--;
        }
        sadistattack(aptr);
}

void sadistattack(ACTOR *aptr)
{
        int a, ad;

        if (aptr->attack) aptr->attack--;
        if (aptr->attackdelay) aptr->attackdelay--;
        if (aptr->enemymode != MODE_ATTACK) return;
	if (!checkvision(aptr))
	{
                aptr->attack = 0;
	        return;
        }

        a = findangle(aptr->x, aptr->y, actor[0].x, actor[0].y);
        ad = angledist(aptr->angle, a);
	if (abs(ad) > 120) return;

	if ((!aptr->attack) && (!aptr->attackdelay) && (finddist(aptr->x, aptr->y, actor[0].x, actor[0].y) < SADIST_MAXDIST) && ((rand() & 63) >= crossbowpr[difficulty]))
        {
                int delay = finddist(aptr->x, aptr->y, actor[0].x, actor[0].y)/2 - difficulty * 3;
                if (delay < 10) delay = 10;
                if (delay > 70) delay = 70;

                aptr->attack = delay;
                aptr->enemycounter = 0; /* Turn towards player but don't move
                                           while aiming */
        }

        if (aptr->attack == 4)
        {
                spawnarrow(aptr, 5);
                playpositionalfx(aptr->x, aptr->y, FXCHAN_ENEMYSHOOT, SMP_CROSSBOW, 22050, 64);
                aptr->attackdelay = attackdelaytbl[WEAP_CROSSBOW];
        }
}

void allpatrol(void)
{
        int c;

        /* When player dies, all enemies go back to patrol mode */
        for (c = 0; c < actors; c++)
        {
        	if ((actor[c].type >= ACTOR_FIRSTENEMY) && (actor[c].type <= ACTOR_LASTENEMY))
        	{
	        	if (actor[c].enemymode == MODE_ATTACK) actor[c].enemymode = MODE_PATROL;
	        }
        }
}

void allattack(void)
{
        int c;
        ACTOR *aptr = &actor[0];

        for (c = 0; c < actors; c++)
        {
        	if ((aptr->type >= ACTOR_FIRSTENEMY) && (aptr->type <= ACTOR_LASTENEMY))
        	{
        		if (!isserverroom(aptr->x, aptr->y))
        		{
		        	if (aptr->enemymode == MODE_PATROL)
		        	{
		        	        aptr->enemymode = MODE_ATTACK;
                                        aptr->enemybored = -1; /* Never give up the hunt */
                                }
		        }
		}
		aptr++;
        }
}

void alertenemies(int x, int y, int maxdist)
{
	int c;
	ACTOR *cptr = &actor[0];
        int modify = (rand() % 32)-16;
        maxdist += modify;
        if (maxdist < 0) maxdist = 0;

	for (c = 0; c < actors; c++)
	{
		if ((cptr->type >= ACTOR_FISTMAN) && (cptr->type <= ACTOR_LEADER))
		{
			if (cptr->enemymode == MODE_PATROL)
			{
				if (finddist(x, y, cptr->x, cptr->y) < maxdist)
				{
					attackplayer_notaunt(cptr);
				}
			}
		}
		cptr++;
	}
}

void detectplayer(ACTOR *aptr)
{
        int a, ad;

        /* If player dead, don't detect */
        if (actor[0].type != ACTOR_BOFH) return;

	if (!checkvision(aptr)) return;
        a = findangle(aptr->x, aptr->y, actor[0].x, actor[0].y);
        ad = angledist(aptr->angle, a);
	if (abs(ad) < 128)
	{
		attackplayer(aptr);
	}
}

void attackplayer(ACTOR *aptr)
{
	/* Don't attack if player dead */
	if (actor[0].type != ACTOR_BOFH) return;
	aptr->enemymode = MODE_ATTACK;
	/* Technicians & leaders don't insult player */
        if (aptr->type == ACTOR_TECHNICIAN) return;
        if (aptr->type == ACTOR_LEADER) return;
        aptr->enemybored = 0;
        playownedpositionalfx(aptr, aptr->x, aptr->y, FXCHAN_SPEECH, SMP_TAUNT1+rand()%6, 11025, 64); // *** !!!
        alertenemies(aptr->x, aptr->y, SNDDIST_SPEECH);
  	speaker = aptr;
}

void attackplayer_notaunt(ACTOR *aptr)
{
	/* Don't attack if player dead */
	if (actor[0].type != ACTOR_BOFH) return;
	aptr->enemymode = MODE_ATTACK;
        aptr->enemybored = 0;
}

void enemyboredom(ACTOR *aptr)
{
        if (aptr->enemymode == MODE_PATROL) return;
        /* If can't see player, becomes bored */
        if (!checkvision(aptr))
        {
                if (aptr->enemybored != -1) /* Never give up */
                {
                        aptr->enemybored++;
                        if (aptr->enemybored >= GIVEUPTIME)
                        {
                                aptr->enemybored = 0;
                                aptr->enemymode = MODE_PATROL;
                        }
                }
        }
        else
        {
                if (aptr->enemybored > 0) aptr->enemybored = 0; /* Reset counter */
        }
}

