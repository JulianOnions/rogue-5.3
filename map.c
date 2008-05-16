/*
 *	ROGUE
 *
 *	@(#)map.c	1.1	Strathclyde	10/31/84
 */

# include "param.h"
# include "types.h"
# include "extern.h"

void look(bool wake)
{
	int ylimit, xlimit;
	int passcount = 0;
	unsigned char flags;
	unsigned char *lflags;
	char floorch;
	int ystart, xstart;
	int yxsum = 0, yxdiff = 0;
	register int x, y;
	register int lookch;
	register struct being *monst;
	register struct place *lookat;
	register struct room *rp = Player.b_room;

	if(cne(Oldpos,ME)) {
		erase_lamp(&Oldpos,Oldrp);
		Oldpos = ME;
		Oldrp = rp;
	}
	ylimit = MYY+1;
	xlimit = MYX+1;
	xstart = MYX-1;
	ystart = MYY-1;
	if(Door_stop && !Firstmove && Running) {
		yxsum = MYY+MYX;
		yxdiff = MYY-MYX;
	}
	lookat = &Places[MYX][MYY];
	floorch = lookat->p_ch;
	flags = lookat->p_flags;
	for(y = ystart; y <= ylimit; y++) if(y > 0 && y < 23) {
		for(x = xstart; x <= xlimit; x++) if(x >= 0 && x < 80) {
			if((Player.b_flags & BLIND) == 0 && y == MYY && x == MYX) continue;
			lookat = &Places[x][y];
			lookch = lookat->p_ch;
			if(lookch == ' ') continue;
			lflags = &lookat->p_flags;
			if(floorch != DOOR && lookch != DOOR) {
				if((flags & P_PASSAGE) != (*lflags & P_PASSAGE)) continue;
			}
			if(
				((*lflags & P_PASSAGE) == 0 && lookch != DOOR) ||
				((flags & P_PASSAGE) == 0 && floorch != DOOR) ||
				MYX == x ||
				MYY == y ||
				step_ok(Places[MYX][y].p_ch) ||
				step_ok(Places[x][MYY].p_ch)
			) {
				if((monst = lookat->p_being) != NULL) {
					if((Player.b_flags & DETECTING) && (monst->b_flags & INVISIBLE)) {
						if(Door_stop && !Firstmove) Running = FALSE;
						continue;
					} else {
						if(wake) wake_monster(y,x);
						if(see_monst(monst)) {
							if(Player.b_flags & HALLUCINATING) {
								lookch = RANDMONST;
							} else {
								lookch = monst->b_app;
							}
						}
					}
				} else {
					lookch = trip_ch(y,x,lookch);
				}
				if(Player.b_flags & BLIND) {
					if(y != MYY || x != MYX) continue;
				}
				move(y,x);
				if((Player.b_room->r_flags & R_DARK) && !See_floor && lookch == FLOOR) lookch = ' ';
				if(monst || lookch != winch(stdscr)) addch(lookch);
				if(!Door_stop || Firstmove || !Running) continue;
				switch(Runch) {
				case 'h':
					if(x == xlimit) continue;
					break;
				case 'j':
					if(y == ystart) continue;
					break;
				case 'k':
					if(y == ylimit) continue;
					break;
				case 'l':
					if(x == xstart) continue;
					break;
				case 'y':
					if(y+x-yxsum >= 1) continue;
					break;
				case 'u':
					if(y-x-yxdiff >= 1) continue;
					break;
				case 'n':
					if(y+x-yxsum <= -1) continue;
					break;
				case 'b':
					if(y-x-yxdiff <= -1) continue;
				}
				switch(lookch) {
				case PASSAGE:
					if(x == MYX || y == MYY) passcount++;
					break;
				case DOOR:
					if(x == MYX || y == MYY) Running = FALSE;
				case ' ':
				case HWALL:
				case VWALL:
				case FLOOR:
					break;
				default:
					Running = FALSE;
				}
			}
		}
	}
	if(Door_stop && !Firstmove && passcount > 1) Running = FALSE;
	if(!Running || !Jump) mvaddch(MYY,MYX,PLAYER);
}

char trip_ch(int y, int x, char ch)
{
	if((Player.b_flags & HALLUCINATING) && After) {
		switch(ch) {
		case ' ':
		case PASSAGE:
		case DOOR:
		case HWALL:
		case FLOOR:
		case TRAP:
		case VWALL:
			break;
		default:
			if(y != Stairs.y || x != Stairs.x || !Seenstairs) ch = rnd_thing();
		}
	}
	return(ch);
}

void erase_lamp(register struct coords *oldpos, struct room *oldrp)
{
	register int y, x, ylimit, ystart, xlimit;
	if(!See_floor || (oldrp->r_flags & (R_DARK|R_PASSAGE)) != R_DARK || (Player.b_flags & BLIND)) return;
	ylimit = oldpos->y+1;
	xlimit = oldpos->x+1;
	ystart = oldpos->y-1;
	for(x = oldpos->x-1; x <= xlimit; x++) {
		for(y = ystart; y <= ylimit; y++) {
			if(y != MYY || x != MYX) {
				move(y,x);
				if(winch(stdscr) == FLOOR) addch(' ');
			}
		}
	}
}

int show_floor()
{
	if((Player.b_room->r_flags & (R_DARK|R_PASSAGE)) == R_DARK && (Player.b_flags & BLIND) == 0) return(See_floor);
	return(TRUE);
}

struct item *find_obj(register int y, int x)
{
	register struct item *thing;
	for(thing = Lvl_obj; thing; thing = thing->i_link) {
		if(thing->i_coords.y == y && thing->i_coords.x == x) return(thing);
	}
	return(0);
}

