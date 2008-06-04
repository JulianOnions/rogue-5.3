/*
 *	ROGUE
 *
 *	@(#)scroll.c	1.1	Strathclyde	10/31/84
 */

# include "param.h"
# include "types.h"
# include "extern.h"

struct coords	Create_pos;

char	Ident_type[] = {
	0,	0,	0,	0,	0,
	POTION,	SCROLL,	WEAPON,	ARMOUR,	-2,
	0,	0,	0,	0,	0
};
static void uncurse(register struct item *thing);

void read_scroll()
{
	register struct item *scroll;
	register int y, x, count;
	register struct room *rm;
	register struct being *monst;
	int mcount;
	char floorch;
	bool one = FALSE, found;
	struct item *save;
	if((scroll = get_item("read",SCROLL)) == 0) return;
	if(scroll->i_item != SCROLL) {
		msg(Verbose ? "there is nothing on it to read" : "nothing to read");
		return;
	}
	if(scroll == Cur_weapon) Cur_weapon = 0;
	one = scroll->i_number == 1;
	leave_pack(scroll,FALSE,FALSE);
	save = scroll;
	switch(scroll->i_type) {
	case S_MONSTER_CONFUSION:
		Player.b_flags |= MONST_CONFUSION;
		msg("your hands begin to glow %s",pick_colour("red"));
		break;
	case S_ENCHANT_ARMOUR:
		if(Cur_armor) {
			Cur_armor->i_value--;
			Cur_armor->i_flags &= ~CURSED;
			msg("your armor glows %s for a moment",pick_colour("silver"));
		}
		break;
	case S_HOLD_MONSTER:
		mcount = FALSE;
		for(x = MYX-2; x <= MYX+2; x++) {
			if(x >= 0 && x < 80) {
				for(y = MYY-2; y <= MYY+2; y++) {
					if(y >= 0 && y <= 23) {
						if((monst = Places[x][y].p_being) && (monst->b_flags & MOBILE)) {
							monst->b_flags &= ~MOBILE;
							monst->b_flags |= HELD;
							mcount++;
						}
					}
				}
			}
		}
		if(mcount) {
			addmsg("the monster");
			if(mcount > 1) addmsg("s around you");
			addmsg(" freeze");
			if(mcount == 1) addmsg("s");
			endmsg();
			Scr_info[2].inf_known = TRUE;
		} else {
			msg("you feel a strange sense of loss");
		}
		break;
	case S_SLEEP:
		Scr_info[3].inf_known = TRUE;
		No_command += rnd(spread(5))+4;
		Player.b_flags &= ~MOBILE;
		msg("you fall asleep");
		break;
	case S_CREATE_MONSTER:
		count = 0;
		for(y = MYY-1; y <= MYY+1; y++) {
			for(x = MYX-1; x <= MYX+1; x++) {
				if(
					(y != MYY || x != MYX) &&
					step_ok(floorch = Places[x][y].p_being ? Places[x][y].p_being->b_app : Places[x][y].p_ch) &&
					(floorch != SCROLL || find_obj(y,x)->i_type != S_SCARE_MONSTER) &&
					rnd(++count) == 0
				) {
					Create_pos.y = y;
					Create_pos.x = x;
				}
			}
		}
		if(count == 0) {
			msg("you hear a faint cry of anguish in the distance");
		} else {
			monst = (struct being *)(new_item());
			new_monster(monst,randmonster(FALSE),&Create_pos);
		}
		break;
	case S_IDENTIFY_POTION:
	case S_IDENTIFY_SCROLL:
	case S_IDENTIFY_WEAPON:
	case S_IDENTIFY_ARMOUR:
	case S_IDENTIFY_RWS:
		Scr_info[scroll->i_type].inf_known = TRUE;
		msg("this scroll is an %s scroll",Scr_info[scroll->i_type].inf_name);
		whatis(TRUE,Ident_type[scroll->i_type]);
		break;
	case S_MAGIC_MAPPING:
		Scr_info[1].inf_known = TRUE;
		msg("oh, now this scroll has a map on it");
		show_level(FALSE);
		break;
	case S_FOOD_DETECTION:
		found = FALSE;
		wclear(Hw);
		for(scroll = Lvl_obj; scroll; scroll = scroll->i_link) {
			if(scroll->i_item == FOOD) {
				found = TRUE;
				wmove(Hw,scroll->i_coords.y,scroll->i_coords.x);
				waddch(Hw,FOOD);
			}
		}
		if(found) {
			Scr_info[S_FOOD_DETECTION].inf_known = TRUE;
			show_win("Your nose tingles and you smell food.--More--");
		} else {
			msg("your nose tingles");
		}
		break;
	case S_TELEPORTATION:
		rm = Player.b_room;
		teleport();
		if(rm != Player.b_room) Scr_info[S_TELEPORTATION].inf_known = TRUE;
		break;
	case S_ENCHANT_WEAPON:
		if(Cur_weapon && Cur_weapon->i_item == WEAPON) {
			Cur_weapon->i_flags &= ~CURSED;
			if(rnd(2) == 0) {
				Cur_weapon->i_hitplus++;
			} else {
				Cur_weapon->i_damplus++;
			}
			msg("your %s glows %s for a moment",Weap_info[Cur_weapon->i_type].inf_name,pick_colour("blue"));
		} else {
			msg("you feel a strange sense of loss");
		}
		break;
	case S_SCARE_MONSTER:
		msg("you hear maniacal laughter in the distance");
		break;
	case S_REMOVE_CURSE:
		uncurse(Cur_armor);
		uncurse(Cur_weapon);
		uncurse(Cur_ring[LEFT]);
		uncurse(Cur_ring[RIGHT]);
		msg(choose_str("you feel in touch with the Universal Onenes","you feel as if somebody is watching over you"));
		break;
	case S_AGGRAVATE_MONSTERS:
		aggravate();
		msg("you hear a high pitched humming noise");
		break;
	case S_PROTECT_ARMOUR:
		if(Cur_armor) {
			Cur_armor->i_flags |= PROTECTED;
			msg("your armor is covered by a shimmering %s shield",pick_colour("gold"));
		} else {
			msg("you feel a strange sense of loss");
		}
	}
	scroll = save;
	look(TRUE);
	status();
	call_it(&Scr_info[scroll->i_type]);
	if(one) discard_item(scroll);
}

static void uncurse(register struct item *thing)
{
	if(thing) thing->i_flags &= ~CURSED;
}

