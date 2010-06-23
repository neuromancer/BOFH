#include "bofh.h"
#include "sincos.h"
#include "extern.h"
#include "keybinds.h"

char *colorstring[] = {"RED", "GREEN", "BLUE", "YELLOW"};

void drawinstr(int num, int x, int y)
{
        /* First draw the paper */
	gfx_drawsprite(x, y, 0x90001);
	/* Then all texts on it */
        txt_print(x + 5, y + 5, SPR_BLACKFONTS, closet[bomb[num].location].name);

	txt_print(x + 10, y + 20, SPR_BLACKFONTS,colorstring[bomb[num].wireorder[0]]);
	txt_print(x + 10, y + 30, SPR_BLACKFONTS,colorstring[bomb[num].wireorder[1]]);
	txt_print(x + 10, y + 40, SPR_BLACKFONTS,colorstring[bomb[num].wireorder[2]]);
	txt_print(x + 10, y + 50, SPR_BLACKFONTS,colorstring[bomb[num].wireorder[3]]);
	/* If all wires cut, draw an "X" */
	if (!bomb[num].wiresleft) gfx_drawsprite(x, y, 0x90012);
}

void drawnearcloset(void)
{
	int c;
        int xb = actor[0].x >> 12;
        int yb = actor[0].y >> 12;

        if (actor[0].type != ACTOR_BOFH) return;

	for (c = 0; c < numclosets; c++)
	{
        	if ((xb == closet[c].xb) && (yb == closet[c].yb))
        	{
        		drawcloset(c);
        	}
        }
}

void drawcloset(int num)
{
        int c, isbomb = 0;
        int flash = gametime % 70;
        int bombnum = 0;

        /* Is there a bomb */
        for (c = 0; c < numbombs; c++)
        {
        	if (bomb[c].location == num)
        	{
        		isbomb = 1;
        		bombnum = c;
        		/* Nothing can be done when time runs out */
        		if (!gametime) return;
        	}
        }

        /* Draw closet */
	gfx_drawsprite(123, 48, 0x90002);
	/* Draw all network equipment */
	for (c = MAX_EQUIP-1; c >= 0; c--)
	{
		switch(closet[num].equipment[c])
		{
			case EQUIP_RIMA:
			gfx_drawsprite(123, 53 + 10 * c, 0x90003);
			break;

			case EQUIP_HP_HUB:
			gfx_drawsprite(123, 53 + 10 * c, 0x90004 + (rand()&1));
			break;

			case EQUIP_AT_T_HUB:
			gfx_drawsprite(123, 53 + 10 * c, 0x90006 + (rand()&1));
			break;

			case EQUIP_CISCO_SWITCH:
			gfx_drawsprite(123, 53 + 10 * c, 0x90008 + (rand()&1));
			break;

			case EQUIP_RJ45RIMA:
			gfx_drawsprite(123, 53 + 10 * c, 0x9000a);
			break;
		}
	}

	/* Draw name of closet */
        txt_print(128, 52, SPR_SMALLFONTS, closet[num].name);

        /* Is there a bomb? */
        if (isbomb)
        {
                int wc = 4 - bomb[bombnum].wiresleft;
                /* Table of wires still uncutted (order by color) */
                int wt[4] = {0,0,0,0};
                int w = WIRE_NONE;
		if (wc < 4)
		{
			if (bomb[bombnum].instructions) drawinstr(bombnum, 256, 8);
                        txt_printcenter(160, SPR_SMALLFONTS, "PRESS R,G,B,Y TO CUT WIRES");
		}
		else
		{
			flash = 10; /* When all wires cutted, timer stops */
                        txt_printcenter(160, SPR_SMALLFONTS, "BOMB DISARMED");
		}

		/* Flashing on the bomb timer */
                if (flash >= 10) flash = 10;
		gfx_drawsprite(135, 132, 0x9000b + flash / 5);

                /* Which color wires still exist? */
                if (wc < 1) wt[bomb[bombnum].wireorder[0]] = 1;
                if (wc < 2) wt[bomb[bombnum].wireorder[1]] = 1;
                if (wc < 3) wt[bomb[bombnum].wireorder[2]] = 1;
                if (wc < 4) wt[bomb[bombnum].wireorder[3]] = 1;
                if (wt[WIRE_RED]) gfx_drawsprite(135,132,0x9000e);
                if (wt[WIRE_GREEN]) gfx_drawsprite(135,132,0x9000f);
                if (wt[WIRE_BLUE]) gfx_drawsprite(135,132,0x90010);
                if (wt[WIRE_YELLOW]) gfx_drawsprite(135,132,0x90011);

		/* Cutting a wire */
                if (wc < 4)
                {
	                if ((kbd_checkkey(redkey)) && (wt[WIRE_RED])) w = WIRE_RED;
	                if ((kbd_checkkey(greenkey)) && (wt[WIRE_GREEN])) w = WIRE_GREEN;
	                if ((kbd_checkkey(bluekey)) && (wt[WIRE_BLUE])) w = WIRE_BLUE;
	                if ((kbd_checkkey(yellowkey)) && (wt[WIRE_YELLOW])) w = WIRE_YELLOW;
	                if (w != WIRE_NONE)
	                {
		                playfx(FXCHAN_KLONK, SMP_UZI, 30000, 32, 128);
	                	if (w == bomb[bombnum].wireorder[wc])
	                	{
	                		/* Correct wire */
	                		reward(1500);
	                		bomb[bombnum].wiresleft--;
	                		if (!bomb[bombnum].wiresleft)
	                		{
                                                reward(4000);
	                			bombs--;

	                			/* Extra bonus for courage */
	                			if (!bomb[bombnum].instructions)
	                			{
	                				printgamemsg("HUH!", 200);
	                				reward(10000);
	                			}

	                			/* If it was the last bomb, give instructions */
	                			if (!bombs)
	                			{
	                				printgamemsg("NOW GO TO THE SERVER ROOM!", 200);
                                                        /* All remaining terrorists outside the server room attack */
	                				allattack();
	                			}
                                        }
	                	}
	                	else
	                	{
                                        /* Incorrect wire :-) */
                                        gametime = 0;
			        	printgamemsg("SHIT, WRONG WIRE!", 200);
		                }
	                }
		}
	}
}

void countdown(void)
{
        /* Decrease time if bombs left */
        if (((gametime) && (bombs)) && (!timecheat)) gametime--;
	/* Check running out of time */
        if ((!gametime) && (bombs))
        {
                /* Shifting of screen when explosions happen */
        	xshift = (rand()&3)-2;
        	yshift = (rand()&3)-2;
        	if (!gameover) gameover++;
        	if (!(rand() & 1))
        	{
                        /* Spawn explosion at a random closet */
                       	int bombnum = (rand() % numbombs);

                       	if (bomb[bombnum].wiresleft)
                       	{
                               	int c;
                               	int explx;
                               	int exply;
                               	ACTOR *sptr;

                                /* Get a position that doesn't contain a wall */
                        	for (;;)
                        	{
                               	 	explx = (closet[bomb[bombnum].location].xb*16+(rand()&15))*DEC;
                               	 	exply = (closet[bomb[bombnum].location].yb*16+(rand()&15))*DEC;
                               	 	if (!(map_getblockinfo(0, explx/DEC, exply/DEC)|map_getblockinfo(1, explx/DEC, exply/DEC))
                               	 		&& (INF_OBSTACLE|INF_WALL)) break;
                               	}

                               	/* Sound not always */
                        	sptr = spawnactor(ACTOR_EXPLOSION, explx, exply, rand()&0x3ff);
        			if (sptr)
        			{
        				/* Spawn chunks only if the player will see them */
        				if ((abs(sptr->x - actor[0].x) < 200*DEC) &&
        				   (abs(sptr->y - actor[0].y) < 200*DEC))
        				{
		                               	int pieces = (rand()%4)+2;
		                               	/* Near explosion always causes sound */
	                       			playpositionalfx(explx, exply, FIRSTFXCHAN+(rand()&7), SMP_EXPLODE, 20000+(rand()&2047),64);
	        				for (c = 0; c < pieces; c++)
	        				{
        		                        	spawnchunk(sptr, DMG_BOMBCHUNK);
        		                        }
        				}
        				else
        				{
                                                /* Far explosion sometimes causes sound */
        					if (!(rand()%10)) playpositionalfx(explx, exply, FIRSTFXCHAN+(rand()&7), SMP_EXPLODE, 20000+(rand()&2047),64);
					}
        			}
        		}
               }
        }
        else
        {
        	xshift = 0;
        	yshift = 0;
        }
}

