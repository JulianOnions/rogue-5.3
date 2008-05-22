/*
 *	ROGUE
 *
 *	@(#)potion.c	1.1	Strathclyde	10/31/84
 */

# include "param.h"
# include "types.h"
# include "extern.h"

struct pact {
	int	p_flag;
	void	(*p_fuse)();
	int	p_spread;
	char	*p_hstr;
	char	*p_nstr;
} P_actions[] = {
{	CONFUSED,	unconfuse,	20,
		"what a trippy feeling!",
		"wait, what's going on here. Huh? What? Who?"},
{	HALLUCINATING,	come_down,	850,
		"Oh, wow!  Everything seems so cosmic!",
		"Oh, wow!  Everything seems so cosmic!"},
{	0,		0,		0,
		0,
		0},
{	0,		0,		0,
		0,
		0},
{	SEE_INVISIBLE,	unsee,		850,
		Prbuf,
		Prbuf},
{	0,		0,		0,
		0,
		0},
{	0,		0,		0,
		0,
		0},
{	0,		0,		0,
		0,
		0},
{	0,		0,		0,
		0,
		0},
{	0,		0,		0,
		0,
		0},
{	0,		0,		0,
		0,
		0},
{	0,		0,		0,
		0,
		0},
{	BLIND,		sight,		850,
		"oh, bummer!  Everything is dark!  Help!",
		"a cloak of darkness falls around you"},
{	LEVITATED,	land,		30,
		"oh, wow!  You're floating in the air!",
	    "you start to float in the air"}
};

void fuse_turn_see(int arg)
{
    turn_see(arg);
}

void quaff()
{
	register struct item *potion, *thing;
	register struct being *monst;
	bool one = FALSE, seen, hallucinating;
	if((potion = get_item("quaff",POTION)) == 0) return;
	if(potion->i_item != POTION) {
		msg(Verbose ? "yuk! Why would you want to drink that?" : "that's undrinkable");
		return;
	}
	if(potion == Cur_weapon) Cur_weapon = 0;
	hallucinating = (Player.b_flags & HALLUCINATING) != 0;
	one = potion->i_number == 1;
	leave_pack(potion,FALSE,FALSE);
	switch(potion->i_type) {
	case P_CONFUSION:
		do_pot(P_CONFUSION,!hallucinating);
		break;
	case P_POISON:
		Pot_info[2].inf_known = TRUE;
		if(ring(R_SUSTAIN_STRENGTH)) {
			msg("you feel momentarily sick");
		} else {
			chg_str(-(rnd(3)+1));
			msg("you feel very sick now");
			come_down();
		}
		break;
	case P_HEALING:
		Pot_info[P_HEALING].inf_known = TRUE;
		Player.b_stats.hp += roll(Player.b_stats.level,4);
		if(Player.b_stats.hp > Player.b_stats.maxhp) Player.b_stats.hp = ++Player.b_stats.maxhp;
		sight();
		msg("you begin to feel better");
		break;
	case P_GAIN_STRENGTH:
		Pot_info[P_GAIN_STRENGTH].inf_known = TRUE;
		chg_str(1);
		msg("you feel stronger, now.  What bulging muscles!");
		break;
	case P_MONSTER_DETECTION:
		if(Player.b_flags & DETECTING) {
			lengthen(fuse_turn_see,20);
		} else {
			fuse(fuse_turn_see,TRUE,20,spread(2));
		}
		if(!turn_see(FALSE)) msg("you have a %s feeling for a moment, then it passes",choose_str("normal","strange"));
		break;
	case P_MAGIC_DETECTION:
		seen = FALSE;
		if(Lvl_obj) {
			wclear(Hw);
			for(thing = Lvl_obj; thing; thing = thing->i_link) {
				if(is_magic(thing)) {
					seen = TRUE;
					wmove(Hw,thing->i_coords.y,thing->i_coords.x);
					waddch(Hw,'$');
					Pot_info[P_MAGIC_DETECTION].inf_known = TRUE;
				}
			}
			for(monst = Mlist; monst; monst = monst->b_link) {
				for(thing = monst->b_pack; thing; thing = thing->i_link) {
					if(is_magic(thing)) {
						seen = TRUE;
						wmove(Hw,by(monst),bx(monst));
						waddch(Hw,'$');
					}
				}
			}
		}
		if(seen) {
			Pot_info[P_MAGIC_DETECTION].inf_known = TRUE;
			show_win("You sense the presence of magic on this level.--More--");
		} else {
			msg("you have a %s feeling for a moment, then it passes",choose_str("normal","strange"));
		}
		break;
	case P_HALLUCINATION:
		if(!hallucinating) {
			if(Player.b_flags & DETECTING) turn_see(FALSE);
			add_daemon(visuals,0,spread(1));
			Seenstairs = seen_stairs();
		}
		do_pot(P_HALLUCINATION,TRUE);
		break;
	case P_SEE_INVISIBLE:
		sprintf(Prbuf,"this potion tastes like %s juice",Fruit);
		seen = (Player.b_flags & SEE_INVISIBLE) != 0;
		do_pot(P_SEE_INVISIBLE,FALSE);
		if(!seen) invis_on();
		sight();
		break;
	case P_RAISE_LEVEL:
		Pot_info[P_RAISE_LEVEL].inf_known = TRUE;
		msg("you suddenly feel much more skillful");
		raise_level();
		break;
	case P_EXTRA_HEALING:
		Pot_info[P_EXTRA_HEALING].inf_known = TRUE;
		Player.b_stats.hp += roll(Player.b_stats.level,8);
		if(Player.b_stats.hp > Player.b_stats.maxhp) {
			if(Player.b_stats.hp > Player.b_stats.maxhp+Player.b_stats.level+1) Player.b_stats.maxhp++;
			Player.b_stats.hp = ++Player.b_stats.maxhp;
		}
		sight();
		come_down();
		msg("you begin to feel much better");
		break;
	case P_HASTE_SELF:
		Pot_info[P_HASTE_SELF].inf_known = TRUE;
		After = FALSE;
		if(add_haste(1)) msg("you feel yourself moving much faster");
		break;
	case P_RESTORE_STRENGTH:
		if(lring(R_ADD_STRENGTH)) add_str(&Player.b_stats.strength,-Cur_ring[LEFT]->i_value);
		if(rring(R_ADD_STRENGTH)) add_str(&Player.b_stats.strength,-Cur_ring[RIGHT]->i_value);
		if(Player.b_stats.strength < Max_stats.strength) Player.b_stats.strength = Max_stats.strength;
		if(lring(R_ADD_STRENGTH)) add_str(&Player.b_stats.strength,Cur_ring[LEFT]->i_value);
		if(rring(R_ADD_STRENGTH)) add_str(&Player.b_stats.strength,Cur_ring[RIGHT]->i_value);
		msg("hey, this tastes great.  It make you feel warm all over");
		break;
	case P_BLINDNESS:
		do_pot(P_BLINDNESS,TRUE);
		break;
	case P_LEVITATION:
		do_pot(P_LEVITATION,TRUE);
	}
	status();
	call_it(&Pot_info[potion->i_type]);
	if(one) discard(potion);
}

int is_magic(register struct item *thing)
{
	switch(thing->i_item) {
	case ARMOUR:
		return((thing->i_flags & PROTECTED) || thing->i_value != A_class[thing->i_type]);
	case WEAPON:
		return(thing->i_hitplus || thing->i_damplus);
	case POTION:
	case AMULET:
	case STICK:
	case RING:
	case SCROLL:
		return(TRUE);
	default:
		return(FALSE);
	}
}

void invis_on()
{
	register struct being *monst;
	Player.b_flags |= SEE_INVISIBLE;
	for(monst = Mlist; monst; monst = monst->b_link) {
		if((monst->b_flags & INVISIBLE) && see_monst(monst) && (Player.b_flags & HALLUCINATING) == 0) {
			mvaddch(by(monst),bx(monst),monst->b_app);
		}
	}
}

int turn_see(bool off)
{
	register struct being *monst;
	bool visible, detected = FALSE;
	for(monst = Mlist; monst; monst = monst->b_link) {
		move(by(monst),bx(monst));
		visible = see_monst(monst);
		if(off) {
			if(visible) continue;
			addch(monst->b_oldch);
		} else {
			if(!visible) standout();
			addch((Player.b_flags & HALLUCINATING) == 0 ? monst->b_ch : RANDMONST);
			if(!visible) {
				standend();
				detected = TRUE;
			}
		}
	}
	if(off) {
		Player.b_flags &= ~DETECTING;
	} else {
		Player.b_flags |= DETECTING;
	}
	return(detected);
}

int seen_stairs()
{
	register struct being *monst;
	move(Stairs.y,Stairs.x);
	if(WINCH(stdscr) == STAIRCASE) return(TRUE);
	if(MYX == Stairs.x && MYY == Stairs.y) return(TRUE);
	if((monst = Places[Stairs.x][Stairs.y].p_being) != NULL) {
		if((see_monst(monst) && (monst->b_flags & MOBILE)) || ((Player.b_flags & DETECTING) && monst->b_oldch == STAIRCASE)) return(TRUE);
	}
	return(FALSE);
}

void raise_level()
{
	Player.b_stats.experience = E_levels[Player.b_stats.level-1]+1;
	check_level();
}

void do_pot(int which, bool known)
{
	register struct pact *pot_act = &P_actions[which];
	register int sp;
	if(!Pot_info[which].inf_known) Pot_info[which].inf_known = known;
	sp = spread(pot_act->p_spread);
	if((Player.b_flags & pot_act->p_flag) == 0) {
		Player.b_flags |= pot_act->p_flag;
		fuse(pot_act->p_fuse,0,sp,spread(2));
		look(FALSE);
	} else {
		lengthen(pot_act->p_fuse,sp);
	}
	msg(choose_str(pot_act->p_hstr,pot_act->p_nstr));
}

