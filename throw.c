/*
 *	ROGUE
 *
 *	@(#)throw.c	1.1	Strathclyde	10/31/84
 */

# include "param.h"
# include "types.h"
# include "extern.h"

struct coords	Fall_pos;

void missile(int yinc, int xinc)
{
	register struct item *thing;
	if((thing = get_item("throw",WEAPON)) == 0) return;
	if(!dropcheck(thing) || is_current(thing)) return;
	thing = leave_pack(thing,TRUE,FALSE);
	do_motion(thing,yinc,xinc);
	if(Places[thing->i_coords.x][thing->i_coords.y].p_being == 0 || !hit_monster(thing->i_coords.y,thing->i_coords.x,thing)) fall(thing,TRUE);
}

void do_motion(struct item *thing, int yinc, int xinc)
{
	register int floorch;
	thing->i_coords = Player.b_coords;
	for(;;) {
		if(thing->i_coords.x != MYX || thing->i_coords.y != MYY) {
			if(cansee(thing->i_coords.y,thing->i_coords.x) && Verbose) {
				floorch = Places[thing->i_coords.x][thing->i_coords.y].p_ch;
				if(floorch == FLOOR && !show_floor()) floorch = ' ';
				mvaddch(thing->i_coords.y,thing->i_coords.x,floorch);
			}
		}
		thing->i_coords.y += yinc;
		thing->i_coords.x += xinc;
		if(
			!step_ok(
				floorch = Places[thing->i_coords.x][thing->i_coords.y].p_being ?
					Places[thing->i_coords.x][thing->i_coords.y].p_being->b_app :
					Places[thing->i_coords.x][thing->i_coords.y].p_ch
			) ||
			floorch == DOOR
		) break;
		if(cansee(thing->i_coords.y,thing->i_coords.x) && Verbose) {
			mvaddch(thing->i_coords.y,thing->i_coords.x,thing->i_item);
			refresh();
		}
	}
}

void fall(register struct item *thing, bool mine)
{
	register struct place *fplace;
	if(fallpos(&thing->i_coords,&Fall_pos)) {
		fplace = &Places[Fall_pos.x][Fall_pos.y];
		fplace->p_ch = thing->i_item;
		thing->i_coords = Fall_pos;
		if(cansee(Fall_pos.y,Fall_pos.x)) {
			if(fplace->p_being) {
				fplace->p_being->b_oldch = thing->i_item;
			} else {
				mvaddch(Fall_pos.y,Fall_pos.x,thing->i_item);
			}
		}
		_attach(&Lvl_obj,thing);
		return;
	}
	discard(thing);
	if(mine) {
		if(Has_hit) {
			endmsg();
			Has_hit = FALSE;
		}
		msg("the %s vanishes as it hits the ground",missile_name(thing));
	}
}

char *missile_name(register struct item *thing)
{
	switch(thing->i_item) {
	case ')':
		return(Weap_info[thing->i_type].inf_name);
	case '/':
		return(Ws_type[thing->i_type]);
	default:
		return(type_name(thing->i_item));
	}
}

