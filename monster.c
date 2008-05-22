/*
 *	ROGUE
 *
 *	@(#)monster.c	1.1	Strathclyde	10/31/84
 */

# include "param.h"
# include "types.h"
# include "extern.h"

char Lvl_mons[] =	"KEBSHIROZLCQANYFTWPXUMVGJD";
char Wand_mons[] =	"KEBSH\0ROZ\0CQA\0Y\0TWP\0UMVGJ\0";

int randmonster(bool wander)
{
	register int choice;
	register char *mons;
	mons = wander ? Wand_mons : Lvl_mons;
	do {
		choice = Level-6+rnd(10);
		if(choice < 0) choice = rnd(5);
		if(choice > 25) choice = 21+rnd(5);
	} while(mons[choice] == 0);
	return(mons[choice]);
}

void new_monster(register struct being *monst, char ch, register struct coords *pos)
{
	register struct monster *minit;
	register int extra_level;
	if((extra_level = Level-26) < 0) extra_level = 0;
	_attach(&Mlist,monst);
	monst->b_ch = ch;
	monst->b_app = ch;
	bc(monst) = *pos;
	move(pos->y,pos->x);
	monst->b_oldch = WINCH(stdscr);
	monst->b_room = roomin(pos);
	Places[pos->x][pos->y].p_being = monst;
	minit = &Monsters[mindex(monst->b_ch)];
	monst->b_stats.level = minit->m_level+extra_level;
	monst->b_stats.hp = roll(monst->b_stats.level,8);
	monst->b_stats.maxhp = monst->b_stats.hp;
	monst->b_stats.protection = minit->m_protection-extra_level;
	monst->b_stats.power = minit->m_power;
	monst->b_stats.strength = minit->m_strength;
	monst->b_stats.experience = minit->m_experience+exp_add(monst)+extra_level*10;
	monst->b_flags = minit->m_flags;
	if(Level > 29) monst->b_flags |= FAST;
	monst->b_moved = TRUE;
	monst->b_pack = 0;
	if(ring(R_AGGRAVATE_MONSTER)) runto(pos);
	if(ch == XEROC) monst->b_app = rnd_thing();
}

int exp_add(register struct being *monst)
{
	register int extra;
	if(monst->b_stats.level == 1) {
		extra = monst->b_stats.maxhp/8;
	} else {
		extra = monst->b_stats.maxhp/6;
	}
	if(monst->b_stats.level > 9) {
		extra *= 20;
	} else if(monst->b_stats.level > 6) {
		extra *= 4;
	}
	return(extra);
}

void wanderer()
{
	register struct being *monst;
	static struct coords position;
	monst = (struct being *)(new_item());
	do find_floor((struct room *)0,&position,0,TRUE); while(roomin(&position) == Player.b_room);
	new_monster(monst,randmonster(TRUE),&position);
	if(Player.b_flags & DETECTING) {
		standout();
		addch((Player.b_flags & HALLUCINATING) == 0 ? monst->b_ch : RANDMONST);
		standend();
	}
	runto(&bc(monst));
}

struct being *wake_monster(int y, int x)
{
	register struct being *monst;
	register struct room *inroom;
	register char *mname;
	char mch;
	if((monst = Places[x][y].p_being) == 0) {
		endwin();
		abort();
	}
	mch = monst->b_ch;
	if(
		(monst->b_flags & MOBILE) == 0 &&
		rnd(3) &&
		(monst->b_flags & AGGRESSIVE) &&
		(monst->b_flags & HELD) == 0 &&
		!ring(R_STEALTH) &&
		(Player.b_flags & LEVITATED) == 0
	) {
		monst->b_dest = &ME;
		monst->b_flags |= MOBILE;
	}
	if(
		mch == MEDUSA &&
		(Player.b_flags & BLIND) == 0 &&
		(Player.b_flags & HALLUCINATING) == 0 &&
		(monst->b_flags & HAS_CONFUSED) == 0 &&
		(monst->b_flags & CANCELLED) == 0 &&
		(monst->b_flags & MOBILE) &&
		(((inroom = Player.b_room) && (inroom->r_flags & R_DARK)) || dist(y,x,MYY,MYX) < 3)
	) {
		monst->b_flags |= HAS_CONFUSED;
		if(!save(3)) {
			if(Player.b_flags & CONFUSED) {
				lengthen(unconfuse,spread(20));
			} else {
				fuse(unconfuse,0,spread(20),spread(2));
			}
			Player.b_flags |= CONFUSED;
			mname = set_mname(monst);
			addmsg("%s",mname);
			if(strcmp(mname,"it") != 0) addmsg("'");
			msg("s gaze has confused you");
		}
	}
	if((monst->b_flags & GUARD_GOLD) && (monst->b_flags & MOBILE) == 0) {
		monst->b_flags |= MOBILE;
		if(Player.b_room->r_value) {
			monst->b_dest = &Player.b_room->r_gold;
		} else {
			monst->b_dest = &ME;
		}
	}
	return(monst);
}

void give_pack(register struct being *monst)
{
	if(Level >= Max_level && rnd(100) < Monsters[mindex(monst->b_ch)].m_take) {
		_attach(&monst->b_pack,new_thing());
	}
}

