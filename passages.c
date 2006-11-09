/*
 *	ROGUE
 *
 *	@(#)passages.c	1.1	Strathclyde	10/31/84
 */

# include "param.h"
# include "types.h"
# include "extern.h"

struct cn {
	bool	cn_possible[ROOMS];
	bool	cn_done[ROOMS];
	bool	cn_connected;
} Conn[ROOMS] = {
{	{FALSE,  TRUE, FALSE,  TRUE, FALSE, FALSE, FALSE, FALSE, FALSE}, 
	{FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE},
	FALSE},
{	{ TRUE, FALSE,  TRUE, FALSE,  TRUE, FALSE, FALSE, FALSE, FALSE}, 
        {FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE},
	FALSE},
{	{FALSE,  TRUE, FALSE, FALSE, FALSE,  TRUE, FALSE, FALSE, FALSE}, 
	{FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE},
	FALSE},
{	{TRUE, FALSE, FALSE, FALSE,  TRUE, FALSE,  TRUE, FALSE, FALSE}, 
	{FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE},
	FALSE},
{	{FALSE,  TRUE, FALSE,  TRUE, FALSE,  TRUE, FALSE,  TRUE, FALSE}, 
	{FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE},
	FALSE},
{	{FALSE, FALSE,  TRUE, FALSE,  TRUE, FALSE, FALSE, FALSE,  TRUE}, 
	{FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE},
	FALSE},
{	{FALSE, FALSE, FALSE,  TRUE, FALSE, FALSE, FALSE,  TRUE, FALSE}, 
	{FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE},
	FALSE},
{	{FALSE, FALSE, FALSE, FALSE,  TRUE, FALSE,  TRUE, FALSE,  TRUE}, 
	{FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE},
	FALSE},
{	{FALSE, FALSE, FALSE, FALSE, FALSE,  TRUE, FALSE,  TRUE, FALSE}, 
	{FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE},
	    FALSE}
};

int	Xinc, Yinc;

struct coords	Pass;

int	Xturn, Yturn;

struct coords	Start_door, Finish_door;

int	Pnum;
bool	Newpnum;

char floor_ch()
{
	if(Player.b_room->r_flags & R_PASSAGE) return(PASSAGE);
	return(show_floor() ? FLOOR : ' ');
}

char floor_at()
{
	char floorch = Places[MYX][MYY].p_ch;
	if(floorch == FLOOR) floorch = floor_ch();
	return(floorch);
}

void do_passages()
{
	register struct cn *start, *dest = NULL;
	register int startnum, destnum, ccount;
	for(start = Conn; start < &Conn[ROOMS]; start++) {
		for(destnum = 0; destnum < ROOMS; destnum++) start->cn_done[destnum] = FALSE;
		start->cn_connected = FALSE;
	}
	ccount = 1;
	start = &Conn[rnd(ROOMS)];
	start->cn_connected = TRUE;
	do {
		destnum = 0;
		for(startnum = 0; startnum < ROOMS; startnum++) {
			if(start->cn_possible[startnum] && !Conn[startnum].cn_connected) {
				if(rnd(++destnum) == 0) dest = &Conn[startnum];
			}
		}
		if(destnum == 0) {
			do start = &Conn[rnd(ROOMS)]; while(!start->cn_connected);
		} else {
			dest->cn_connected = TRUE;
			startnum = start-Conn;
			destnum = dest-Conn;
			conn(startnum,destnum);
			start->cn_done[destnum] = TRUE;
			dest->cn_done[startnum] = TRUE;
			ccount++;
		}
	} while(ccount < ROOMS);
	for(ccount = rnd(5); ccount > 0; ccount--) {
		start = &Conn[rnd(ROOMS)];
		destnum = 0;
		for(startnum = 0; startnum < ROOMS; startnum++) {
			if(start->cn_possible[startnum] && !start->cn_done[startnum]) {
				if(rnd(++destnum) == 0) dest = &Conn[startnum];
			}
		}
		if(destnum) {
			startnum = start-Conn;
			destnum = dest-Conn;
			conn(startnum,destnum);
			start->cn_done[destnum] = TRUE;
			dest->cn_done[startnum] = TRUE;
		}
	}
	passnum();
}

void conn(int startnum, int destnum)
{
	register struct room *start, *dest = NULL;
	register int passlength = 0, turning, turnlength = 0, roomnum;
	int rm, direction;
	if(startnum < destnum) {
		roomnum = startnum;
		direction = roomnum+1 == destnum ? 'r' : 'd';
	} else {
		roomnum = destnum;
		direction = roomnum+1 == startnum ? 'r' : 'd';
	}
	start = &Rooms[roomnum];
	if(direction == 'd') {
		rm = roomnum+3;
		dest = &Rooms[rm];
		Xinc = 0;
		Yinc = 1;
		Start_door.x = start->r_xstart;
		Start_door.y = start->r_ystart;
		Finish_door.x = dest->r_xstart;
		Finish_door.y = dest->r_ystart;
		if((start->r_flags & R_PASSAGE) == 0) {
			do {
				Start_door.x = rnd(start->r_xsize-2)+start->r_xstart+1;
				Start_door.y = start->r_ystart+start->r_ysize-1;
				if((start->r_flags & R_MAZE) == 0) break;
			} while((Places[Start_door.x][Start_door.y].p_flags & P_PASSAGE) == 0);
		}
		if((dest->r_flags & R_PASSAGE) == 0) {
			do {
				Finish_door.x = rnd(dest->r_xsize-2)+dest->r_xstart+1;
				if((dest->r_flags & R_MAZE) == 0) break;
			} while((Places[Finish_door.x][Finish_door.y].p_flags & P_PASSAGE) == 0);
		}
		passlength = abs(Start_door.y-Finish_door.y)-1;
		Yturn = 0;
		Xturn = Start_door.x < Finish_door.x ? 1 : -1;
		turnlength = abs(Start_door.x-Finish_door.x);
	} else if(direction == 'r') {
		rm = roomnum+1;
		dest = &Rooms[rm];
		Xinc = 1;
		Yinc = 0;
		Start_door.x = start->r_xstart;
		Start_door.y = start->r_ystart;
		Finish_door.x = dest->r_xstart;
		Finish_door.y = dest->r_ystart;
		if((start->r_flags & R_PASSAGE) == 0) {
			do {
				Start_door.x = start->r_xstart+start->r_xsize-1;
				Start_door.y = rnd(start->r_ysize-2)+start->r_ystart+1;
				if((start->r_flags & R_MAZE) == 0) break;
			} while((Places[Start_door.x][Start_door.y].p_flags & P_PASSAGE) == 0);
		}
		if((dest->r_flags & R_PASSAGE) == 0) {
			do {
				Finish_door.y = rnd(dest->r_ysize-2)+dest->r_ystart+1;
				if((dest->r_flags & R_MAZE) == 0) break;
			} while((Places[Finish_door.x][Finish_door.y].p_flags & P_PASSAGE) == 0);
		}
		passlength = abs(Start_door.x-Finish_door.x)-1;
		Yturn = Start_door.y < Finish_door.y ? 1 : -1;
		Xturn = 0;
		turnlength = abs(Start_door.y-Finish_door.y);
	}
	turning = rnd(passlength-1)+1;
	if((start->r_flags & R_PASSAGE) == 0) {
		door(start,&Start_door);
	} else {
		putpass(&Start_door);
	}
	if((dest->r_flags & R_PASSAGE) == 0) {
		door(dest,&Finish_door);
	} else {
		putpass(&Finish_door);
	}
	Pass.x = Start_door.x;
	Pass.y = Start_door.y;
	for(; passlength > 0; passlength--) {
		Pass.x += Xinc;
		Pass.y += Yinc;
		if(passlength == turning) {
			while(turnlength--) {
				putpass(&Pass);
				Pass.x += Xturn;
				Pass.y += Yturn;
			}
		}
		putpass(&Pass);
	}
	Pass.x += Xinc;
	Pass.y += Yinc;
	if(Pass.x != Finish_door.x || Pass.y != Finish_door.y) msg("warning, connectivity problem on this level");
}

void putpass(register struct coords *pos)
{
	register struct place *p;
	p = &Places[pos->x][pos->y];
	p->p_flags |= P_PASSAGE;
	if(rnd(10)+1 < Level && rnd(40) == 0) {
		p->p_flags &= ~P_KNOWN;
	} else {
		p->p_ch = PASSAGE;
	}
}

void door(register struct room *r, register struct coords *pos)
{
	register struct place *pdoor;
	r->r_doors[r->r_dct] = *pos;
	r->r_dct++;
	if(r->r_flags & R_MAZE) return;
	pdoor = &Places[pos->x][pos->y];
	if(rnd(10)+1 < Level && rnd(5) == 0) {
		if(pos->y == r->r_ystart || pos->y == r->r_ystart+r->r_ysize-1) {
			pdoor->p_ch = HWALL;
		} else {
			pdoor->p_ch = VWALL;
		}
		pdoor->p_flags &= ~P_KNOWN;
	} else {
		pdoor->p_ch = DOOR;
	}
}

void passnum()
{
	register struct room *rm;
	register int door;
	Pnum = 0;
	Newpnum = FALSE;
	for(rm = Passages; rm < &Passages[PASSAGES]; rm++) rm->r_dct = 0;
	for(rm = Rooms; rm < &Rooms[ROOMS]; rm++) {
		for(door = 0; door < rm->r_dct; door++) {
			Newpnum++;
			numpass(rm->r_doors[door].y,rm->r_doors[door].x);
		}
	}
}

void numpass(register int y, int x)
{
	register unsigned char *flags;
	register struct room *pass;
	char floorch;
	if(x >= 80 || x < 0 || y >= 24 || y <= 0) return;
	flags = &Places[x][y].p_flags;
	if(*flags & PASSNUM) return;
	if(Newpnum) {
		Pnum++;
		Newpnum = FALSE;
	}
	floorch = Places[x][y].p_ch;
	if(floorch == DOOR || ((*flags & P_KNOWN) == 0 && (floorch == VWALL || floorch == HWALL))) {
		pass = &Passages[Pnum];
		pass->r_doors[pass->r_dct].y = y;
		pass->r_doors[pass->r_dct].x = x;
		pass->r_dct++;
	} else {
		if((*flags & P_PASSAGE) == 0) return;
	}
	*flags |= Pnum;
	numpass(y+1,x);
	numpass(y-1,x);
	numpass(y,x+1);
	numpass(y,x-1);
}

