/*
 *	ROGUE
 *
 *	@(#)screen.c	1.1	Strathclyde	10/31/84
 */

# include "param.h"
# include "types.h"
# include "extern.h"

# define MAZEY	9
# define MAZEX	27

static void do_maze(register struct room *maze);
static void vert(register struct room *room, register int x);
static void horiz(register struct room *room,int y);
static void dig(register int y, int x);
static void accnt_maze(int y, int x, int ypoint, int xpoint);

int	Xminimum, Yminimum;
int	Maxy, Maxx;
int	Starty, Startx;

struct maze {
	int		maze_ct;
	struct coords	maze_e4[4];
	int		maze_l24;
} Maze[MAZEY][MAZEX];

struct coords Pass_inc[] = {
{	2,	0},
{	-2,	0},
{	0,	2},
    {	0,	-2}
};

void do_rooms()
{
	register int rm;
	register struct room *droom;
	register struct being *monst;
	register int skip;
	register struct item *gold;
	int ylimit, xlimit;
	struct coords mpos;
	short *flags;
	xlimit = MAZEX-1;
	ylimit = MAZEY-1;
	for(droom = Rooms; droom < &Rooms[ROOMS]; droom++) {
		droom->r_value = 0;
		droom->r_dct = 0;
		droom->r_flags = 0;
	}
	skip = rnd(4);
	for(rm = 0; rm < skip; rm++) {
		flags = &Rooms[rnd_room()].r_flags;
		*flags |= R_PASSAGE;
	}
	for(rm = 0, droom = Rooms; rm < 9; droom++, rm++) {
		Xminimum = (rm%3)*xlimit+1;
		Yminimum = (rm/3)*ylimit;
		if(droom->r_flags & R_PASSAGE) {
			do {
				droom->r_xstart = rnd(xlimit-2)+Xminimum+1;
				droom->r_ystart = rnd(ylimit-2)+Yminimum+1;
				droom->r_xsize = -80;
				droom->r_ysize = -24;
			} while(droom->r_ystart <= 0 || droom->r_ystart >= 23);
		} else {
			if(rnd(10) < Level-1) {
				droom->r_flags |= R_DARK;
				if(rnd(15) == 0) droom->r_flags = R_MAZE;
			}
			if(droom->r_flags & R_MAZE) {
				droom->r_xsize = xlimit-1;
				droom->r_ysize = ylimit-1;
				droom->r_xstart = Xminimum;
				if(droom->r_xstart == 1) droom->r_xstart = 0;
				if((droom->r_ystart = Yminimum) == 0) {
					droom->r_ystart++;
					droom->r_ysize--;
				}
			} else {
				do {
					droom->r_xsize = rnd(xlimit-4)+4;
					droom->r_ysize = rnd(ylimit-4)+4;
					droom->r_xstart = rnd(xlimit-droom->r_xsize)+Xminimum;
					droom->r_ystart = rnd(ylimit-droom->r_ysize)+Yminimum;
				} while(droom->r_ystart == 0);
			}
			draw_room(droom);
			if(rnd(2) == 0 && (!Amulet || Level >= Max_level)) {
				gold = new_item();
				droom->r_value = AU;
				gold->i_value = droom->r_value;
				find_floor(droom,&droom->r_gold,0,FALSE);
				gold->i_coords = droom->r_gold;
				Places[droom->r_gold.x][droom->r_gold.y].p_ch = GOLD;
				gold->i_flags = GROUP;
				gold->i_group = 1;
				gold->i_item = GOLD;
				_attach(&Lvl_obj,gold);
			}
			if(rnd(100) < (droom->r_value > 0 ? 80 : 25)) {
				monst = (struct being *)(new_item());
				find_floor(droom,&mpos,0,TRUE);
				new_monster(monst,randmonster(FALSE),&mpos);
				give_pack(monst);
			}
		}
	}
}

void draw_room(register struct room *room)
{
	register int y, x;
	if(room->r_flags & R_MAZE) {
		do_maze(room);
		return;
	}
	vert(room,room->r_xstart);
	vert(room,room->r_xstart+room->r_xsize-1);
	horiz(room,room->r_ystart);
	horiz(room,room->r_ystart+room->r_ysize-1);
	for(y = room->r_ystart+1; y < room->r_ystart+room->r_ysize-1; y++) {
		for(x = room->r_xstart+1; x < room->r_xstart+room->r_xsize-1; x++) {
			Places[x][y].p_ch = FLOOR;
		}
	}
}

void vert(register struct room *room, register int x)
{
	register int y;
	for(y = room->r_ystart+1; y <= room->r_ysize+room->r_ystart-2; y++) {
		Places[x][y].p_ch = VWALL;
	}
}

void horiz(register struct room *room,
	   int y)
{
	register int x;
	for(x = room->r_xstart; x <= room->r_xstart+room->r_xsize-1; x++) {
		Places[x][y].p_ch = HWALL;
	}
}

void do_maze(register struct room *maze)
{
	register struct maze *mz;
	register int ystart, xstart;
	static struct coords startpos;
	for(mz = Maze[0]; mz <= Maze[MAZEY-1]; mz++) {
		mz->maze_l24 = 0;
		mz->maze_ct = 0;
	}
	Maxy = maze->r_ysize;
	Maxx = maze->r_xsize;
	Starty = maze->r_ystart;
	Startx = maze->r_xstart;
	ystart = (rnd(maze->r_ysize)/2)*2;
	xstart = (rnd(maze->r_xsize)/2)*2;
	startpos.y = ystart+Starty;
	startpos.x = xstart+Startx;
	putpass(&startpos);
	dig(ystart,xstart);
}

void dig(register int y, int x)
{
	register struct coords *inc;
	register int count, ytry, xtry;
	int newy, newx;
	static struct coords pass;
	for(;;) {
		count = 0;
		for(inc = Pass_inc; inc < &Pass_inc[4]; inc++) {
			ytry = y+inc->y;
			xtry = x+inc->x;
			if(
				ytry >= 0 && ytry <= Maxy &&
				xtry >= 0 && xtry <= Maxx &&
				(Places[xtry+Startx][ytry+Starty].p_flags & P_PASSAGE) == 0 &&
				rnd(++count) == 0
			) {
				newy = ytry;
				newx = xtry;
			}
		}
		if(count == 0) return;
		accnt_maze(y,x,newy,newx);
		accnt_maze(newy,newx,y,x);
		if(newy == y) {
			pass.y = y+Starty;
			if(newx-x < 0) {
				pass.x = newx+Startx+1;
			} else {
				pass.x = newx+Startx-1;
			}
		} else {
			pass.x = x+Startx;
			if(newy-y < 0) {
				pass.y = newy+Starty+1;
			} else {
				pass.y = newy+Starty-1;
			}
		}
		putpass(&pass);
		pass.y = newy+Starty;
		pass.x = newx+Startx;
		putpass(&pass);
		dig(newy,newx);
	}
}

void accnt_maze(int y, int x, int ypoint, int xpoint)
{
	register struct maze *mz = &Maze[y][x];
	register struct coords *point;
	for(point = mz->maze_e4; point < &mz->maze_e4[mz->maze_ct]; point++) {
		if(point->y == ypoint && point->x == xpoint) return;
	}
	point->y = ypoint;
	point->x = xpoint;
}

void rnd_pos(
    register struct room *room,
    register struct coords *c)
{
	c->x = rnd(room->r_xsize-2)+room->r_xstart+1;
	c->y = rnd(room->r_ysize-2)+room->r_ystart+1;
}

int find_floor(
    register struct room *room,
    register struct coords *c,
    int max_tries,
	bool monst)
{
	register struct place *floor;
	register int count;
	char floorch;
	bool any_room;
	if(!(any_room = room == 0)) {
		floorch = room->r_flags & R_MAZE ? PASSAGE : FLOOR;
	}
	count = max_tries;
	for(;;) {
		if(max_tries && count-- == 0) return(FALSE);
		if(any_room) {
			room = &Rooms[rnd_room()];
			floorch = room->r_flags & R_MAZE ? PASSAGE : FLOOR;
		}
		rnd_pos(room,c);
		floor = &Places[c->x][c->y];
		if(monst) {
			if(floor->p_being == 0 && step_ok(floor->p_ch)) return(TRUE);
		} else {
			if(floor->p_ch == floorch) return(TRUE);
		}
	}
}

void enter_room(register struct coords *door)
{
	register struct room *rm;
	register struct being *monst;
	register int y, x;
	char floorch;
	rm = Player.b_room = roomin(door);
	door_open(rm);
	if((rm->r_flags & R_DARK) == 0 && (Player.b_flags & BLIND) == 0) {
		for(y = rm->r_ystart; y < rm->r_ysize+rm->r_ystart; y++) {
			move(y,rm->r_xstart);
			for(x = rm->r_xstart; x < rm->r_xsize+rm->r_xstart; x++) {
				monst = Places[x][y].p_being;
				floorch = Places[x][y].p_ch;
				if(monst == 0) {
					if(winch(stdscr) != floorch) {
						addch(floorch);
					} else {
						move(y,x+1);
					}
				} else {
					monst->b_oldch = floorch;
					if(!see_monst(monst)) {
						if(Player.b_flags & DETECTING) {
							standout();
							addch(monst->b_app);
							standend();
						} else {
							addch(floorch);
						}
					} else {
						addch(monst->b_app);
					}
				}
			}
		}
	}
}

void leave_room(register struct coords *door)
{
	register struct room *rm = Player.b_room;
	register int y, x;
	char floorch, dispch;
	if(rm->r_flags & R_MAZE) return;
	if(rm->r_flags & R_PASSAGE) {
		floorch = PASSAGE;
	} else if((rm->r_flags & R_DARK) == 0 || (Player.b_flags & BLIND)) {
		floorch = FLOOR;
	} else {
		floorch = ' ';
	}
	Player.b_room = &Passages[Places[door->x][door->y].p_flags & PASSNUM];
	for(y = rm->r_ystart; y < rm->r_ysize+rm->r_ystart; y++) {
		for(x = rm->r_xstart; x < rm->r_xsize+rm->r_xstart; x++) {
			move(y,x);
			if((dispch = winch(stdscr)) != FLOOR) {
				if(isupper(toascii(dispch))) {
					if(Player.b_flags & DETECTING) {
						standout();
						addch(dispch);
						standend();
					} else {
						addch(Places[x][y].p_ch == DOOR ? DOOR : floorch);
					}
				}
			} else if(floorch == ' ' && dispch != ' ') {
				addch(' ');
			}
		}
	}
	door_open(rm);
}

