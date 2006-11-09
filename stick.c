/*
 *	ROGUE
 *
 *	@(#)stick.c	1.1	Strathclyde	10/31/84
 */

# include "param.h"
# include "types.h"
# include "extern.h"

struct item	Missile;

struct being	*Hit_monst[40];

struct coords	Bolt_pos, Bolt_path[6];

static void drain();


void fix_stick(register struct item *stick)
{
	if(strcmp(Ws_type[stick->i_type],"staff") == 0) {
		stick->i_hpower = "2x3";
	} else {
		stick->i_hpower = "1x1";
	}
	stick->i_tpower = "1x1";
	if(stick->i_type == WS_LIGHT) {
		stick->i_value = rnd(10)+10;
	} else {
		stick->i_value = rnd(5)+3;
	}
}

void do_zap()
{
	register struct item *stick;
	register struct being *monst;
	register int y, x;
	register char *boltname;
	char floorch, oldch;
	struct item *pack;
	if((stick = get_item("zap with",STICK)) == 0) return;
	if(stick->i_item != STICK) {
		After = FALSE;
		msg("you can't zap with that!");
		return;
	}
	if(stick->i_value == 0) {
		msg("nothing happens");
		return;
	}
	switch(stick->i_type) {
	case WS_NOTHING:
		break;
	case WS_LIGHT:
		Ws_info[WS_LIGHT].inf_known = TRUE;
		if(Player.b_room->r_flags & R_PASSAGE) {
			msg("the corridor glows and then fades");
		} else {
			Player.b_room->r_flags &= ~R_DARK;
			enter_room(&ME);
			addmsg("the room is lit");
			if(Verbose) addmsg(" by a shimmering %s light",pick_colour("blue"));
			endmsg();
		}
		break;
	case WS_DRAIN_LIFE:
		if(Player.b_stats.hp < 2) {
			msg("you are too weak to use it");
			return;
		}
		drain();
		break;
	case WS_INVISIBILITY:
	case WS_POLYMORPH:
	case WS_TELEPORT_AWAY:
	case WS_TELEPORT_TO:
	case WS_CANCELLATION:
		y = MYY;
		x = MYX;
		while(step_ok(Places[x][y].p_being ? Places[x][y].p_being->b_app : Places[x][y].p_ch)) {
			y += Delta.y;
			x += Delta.x;
		}
		if((monst = Places[x][y].p_being) != NULL) {
			floorch = monst->b_ch;
			if(floorch == VENUS_FLYTRAP) Player.b_flags &= ~HELD;
			switch(stick->i_type) {
			case WS_INVISIBILITY:
				monst->b_flags |= INVISIBLE;
				if(cansee(y,x)) mvaddch(y,x,monst->b_oldch);
				break;
			case WS_POLYMORPH:
				pack = monst->b_pack;
				_detach(&Mlist,monst);
				if(see_monst(monst)) mvaddch(y,x,Places[x][y].p_ch);
				oldch = monst->b_oldch;
				Delta.y = y;
				Delta.x = x;
				new_monster(monst,floorch = RANDMONST,&Delta);
				if(see_monst(monst)) mvaddch(y,x,floorch);
				monst->b_oldch = oldch;
				monst->b_pack = pack;
				Ws_info[5].inf_known |= see_monst(monst);
				break;
			case WS_CANCELLATION:
				monst->b_flags |= CANCELLED;
				monst->b_flags &= ~(INVISIBLE|MONST_CONFUSION);
				monst->b_app = monst->b_ch;
				if(see_monst(monst)) mvaddch(y,x,monst->b_app);
				break;
			case WS_TELEPORT_AWAY:
			case WS_TELEPORT_TO:
				if(see_monst(monst) || (Player.b_flags & DETECTING)) mvaddch(y,x,monst->b_oldch);
				if(stick->i_type == WS_TELEPORT_AWAY) {
					do find_floor((struct room *)0,&bc(monst),0,TRUE); while(bx(monst) == MYX && by(monst) == MYY);
				} else {
					by(monst) = MYY+Delta.y;
					bx(monst) = MYX+Delta.x;
				}
				Places[x][y].p_being = 0;
				Places[bx(monst)][by(monst)].p_being = monst;
				if(monst->b_ch == VENUS_FLYTRAP) Player.b_flags &= ~HELD;
				if(by(monst) != y || bx(monst) != x) {
					move(by(monst),bx(monst));
					monst->b_oldch = winch(stdscr);
					if(see_monst(monst)) {
						addch(monst->b_app);
					} else if(Player.b_flags & DETECTING) {
						standout();
						addch(monst->b_app);
						standend();
					}
				}
			}
			monst->b_dest = &ME;
			monst->b_flags |= MOBILE;
		}
		break;
	case WS_MAGIC_MISSILE:
		Ws_info[WS_MAGIC_MISSILE].inf_known = TRUE;
		Missile.i_item = '*';
		Missile.i_tpower = "1x4";
		Missile.i_hitplus = 100;
		Missile.i_damplus = 1;
		Missile.i_flags = THROW;
		if(Cur_weapon) Missile.i_uses = Cur_weapon->i_type;
		do_motion(&Missile,Delta.y,Delta.x);
		if((monst = Places[Missile.i_coords.x][Missile.i_coords.y].p_being) && save_throw(3,monst)) {
			 hit_monster(Missile.i_coords.y,Missile.i_coords.x,&Missile);
		} else {
			msg(Terse ? "missile vanishes" : "the missile vanishes with a puff of smoke");
		}
		break;
	case WS_HASTE_MONSTER:
	case WS_SLOW_MONSTER:
		y = MYY;
		x = MYX;
		while(step_ok(Places[x][y].p_being ? Places[x][y].p_being->b_app : Places[x][y].p_ch)) {
			y += Delta.y;
			x += Delta.x;
		}
		if((monst = Places[x][y].p_being) != NULL) {
			if(stick->i_type == WS_HASTE_MONSTER) {
				if(monst->b_flags & SLOW) {
					monst->b_flags &= ~SLOW;
				} else {
					monst->b_flags |= FAST;
				}
			} else {
				if(monst->b_flags & FAST) {
					monst->b_flags &= ~FAST;
				} else {
					monst->b_flags |= SLOW;
				}
				monst->b_moved = TRUE;
			}
			Delta.y = y;
			Delta.x = x;
			runto(&Delta);
		}
		break;
	case WS_LIGHTNING:
	case WS_FIRE:
	case WS_COLD:
		if(stick->i_type == WS_LIGHTNING) {
			boltname = "bolt";
		} else if(stick->i_type == WS_FIRE) {
			boltname = "flame";
		} else {
			boltname = "ice";
		}
		fire_bolt(&ME,&Delta,boltname);
		Ws_info[stick->i_type].inf_known = TRUE;
	}
	stick->i_value--;
}

void drain()
{
	register struct being *monst;
	register struct room *passage;
	register struct being **mlist;
	register int hits;
	bool inpass;
	hits = 0;
	if(Places[MYX][MYY].p_ch == DOOR) {
		passage = &Passages[Places[MYX][MYY].p_flags & PASSNUM];
	} else {
		passage = 0;
	}
	inpass = Player.b_room->r_flags & R_PASSAGE;
	mlist = Hit_monst;
	for(monst = Mlist; monst; monst = monst->b_link) {
		if(
			monst->b_room == Player.b_room ||
			monst->b_room == passage ||
			(inpass && Places[bx(monst)][by(monst)].p_ch == DOOR && Player.b_room == &Passages[Places[bx(monst)][by(monst)].p_flags & PASSNUM])
		) {
			*mlist++ = monst;
		}
	}
	if((hits = mlist-Hit_monst) == 0) {
		msg("you have a tingling feeling");
		return;
	}
	*mlist = 0;
	Player.b_stats.hp /= 2;
	hits = Player.b_stats.hp/hits;
	for(mlist = Hit_monst; *mlist; mlist++) {
		monst = *mlist;
		if((monst->b_stats.hp -= hits) <= 0) {
			killed(monst,see_monst(monst));
		} else {
			runto(&bc(monst));
		}
	}
}

void fire_bolt(struct coords *start, struct coords *delta,char *boltname)
{
	register struct coords *path, *draw_path;
	register struct being *monst;
	char boltch, floorch;
	bool atplayer, bolthit, missed;
	struct item bolt;
	bolt.i_item = WEAPON;
	bolt.i_type = BOLT;
	bolt.i_tpower = "6x6";
	bolt.i_uses = -1;
	bolt.i_hitplus = 100;
	bolt.i_damplus = 0;
	Weap_info[BOLT].inf_name = boltname;
	switch(delta->y+delta->x) {
	case 0:
		boltch = '/';
		break;
	case 1:
	case -1:
		boltch = delta->y == 0 ? '-' : '|';
		break;
	case 2:
	case -2:
		boltch = '\\';
	}
	Bolt_pos = *start;
	atplayer = start != &ME;
	bolthit = FALSE;
	missed = FALSE;
	for(path = Bolt_path; path < &Bolt_path[6] && !bolthit; path++) {
		Bolt_pos.y += delta->y;
		Bolt_pos.x += delta->x;
		*path = Bolt_pos;
		floorch = Places[Bolt_pos.x][Bolt_pos.y].p_being ? Places[Bolt_pos.x][Bolt_pos.y].p_being->b_app : Places[Bolt_pos.x][Bolt_pos.y].p_ch;
		switch(floorch) {
		case DOOR:
			if(MYX == Bolt_pos.x && MYY == Bolt_pos.y) break;
		case ' ':
		case HWALL:
		case VWALL:
			if(!missed) atplayer = !atplayer;
			missed = FALSE;
			delta->y = -delta->y;
			delta->x = -delta->x;
			path--;
			msg("the %s bounces",boltname);
			continue;
		}
		if(!atplayer && (monst = Places[Bolt_pos.x][Bolt_pos.y].p_being)) {
			atplayer = TRUE;
			missed = !missed;
			monst->b_oldch = Places[Bolt_pos.x][Bolt_pos.y].p_ch;
			if(!save_throw(3,monst)) {
				bolt.i_coords = Bolt_pos;
				bolthit = TRUE;
				if(monst->b_ch == DRAGON && strcmp(boltname,"flame") == 0) {
					addmsg("the flame bounces");
					if(Verbose) addmsg("off the dragon");
					endmsg();
				} else {
					hit_monster(Bolt_pos.y,Bolt_pos.x,&bolt);
				}
			} else if(floorch != MEDUSA || monst->b_app == MEDUSA) {
				if(start == &ME) runto(&Bolt_pos);
				if(Terse) {
					msg("%s misses",boltname);
				} else {
					msg("the %s whizzes past %s",boltname,set_mname(monst));
				}
			}
		} else if(atplayer && Bolt_pos.x == MYX && Bolt_pos.y == MYY) {
			atplayer = FALSE;
			missed = !missed;
			if(!save(3)) {
				if((Player.b_stats.hp -= roll(6,6)) <= 0) {
					if(start == &ME) {
						death('b');
					} else {
						death(Places[start->x][start->y].p_being->b_ch);
					}
				}
				bolthit = TRUE;
				if(Terse) {
					msg("the %s hits",boltname);
				} else {
					msg("you are hit by the %s",boltname);
				}
			} else {
				msg("the %s whizzes by you",boltname);
			}
		}
		mvaddch(Bolt_pos.y,Bolt_pos.x,boltch);
		refresh();
	}
	for(draw_path = Bolt_path; draw_path < path; draw_path++) mvaddch(draw_path->y,draw_path->x,Places[draw_path->x][draw_path->y].p_ch);
}

char *charge_str(stick)
	register struct item *stick;
{
	static char buf[20];
	if((stick->i_flags & KNOWN) == 0) {
		buf[0] = 0;
	} else if(Terse) {
		sprintf(buf," [%d]",stick->i_value);
	} else {
		sprintf(buf," [%d charges]",stick->i_value);
	}
	return(buf);
}

