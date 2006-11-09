/*
 *	ROGUE
 *
 *	@(#)move.c	1.1	Strathclyde	10/31/84
 */

# include "param.h"
# include "types.h"
# include "extern.h"

int save_throw(int v, struct being *b)
{
	register int vulnerable;
	vulnerable = v+20-b->b_stats.level/2;
	return(roll(1,20) >= vulnerable);
}

int save(register int v)
{
	if(v == 3) {
		if(lring(R_PROTECTION)) v -= Cur_ring[LEFT]->i_value;
		if(rring(R_PROTECTION)) v -= Cur_ring[RIGHT]->i_value;
	}
	return(save_throw(v,&Player));
}

void do_run(int dir)
{
	Running = TRUE;
	After = FALSE;
	Runch = dir;
}

void do_move(int yinc,int xinc)
{
	char floorch;
	unsigned char flags;
	bool turn1, turn2;

	Firstmove = FALSE;
	if(No_move) {
		No_move--;
		msg("you are still stuck in the bear trap");
		return;
	}
	if((Player.b_flags & CONFUSED) && rnd(5)) {
		nh = *(rndmove(&Player));
		if(nh.x == MYX && nh.y == MYY) {
			After = FALSE;
			Running = FALSE;
			To_death = FALSE;
			return;
		}
	} else {
retry:
		nh.y = MYY+yinc;
		nh.x = MYX+xinc;
	}
	if(nh.x >= 0 && nh.x < 80 && nh.y > 0 && nh.y < 23) {
		if(!diag_ok(&ME,&nh)) {
			After = FALSE;
			Running = FALSE;
			return;
		}
		if(Running && MYX == nh.x && MYY == nh.y) After = Running = FALSE;
		flags = Places[nh.x][nh.y].p_flags;
		if(Places[nh.x][nh.y].p_being) {
			floorch = Places[nh.x][nh.y].p_being->b_app;
		} else {
			floorch = Places[nh.x][nh.y].p_ch;
		}
		if((flags & P_KNOWN) == 0 && floorch == FLOOR) {
			if((Player.b_flags & LEVITATED) == 0) {
				Places[nh.x][nh.y].p_ch = floorch = TRAP;
				Places[nh.x][nh.y].p_flags |= P_KNOWN;
			}
		} else if((Player.b_flags & HELD) && floorch != VENUS_FLYTRAP) {
			msg("you are being held");
			return;
		}
		switch(floorch) {
		case ' ':
		case HWALL:
		case VWALL:
			goto badmove;
		case DOOR:
			Running = FALSE;
			if(Places[MYX][MYY].p_flags & P_PASSAGE) enter_room(&nh);
			break;
		case PASSAGE:
			Player.b_room = roomin(&ME);
			break;
		case TRAP:
			if((floorch = be_trapped(&nh)) == 0 || floorch == 4) return;
			break;
		case FLOOR:
			if((flags & P_KNOWN) == 0) be_trapped(&ME);
			break;
		case STAIRCASE:
			Seenstairs = TRUE;
		default:
			Running = FALSE;
			if(isupper(floorch) || Places[nh.x][nh.y].p_being) {
				fight(&nh,Cur_weapon,FALSE);
				return;
			}
			if(floorch != STAIRCASE) Take = floorch;
		}
		mvaddch(MYY,MYX,floor_at());
		if((flags & P_PASSAGE) && Places[Oldpos.x][Oldpos.y].p_ch == DOOR) leave_room(&nh);
		Player.b_coords = nh;
		return;
	}
badmove:
	if(Passgo && Running && (Player.b_room->r_flags & R_PASSAGE) && (Player.b_flags & BLIND) == 0) {
		switch(Runch) {
		case 'h':
		case 'l':
			turn1 = MYY != 1 && turn_ok(MYY-1,MYX);
			turn2 = MYY != 22 && turn_ok(MYY+1,MYX);
			if(turn1 ^ turn2) {
				if(turn1) {
					Runch = 'k';
					yinc = -1;
				} else {
					Runch = 'j';
					yinc = 1;
				}
				xinc = 0;
				turnref();
				goto retry;
			}
			break;
		case 'j':
		case 'k':
			turn1 = MYX != 0 && turn_ok(MYY,MYX-1);
			turn2 = MYX != 79 && turn_ok(MYY,MYX+1);
			if(turn1 ^ turn2) {
				if(turn1) {
					Runch = 'h';
					xinc = -1;
				} else {
					Runch = 'l';
					xinc = 1;
				}
				yinc = 0;
				turnref();
				goto retry;
			}
		}
	}
	Running = FALSE;
	After = FALSE;
}

int turn_ok(int y, int x)
{
	register struct place *p;
	p = &Places[x][y];
	return(p->p_ch == DOOR || (p->p_flags & (P_PASSAGE|P_KNOWN)) == (P_PASSAGE|P_KNOWN));
}

void turnref()
{
	register struct place *p;
	p = &Places[MYX][MYY];
	if((p->p_flags & P_DISCOVERED) == 0) {
		if(Jump) {
			leaveok(stdscr,TRUE);
			refresh();
			leaveok(stdscr,FALSE);
		}
		p->p_flags |= P_DISCOVERED;
	}
}

void door_open(register struct room *r)
{
	register int y, x;
	if(r->r_flags & R_PASSAGE) return;
	for(y = r->r_ystart; y < r->r_ystart+r->r_ysize; y++) {
		for(x = r->r_xstart; x < r->r_xstart+r->r_xsize; x++) {
			if(isupper(Places[x][y].p_being ? Places[x][y].p_being->b_app : Places[x][y].p_ch)) wake_monster(y,x);
		}
	}
}

int be_trapped(register struct coords *pos)
{
	register struct place *snare;
	register struct item *arrow;
	char trap;
	if(Player.b_flags & LEVITATED) return(RUST_TRAP);
	Running = FALSE;
	Count = 0;
	snare = &Places[pos->x][pos->y];
	snare->p_ch = TRAP;
	trap = snare->p_flags & TMASK;
	snare->p_flags |= P_DISCOVERED;
	switch(trap) {
	case TRAPDOOR:
		Level++;
		new_level();
		msg("you fell into a trap!");
		break;
	case BEAR_TRAP:
		No_move += spread(3);
		msg("you are caught in a bear trap");
		break;
	case SLEEPING_GAS_TRAP:
		No_command += spread(5);
		Player.b_flags &= ~MOBILE;
		msg("a strange white mist envelops you and you fall asleep");
		break;
	case ARROW_TRAP:
		if(swing(Player.b_stats.level-1,Player.b_stats.protection,1)) {
			if((Player.b_stats.hp -= roll(1,6)) <= 0) {
				msg("an arrow killed you");
				death('a');
			} else {
				msg("oh no! An arrow shot you");
			}
		} else {
			arrow = new_item();
			init_weapon(arrow,ARROW);
			arrow->i_number = 1;
			arrow->i_coords = Player.b_coords;
			fall(arrow,FALSE);
			msg("an arrow shoots past you");
		}
		break;
	case TELEPORT_TRAP:
		teleport();
		mvaddch(pos->y,pos->x,TRAP);
		break;
	case POISON_DART_TRAP:
		if(swing(Player.b_stats.level+1,Player.b_stats.protection,1)) {
			if((Player.b_stats.hp -= roll(1,4)) <= 0) {
				msg("a poisoned dart killed you");
				death('d');
			}
			if(!ring(R_SUSTAIN_STRENGTH) && !save(0)) chg_str(-1);
			msg("a small dart just hit you in the shoulder");
		} else {
			msg("a small dart whizzes by your ear and vanishes");
		}
		break;
	case RUST_TRAP:
		msg("a gush of water hits you on the head");
		rust_armor(Cur_armor);
	}
	if (Fight_flush) flush_type();
	return(trap);
}

struct coords *rndmove(struct being *b)
{
	register struct item *thing;
	register int x, y;
	char floorch;
	static struct coords rndpos;

	rndpos.y = rnd(3)+by(b)-1;
	y = rndpos.y;
	rndpos.x = rnd(3)+bx(b)-1;
	x = rndpos.x;
	if(y == by(b) && x == bx(b)) return(&rndpos);
	if(diag_ok(&bc(b),&rndpos)) {
		floorch = Places[x][y].p_being ? Places[x][y].p_being->b_app : Places[x][y].p_ch;
		if(step_ok(floorch)) {
			if(floorch != SCROLL) return(&rndpos);
			for(thing = Lvl_obj; thing; thing = thing->i_link) {
				if(y == thing->i_coords.y && x == thing->i_coords.x) break;
			}
			if(thing == 0 || thing->i_type != S_SCARE_MONSTER) return(&rndpos);
		}
	}
	rndpos = bc(b);
	return(&rndpos);
}

void rust_armor(register struct item *armor)
{
	if(armor == 0 || armor->i_item != ARMOUR || armor->i_type == 0 || armor->i_value >= 0x9) return;
	if((armor->i_flags & PROTECTED) || ring(R_MAINTAIN_ARMOUR)) {
		if(!To_death) msg("the rust vanishes instantly");
		return;
	}
	armor->i_value++;
	if(Verbose) {
		msg("your armor appears to be weaker now. Oh my!");
	} else {
		msg("your armor weakens");
	}
}

