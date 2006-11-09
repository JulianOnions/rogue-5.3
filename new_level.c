/*
 *	ROGUE
 *
 *	@(#)new_level.c	1.1	Strathclyde	10/31/84
 */

# include "param.h"
# include "types.h"
# include "extern.h"

void new_level()
{
	register struct being *monst;
	register struct place *p;
	register unsigned char *flags;
	register int traps;
	Player.b_flags &= ~HELD;
	if(Level > Max_level) Max_level = Level;
	for(p = Places[0]; p <= Places[MAXCOLS-1]; p++) {
		p->p_ch = ' ';
		p->p_flags = P_KNOWN;
		p->p_being = 0;
	}
	clear();
	for(monst = Mlist; monst; monst = monst->b_link) _free_list(&monst->b_pack);
	_free_list(&Mlist);
	_free_list(&Lvl_obj);
	do_rooms();
	do_passages();
	No_food++;
	put_things();
	if(rnd(10) < Level) {
		Ntraps = rnd(Level/4)+1;
		if(Ntraps > 10) Ntraps = 10;
		traps = Ntraps;
		while(traps--) {
			do find_floor((struct room *)0,&Stairs,0,FALSE); while(Places[Stairs.x][Stairs.y].p_ch != FLOOR);
			flags = &Places[Stairs.x][Stairs.y].p_flags;
			*flags &= ~P_KNOWN;
			*flags |= rnd(TRAPS);
		}
	}
	find_floor((struct room *)0,&Stairs,0,FALSE);
	Places[Stairs.x][Stairs.y].p_ch = STAIRCASE;
	Seenstairs = FALSE;
	for(monst = Mlist; monst; monst = monst->b_link) monst->b_room = roomin(&bc(monst));
	find_floor((struct room *)0,&ME,0,TRUE);
	enter_room(&ME);
	mvaddch(MYY,MYX,PLAYER);
	if(Player.b_flags & DETECTING) turn_see(FALSE);
	if(Player.b_flags & HALLUCINATING) visuals();
}

int rnd_room()
{
	register int r;
	do r = rnd(ROOMS); while(Rooms[r].r_flags & R_PASSAGE);
	return(r);
}

void put_things()
{
	register int nthings;
	register struct item *thing;
	if(Amulet && Level < Max_level) return;
	if(rnd(20) == 0) treas_room();
	for(nthings = 0; nthings < 9; nthings++) {
		if(rnd(100) < 36) {
			thing = new_thing();
			_attach(&Lvl_obj,thing);
			find_floor((struct room *)0,&thing->i_coords,0,FALSE);
			Places[thing->i_coords.x][thing->i_coords.y].p_ch = thing->i_item;
		}
	}
	if(Level >= 26 && !Amulet) {
		thing = new_item();
		_attach(&Lvl_obj,thing);
		thing->i_hitplus = 0;
		thing->i_damplus = 0;
		thing->i_hpower = thing->i_tpower = "0x0";
		thing->i_value = DEF_VALUE;
		thing->i_item = AMULET;
		find_floor((struct room *)0,&thing->i_coords,0,FALSE);
		Places[thing->i_coords.x][thing->i_coords.y].p_ch = AMULET;
	}
}

void treas_room()
{
	register int count;
	register struct item *thing;
	register struct being *monst;
	register struct room *troom;
	register int max, things;
	static struct coords pos;
	troom = &Rooms[rnd_room()];
	max = (troom->r_ysize-2)*(troom->r_xsize-2)-2;
	if(max > 8) max = 8;
	count = rnd(max)+2;
	things = count;
	while(count--) {
		find_floor(troom,&pos,20,FALSE);
		thing = new_thing();
		thing->i_coords = pos;
		_attach(&Lvl_obj,thing);
		Places[pos.x][pos.y].p_ch = thing->i_item;
	}
	if((count = rnd(max)+2) < things+2) count = things+2;
	max = (troom->r_ysize-2)*(troom->r_xsize-2);
	if(count > max) count = max;
	Level++;
	while(count--) {
		max = 0;
		if(find_floor(troom,&pos,10,TRUE)) {
			monst = (struct being *)(new_item());
			new_monster(monst,randmonster(FALSE),&pos);
			monst->b_flags |= AGGRESSIVE;
			give_pack(monst);
		}
	}
	Level--;
}

void show_level(bool things)
{
	register struct place *here;
	register int x, y;
	register struct being *monst;
	char floorch;
	for(y = 1; y < MAXY; y++) {
		for(x = 0; x < MAXX; x++) {
			here = &Places[x][y];
			floorch = here->p_ch;
			switch(floorch) {
			case VWALL:
			case HWALL:
				if((here->p_flags & P_KNOWN) == 0) {
					here->p_ch = DOOR;
					floorch = here->p_ch;
					here->p_flags |= P_KNOWN;
				}
				break;
			case DOOR:
			case STAIRCASE:
				break;
			case FLOOR:
				if(here->p_flags & P_KNOWN) continue;
				floorch = TRAP;
				here->p_ch = TRAP;
				here->p_flags |= (P_DISCOVERED|P_KNOWN);
			case ' ':
				if((here->p_flags & P_KNOWN) == 0) {
					floorch = PASSAGE;
					here->p_flags |= (P_DISCOVERED|P_KNOWN);
					here->p_ch = PASSAGE;
				}
				break;
			case PASSAGE:
				here->p_flags |= (P_DISCOVERED|P_KNOWN);
				break;
			default:
				if(!things) continue;
			}
			if(floorch != ' ') {
				if((monst = here->p_being) != NULL) monst->b_oldch = floorch;
				if(monst == 0 || (Player.b_flags & DETECTING) == 0) mvaddch(y,x,floorch);
			}
		}
	}
}

