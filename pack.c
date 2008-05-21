/*
 *	ROGUE
 *
 *	@(#)pack.c	1.1	Strathclyde	10/31/84
 */

# include "param.h"
# include "types.h"
# include "extern.h"

static void move_msg(struct item *thing);

void check_coord_valid(struct coords *cp)
{
    struct being *runner;
    for(runner = Mlist; runner; runner = runner->b_link) {
	if(runner->b_dest == cp) abort();
    }
}

void redirect_monster(struct coords *cp)
{
    struct being *runner;
    for(runner = Mlist; runner; runner = runner->b_link) {
	if(runner->b_dest == cp) 
	    runner->b_dest = find_dest(runner);
    }
}


void add_pack(register struct item *thing, bool silent)
{
	register struct item *pack, *after;
	register struct being *monst;
	bool picked_up = FALSE;
	if(thing == 0) {
		if((thing = find_obj(MYY,MYX)) == 0) return;
		picked_up = TRUE;
	}
	if(thing->i_item == SCROLL && thing->i_type == S_SCARE_MONSTER && (thing->i_flags & DROPPED)) {
		_detach(&Lvl_obj,thing);
		mvaddch(MYY,MYX,floor_ch());
		Places[MYX][MYY].p_ch = Player.b_room->r_flags & R_PASSAGE ? PASSAGE : FLOOR;
		check_coord_valid(&thing->i_coords);
		discard(thing);
		msg("the scroll turns to dust as you pick it up");
		return;
	}
	if(Player.b_pack == 0) {
		if(pack_room(picked_up,thing)) {
			Player.b_pack = thing;
			thing->i_packchar = pack_char();
		}
	} else {
		after = 0;
		for(pack = Player.b_pack; pack; pack = pack->i_link) {
			if(pack->i_item == thing->i_item) {
				while(pack->i_item == thing->i_item && pack->i_type != thing->i_type) {
					after = pack;
					if(pack->i_link == 0) break;
					pack = pack->i_link;
				}
				if(pack->i_item != thing->i_item || pack->i_type != thing->i_type) break;
				if(pack->i_item == POTION || pack->i_item == SCROLL || pack->i_item == FOOD) {
					if(pack_room(picked_up,thing) == 0) return;
					pack->i_number++;
					check_coord_valid(&thing->i_coords);
					discard(thing);
					thing = pack;
					after = 0;
					break;
				}
				if(thing->i_group) {
					after = pack;
					while(pack->i_item == thing->i_item && pack->i_type == thing->i_type && pack->i_group != thing->i_group) {
						after = pack;
						if(pack->i_link == 0) break;
						pack = pack->i_link;
					}
					if(pack->i_item != thing->i_item || pack->i_type != thing->i_type || pack->i_group != thing->i_group) break;
					pack->i_number += thing->i_number;
					Inpack--;
					if(pack_room(picked_up,thing)) {
					    check_coord_valid(&thing->i_coords);
						discard(thing);
						thing = pack;
						after = 0;
						break;
					}
					return;
				}
				after = pack;
				break;
			}
			after = pack;
		}
		if(after) {
			if(pack_room(picked_up,thing) == 0) return;
			thing->i_packchar = pack_char();
			thing->i_link = after->i_link;
			thing->i_back = after;
			if(after->i_link) after->i_link->i_back = thing;
			after->i_link = thing;
		}
	}
	thing->i_flags |= DROPPED;
	for(monst = Mlist; monst; monst = monst->b_link) {
		if(monst->b_dest == &thing->i_coords) monst->b_dest = &ME;
	}
	if(thing->i_item == AMULET) Amulet = TRUE;
	if(!silent) {
		if(Verbose) addmsg("you now have ");
		msg("%s (%c)",inv_name(thing,Verbose),thing->i_packchar);
	}
}

int pack_room(bool picked_up, struct item *thing)
{
	if(++Inpack > MAX_PACK) {
		if(Verbose) addmsg("there's ");
		addmsg("no room");
		if(Verbose) addmsg(" in your pack");
		endmsg();
		if(picked_up) move_msg(thing);
		Inpack = MAX_PACK;
		return(FALSE);
	}
	if(picked_up) {
		_detach(&Lvl_obj,thing);
		mvaddch(MYY,MYX,floor_ch());
		Places[MYX][MYY].p_ch = Player.b_room->r_flags & R_PASSAGE ? PASSAGE : FLOOR;
	}
	return(TRUE);
}

struct item *leave_pack(register struct item *thing, bool one, bool all)
{
	register struct item *it;
	Inpack--;
	it = thing;
	if(thing->i_number > 1 && !all) {
		thing->i_number--;
		if(thing->i_group) Inpack++;
		if(one) {
			it = new_item();
			*it = *thing;
			it->i_link = 0;
			it->i_back = 0;
			it->i_number = 1;
		}
	} else {
		Pack_used[thing->i_packchar-'a'] = FALSE;
		_detach(&Player.b_pack,thing);
	}
	return(it);
}

char pack_char()
{
	register bool *packch;
	for(packch = Pack_used; *packch; packch++);
	*packch = TRUE;
	return(packch-Pack_used+'a');
}

int inventory(register struct item *list, register int type)
{
	static char buf[MAXSTR];
	for(N_objs = 0; list; list = list->i_link) {
		if(
			type == 0 ||
			type == list->i_item ||
			(type == -1 && list->i_item != FOOD && list->i_item != AMULET) ||
			(type == -2 && (list->i_item == RING || list->i_item == STICK))
		) {
			N_objs++;
			sprintf(buf,"%c) %%s",list->i_packchar);
			Msg_esc = TRUE;
			if(add_line(buf,inv_name(list,FALSE)) == ESC) {
				Msg_esc = FALSE;
				msg("");
				return(TRUE);
			}
			Msg_esc = FALSE;
		}
	}
	if(N_objs == 0) {
		msg(Terse ?
			type == 0 ? "empty handed" : "nothing appropriate" :
			type == 0 ? "you are empty handed" : "you don't have anything appropriate"
		);
		return(FALSE);
	}
	end_line();
	return(TRUE);
}

void pick_up()
{
	register struct item *thing;
	if(Player.b_flags & LEVITATED) return;
	if((thing = find_obj(MYY,MYX)) == 0) {
		if(Verbose) addmsg("there is ");
		addmsg("nothing ");
		addmsg(Verbose ? "to pick up" : "there");
		endmsg();
		return;
	}
	if(Move_on) {
		move_msg(thing);
		return;
	}
	if(thing->i_item == GOLD) {
		money(thing->i_value);
		_detach(&Lvl_obj,thing);
		redirect_monster(&thing->i_coords);
		discard(thing);
		Player.b_room->r_value = 0;
		return;
	}
	add_pack(0,FALSE);
}

void move_msg(struct item *thing)
{
	if(Verbose) addmsg("you ");
	msg("moved onto %s",inv_name(thing,TRUE));
}

void picky_inven()
{
	register struct item *thing;
	char c;
	if(Player.b_pack == 0) {
		msg("you aren't carrying anything");
		return;
	}
	if(Player.b_pack->i_link == 0) {
		msg("a) %s",inv_name(Player.b_pack,FALSE));
		return;
	}
	msg(Terse ? "item: " : "which item do you wish to inventory: ");
	Mpos = 0;
	if((c = readchar()) == ESC) {
		msg("");
		return;
	}
	for(thing = Player.b_pack; thing; thing = thing->i_link) {
		if(c == thing->i_packchar) {
			msg("%c) %s",c,inv_name(thing,FALSE));
			return;
		}
	}
	msg("'%s' not in pack",unctrl(c));
}

struct item *get_item(action,type)
	char *action, type;
{
	register struct item *thing;
	char c;
	if(Player.b_pack == 0) {
		msg("you aren't carrying anything");
		return(0);
	}
	for(;;) {
		if(Verbose) addmsg("which object do you want to ");
		addmsg(action);
		if(Terse) addmsg(" what");
		msg("? (* for list): ");
		c = readchar();
		Mpos = 0;
		if(c == ESC) {
			After = FALSE;
			msg("");
			return(0);
		}
		N_objs = 1;
		if(c == '*') {
			Mpos = 0;
			if(!inventory(Player.b_pack,type)) {
				After = FALSE;
				return(0);
			}
		} else {
			for(thing = Player.b_pack; thing; thing = thing->i_link) {
				if(thing->i_packchar == c) break;
			}
			if(thing) return(thing);
			msg("that's not a valid item");
		}
	}
}

void money(register int amount)
{
	Purse += amount;
	mvaddch(MYY,MYX,floor_ch());
	Places[MYX][MYY].p_ch = Player.b_room->r_flags & R_PASSAGE ? PASSAGE : FLOOR;
	if(amount > 0) {
		if(Verbose) addmsg("you found ");
		msg("%d gold pieces",amount);
	}
}

