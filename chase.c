/*
 *	ROGUE
 *
 *	@(#)chase.c	1.1	Strathclyde	10/31/84
 */

# include "param.h"
# include "types.h"
# include "extern.h"

static int move_monst(register struct being *runner);
static int do_chase(register struct being *runner);
static void set_oldch(register struct being *monst,
		      register struct coords *c);

static int chase(struct being *runner,
		 struct coords *c);

void waste_time()
{
	do_daemons(spread(1));
	do_fuses(spread(1));
	do_daemons(spread(2));
	do_fuses(spread(2));
}

/* ARGSUSED */
void runners(int junk)
{
	register struct being *runner, *next;
	bool engaged;
	static struct coords oldpos;
	for(runner = Mlist; runner; runner = next) {
	    next = runner->b_link;
		if((runner->b_flags & FEARS_LIGHT) && (runner->b_room->r_flags & R_DARK) == 0) {
			runner->b_flags |= MOBILE;
			runner->b_dest = find_dest(runner);
		}
		if((runner->b_flags & HELD) == 0 && (runner->b_flags & MOBILE)) {
			oldpos = bc(runner);
			engaged = (runner->b_flags & FIGHTING) != 0;
			if (move_monst(runner)) continue;
			if(runner->b_flags & FAST_MOVE) {
				if(dist_cp(&ME,&bc(runner)) >= 3) {
					move_monst(runner);
				}
			}
			if(engaged && cne(oldpos,bc(runner))) {
				runner->b_flags &= ~FIGHTING;
				To_death = FALSE;
			}
		}
	}
	if(Has_hit) {
		endmsg();
		Has_hit = FALSE;
	}
}

static int move_monst(register struct being *runner)
{
    if((runner->b_flags & SLOW) == 0 || runner->b_moved) {
	if (do_chase(runner)) return 1;
    }
    if(runner->b_flags & FAST) {
	if (do_chase(runner)) return 1;
    }
    runner->b_moved ^= TRUE;
    return 0;
}

int do_chase(register struct being *runner)
{
	register struct coords *door;
	register struct room *inroom, *destroom;
	register int closest = 32767, dist;
	bool arrived = FALSE, atdoor;
	struct item *taken;
	struct room *oldroom;
	static struct coords make_for;
	inroom = runner->b_room;
	if((runner->b_flags & GUARD_GOLD) && inroom->r_value == 0) runner->b_dest = &ME;
	if(runner->b_dest == &ME) {
		destroom = Player.b_room;
	} else {
		destroom = roomin(runner->b_dest);
	}
	atdoor = Places[bx(runner)][by(runner)].p_ch == DOOR;
	for(;;) {
		if(inroom == destroom) {
			make_for = *(runner->b_dest);
			if(runner->b_ch == DRAGON) {
				if(
					(by(runner) == MYY || bx(runner) == MYX || abs(bx(runner)-MYX) == abs(by(runner)-MYY)) &&
					dist_cp(&bc(runner),&ME) <= DRAGON_RANGE &&
					(runner->b_flags & CANCELLED) == 0 &&
					rnd(5) == 0
				) {
					Delta.y = sign(MYY-by(runner));
					Delta.x = sign(MYX-bx(runner));
					if(Has_hit) endmsg();
					fire_bolt(&bc(runner),&Delta,"flame");
					Running = FALSE;
					Count = 0;
					Quiet = 0;
					if(To_death && (runner->b_flags & FIGHTING) == 0) {
						To_death = FALSE;
						Kamikaze = FALSE;
					}
					return 0;
				}
			}
			break;
		}
		for(door = inroom->r_doors; door < &inroom->r_doors[inroom->r_dct]; door++) {
			dist = dist_cp(runner->b_dest,door);
			if(dist < closest) {
				make_for = *door;
				closest = dist;
			}
		}
		if(!atdoor) break;
		inroom = &Passages[Places[bx(runner)][by(runner)].p_flags & PASSNUM];
		atdoor = FALSE;
	}
	if(chase(runner,&make_for)) {
		if(runner->b_ch == VENUS_FLYTRAP) return 0;
	} else {
		if(ceq(Ch_ret,ME)) {
		    return attack(runner);
		}
		if(make_for.x == runner->b_dest->x && make_for.y == runner->b_dest->y) {
			for(taken = Lvl_obj; taken; taken = taken->i_link) {
				if(runner->b_dest == &taken->i_coords) {
					_detach(&Lvl_obj,taken);
					_attach(&runner->b_pack,taken);
					Places[taken->i_coords.x][taken->i_coords.y].p_ch = (runner->b_room->r_flags & R_PASSAGE ? PASSAGE : FLOOR);
					runner->b_dest = find_dest(runner);
					break;
				}
			}
			if(runner->b_ch != VENUS_FLYTRAP) arrived = TRUE;
		}
	}
	if(cne(Ch_ret,bc(runner))) {
		mvaddch(by(runner),bx(runner),runner->b_oldch);
		set_oldch(runner,&Ch_ret);
		oldroom = runner->b_room;
		runner->b_room = roomin(&Ch_ret);
		if(oldroom != runner->b_room) {
			runner->b_dest = find_dest(runner);
		}
		Places[bx(runner)][by(runner)].p_being = 0;
		Places[Ch_ret.x][Ch_ret.y].p_being = runner;
		bc(runner) = Ch_ret;
	}
	move(Ch_ret.y,Ch_ret.x);
	if(see_monst(runner)) {
		addch(runner->b_app);
	} else if(Player.b_flags & DETECTING) {
		standout();
		addch(runner->b_ch);
		standend();
	}
	if(arrived && ceq(bc(runner),*(runner->b_dest))) runner->b_flags &= ~MOBILE;
	return 0;
}

void set_oldch(register struct being *monst,
	       register struct coords *c)
{
	register int oldoldch;
	move(c->y,c->x);
	oldoldch = monst->b_oldch;
	monst->b_oldch = WINCH(stdscr);
	if((Player.b_flags & BLIND) == 0) {
		if((oldoldch == FLOOR || monst->b_oldch == FLOOR) && (monst->b_room->r_flags & R_DARK)) {
			monst->b_oldch = ' ';
		} else if(dist_cp(c,&ME) <= 3 && See_floor) {
			monst->b_oldch = Places[Ch_ret.x][Ch_ret.y].p_ch;
		}
	}
}

int see_monst(register struct being *monst)
{
	register int y, x;
	if(Player.b_flags & BLIND) return(FALSE);
	if((monst->b_flags & INVISIBLE) && (Player.b_flags & SEE_INVISIBLE) == 0) return(FALSE);
	y = by(monst);
	x = bx(monst);
	if(dist(y,x,MYY,MYX) < 3) {
		if(y == MYY || x == MYX) return(TRUE);
		if(step_ok(Places[MYX][y].p_ch)) return(TRUE);
		if(step_ok(Places[x][MYY].p_ch)) return(TRUE);
		return(FALSE);
	}
	if(monst->b_room != Player.b_room) return(FALSE);
	return((monst->b_room->r_flags & R_DARK) == FALSE);
}

void runto(register struct coords *c)
{
	register struct being *monst;
	monst = Places[c->x][c->y].p_being;
	monst->b_flags |= MOBILE;
	monst->b_flags &= ~HELD;
	monst->b_dest = find_dest(monst);
}

int chase(struct being *runner,
      struct coords *c)
{
	register struct item *i;
	register struct being *monst;
	register int x, y, closest, trydist;
	register struct coords *mpos;
	char ch;
	int rcount, ylimit, xlimit;
	bool atdoor;
	static struct coords try;
	mpos = &bc(runner);
	rcount = 1;
	if(
		((runner->b_flags & CONFUSED) && rnd(5)) ||
		(runner->b_ch == PHANTOM && rnd(5) == 0) ||
		(runner->b_ch == BAT && rnd(2) == 0)
	) {
		Ch_ret = *(rndmove(runner));
		closest = dist_cp(&Ch_ret,c);
		if(rnd(20) == 0) runner->b_flags &= ~CONFUSED;
	} else {
		atdoor = Places[mpos->x][mpos->y].p_ch == DOOR;
		Ch_ret = *mpos;
		closest = dist_cp(mpos,c);
		ylimit = mpos->y+1;
		if(ylimit >= MAXY) ylimit = MAXY-1;
		xlimit = mpos->x+1;
		if(xlimit >= MAXX) xlimit = MAXX-1;
		for(x = mpos->x-1; x <= xlimit; x++) if(x >= 0) {
			try.x = x;
			for(y = mpos->y-1; y <= ylimit; y++) {
				try.y = y;
				if(diag_ok(mpos,&try)) {
					ch = (Places[x][y].p_being ? Places[x][y].p_being->b_app : Places[x][y].p_ch);
					if(step_ok(ch)) {
						if(
							atdoor &&
							(runner->b_flags & FEARS_LIGHT) &&
							(roomin(&try)->r_flags & R_DARK) == 0
						) continue;
						if(ch == SCROLL) {
							for(i = Lvl_obj; i; i = i->i_link) {
								if(y == i->i_coords.y && x == i->i_coords.x) break;
							}
							if(i && i->i_type == S_SCARE_MONSTER) continue;
						}
						if((monst = Places[x][y].p_being) && monst->b_ch == XEROC) continue;
						trydist = dist(y,x,c->y,c->x);
						if(trydist < closest) {
							rcount = 1;
						} else {
							if(trydist != closest) continue;
							rcount++;
							if(rnd(rcount)) continue;
						}
						Ch_ret = try;
						closest = trydist;
					}
				}
			}
		}
	}
	if(closest == 0) return(FALSE);
	if(Ch_ret.x == MYX && Ch_ret.y == MYY) return(FALSE);
	return(TRUE);
}

struct room *roomin(register struct coords *c)
{
	register struct room *r;
	register unsigned char *flags = &Places[c->x][c->y].p_flags;
	if(*flags & P_PASSAGE) return(&Passages[*flags&PASSNUM]);
	for(r = Rooms; r < &Rooms[ROOMS]; r++) {
		if(c->x > r->r_xstart+r->r_xsize) continue;
		if(r->r_xstart > c->x) continue;
		if(c->y > r->r_ystart+r->r_ysize) continue;
		if(r->r_ystart > c->y) continue;
		return(r);
	}
	msg("in some bizarre place (%d, %d)",c->y,c->x);
	return(0);
}

int diag_ok(register struct coords *from, struct coords *to)
{
	if(to->x < 0 || to->x >= MAXX || to->y <= 0 || to->y >= MAXY) return(FALSE);
	if(to->x == from->x || to->y == from->y) return(TRUE);
	if(step_ok(Places[from->x][to->y].p_ch)) {
		if(step_ok(Places[to->x][from->y].p_ch)) return(TRUE);
	}
	return(FALSE);
}

int cansee(register int y, int x)
{
	register struct room *r;
	static struct coords there;
	if(Player.b_flags & BLIND) return(FALSE);
	if(dist(y,x,MYY,MYX) < 3) {
		if(Places[x][y].p_flags & P_PASSAGE) {
			if(y == MYY || x == MYX) return(TRUE);
			if(step_ok(Places[MYX][y].p_ch) || step_ok(Places[x][MYY].p_ch)) return(TRUE);
			return(FALSE);
		}
		return(TRUE);
	}
	there.y = y;
	there.x = x;
	if((r = roomin(&there)) == Player.b_room) {
		return((r->r_flags & R_DARK) == 0);
	}
	return(FALSE);
}

struct coords *find_dest(register struct being *monst)
{
	register struct item *i;
	register int prob;
	register struct coords *door;
	register struct room *mroom;
	register int dist;
	int closest = 32767;
	struct coords *dest;
	if((monst->b_flags & FEARS_LIGHT) && (monst->b_room->r_flags & R_DARK) == 0) {
		if(Places[bx(monst)][by(monst)].p_ch == DOOR) return(&ME);
		dest = 0;
		mroom = monst->b_room;
		for(door = mroom->r_doors; door < &mroom->r_doors[mroom->r_dct]; door++) {
			dist = dist_cp(&monst->b_coords,door);
			if(dist < closest) {
				dest = door;
				closest = dist;
			}
		}
		if(dest) return(dest);
	}
	if((prob = Monsters[mindex(monst->b_ch)].m_take) <= 0 || monst->b_room == Player.b_room || see_monst(monst)) return(&ME);
	for(i = Lvl_obj; i; i = i->i_link) {
		if(i->i_item == SCROLL && i->i_type == S_SCARE_MONSTER) continue;
		if(roomin(&i->i_coords) == monst->b_room && rnd(100) < prob) {
			for(monst = Mlist; monst; monst = monst->b_link) {
				if(monst->b_dest == &i->i_coords) break;
			}
			if(monst == 0) return(&i->i_coords);
		}
	}
	return(&ME);
}

int dist(int y1, int x1, int y2, int x2)
{
	return((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
}

int dist_cp(register struct coords *a, struct coords *b)
{
	return(dist(a->y,a->x,b->y,b->x));
}

