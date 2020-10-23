/*
 *	ROGUE
 *
 *	@(#)thing.c	1.1	Strathclyde	10/31/84
 */

# include "param.h"
# include "types.h"
# include "extern.h"

int	Line_cnt = 0;
bool	Newpage = FALSE;

int	Win_xstart = -1;

char *Lastfmt;
char *Lastarg;

struct item	Dummy_item;

short	Order[18];
static void nameit(register struct item *thing,
	    char *item_name, char *desc,
	    register struct info *inf,
	    char *(*num)());
int pick_one(register struct info *inf,int nthings);
void print_disc(char item);
void set_order(short *list, int size);

char *inv_name(register struct item *thing,
	       bool lower)
{
	register char *bufp;
	register struct info *inf;
	register char *name;
	register int type;
	bufp = Prbuf;
	type = thing->i_type;
	switch(thing->i_item) {
	case POTION:
		nameit(thing,"potion",P_colours[type],&Pot_info[type],nullstr);
		break;
	case RING:
		nameit(thing,"ring",R_stones[type],&Ring_info[type],ring_num);
		break;
	case STICK:
		nameit(thing,Ws_type[type],Ws_made[type],&Ws_info[type],charge_str);
		break;
	case SCROLL:
		if(thing->i_number == 1) {
			strcpy(bufp,"A scroll ");
			bufp = &Prbuf[9];
		} else {
			sprintf(bufp,"%d scrolls ",thing->i_number);
			bufp = &Prbuf[strlen(Prbuf)];
		}
		inf = &Scr_info[type];
		if(inf->inf_known) {
			sprintf(bufp,"of %s",inf->inf_name);
		} else if(inf->inf_called) {
			sprintf(bufp,"called %s",inf->inf_called);
		} else {
			sprintf(bufp,"titled '%s'",S_names[type]);
		}
		break;
	case FOOD:
		if(type == 1) {
			if(thing->i_number == 1) {
				sprintf(bufp,"A%s %s",vowelstr(Fruit),Fruit);
			} else {
				sprintf(bufp,"%d %ss",thing->i_number,Fruit);
			}
		} else {
			if(thing->i_number == 1) {
				strcpy(bufp,"Some food");
			} else {
				sprintf(bufp,"%d rations of food",thing->i_number);
			}
		}
		break;
	case WEAPON:
		name = Weap_info[type].inf_name;
		if(thing->i_number > 1) {
			sprintf(bufp,"%d ",thing->i_number);
		} else {
			sprintf(bufp,"A%s ",vowelstr(name));
		}
		bufp = &Prbuf[strlen(Prbuf)];
		if(thing->i_flags & KNOWN) {
			sprintf(bufp,"%s %s",num(thing->i_hitplus,thing->i_damplus,WEAPON),name);
		} else {
			sprintf(bufp,"%s",name);
		}
		if(thing->i_number > 1) strcat(bufp,"s");
		if(thing->i_called) {
			bufp = &Prbuf[strlen(Prbuf)];
			sprintf(bufp," called %s",thing->i_called);
		}
		break;
	case ARMOUR:
		name = Arm_info[type].inf_name;
		if(thing->i_flags & KNOWN) {
			sprintf(bufp,"%s %s [",num(A_class[type]-thing->i_value,0,ARMOUR),name);
			if(Verbose) strcat(bufp,"protection ");
			bufp = &Prbuf[strlen(Prbuf)];
			sprintf(bufp,"%d]",10-thing->i_value);
		} else {
			sprintf(bufp,"%s",name);
		}
		if(thing->i_called) {
			bufp = &Prbuf[strlen(Prbuf)];
			sprintf(bufp," called %s",thing->i_called);
		}
		break;
	case AMULET:
		strcpy(bufp,"The Amulet of Yendor");
		break;
	case GOLD:
		sprintf(Prbuf,"%d Gold pieces",thing->i_value);
		break;
	}
	if(Inv_describe) {
		if(thing == Cur_armor) strcat(bufp," (being worn)");
		if(thing == Cur_weapon) strcat(bufp," (weapon in hand)");
		if(thing == Cur_ring[LEFT]) {
			strcat(bufp," (on left hand)");
		} else if(thing == Cur_ring[RIGHT]) {
			strcat(bufp," (on right hand)");
		}
	}
	if(lower && isupper(Prbuf[0])) {
		Prbuf[0] += ('a'-'A');
	} else if(!lower && islower(Prbuf[0])) {
		Prbuf[0] -= ('a'-'A');
	}
	Prbuf[MAXSTR-1] = 0;
	return(Prbuf);
}

void drop()
{
	register struct item *thing;
	char floorch;
	floorch = Places[MYX][MYY].p_ch;
	if(floorch != FLOOR && floorch != PASSAGE) {
		After = FALSE;
		msg("there is something there already");
		return;
	}
	if((thing = get_item("drop",0)) == 0) return;
	if(!dropcheck(thing)) return;
	thing = leave_pack(thing,TRUE,thing->i_item != POTION && thing->i_item != SCROLL && thing->i_item != FOOD);
	_attach(&Lvl_obj,thing);
	Places[MYX][MYY].p_ch = thing->i_item;
	Places[MYX][MYY].p_flags |= P_DROPPED;
	thing->i_coords = ME;
	if(thing->i_item == AMULET) Amulet = FALSE;
	msg("dropped %s",inv_name(thing,TRUE));
}

int dropcheck(register struct item *thing)
{
	if(thing == 0) return(TRUE);
	if(thing == Cur_armor || thing == Cur_weapon || thing == Cur_ring[LEFT] || thing == Cur_ring[RIGHT]) {
		if(thing->i_flags & CURSED) {
			msg("you can't.  It appears to be cursed");
			return(FALSE);
		}
		if(thing == Cur_weapon) {
			Cur_weapon = 0;
		} else if(thing == Cur_armor) {
			waste_time();
			Cur_armor = 0;
		} else {
			Cur_ring[thing == Cur_ring[LEFT] ? LEFT : RIGHT] = 0;
			switch(thing->i_type) {
			case R_ADD_STRENGTH:
				chg_str(-thing->i_value);
				break;
			case R_SEE_INVISIBLE:
				unsee();
				extinguish(unsee);
			}
		}
	}
	return(TRUE);
}

struct item *new_thing()
{
	register struct item *thing;
	register int cursenc;
	thing = new_item();
	thing->i_hitplus = 0;
	thing->i_damplus = 0;
	thing->i_tpower = "0x0";
	thing->i_hpower = thing->i_tpower;
	thing->i_value = DEF_VALUE;
	thing->i_number = 1;
	thing->i_group = 0;
	thing->i_flags = 0;
	switch(No_food > 3 ? T_FOOD : pick_one(Things,THINGS)) {
	case T_POTION:
		thing->i_item = POTION;
		thing->i_type = pick_one(Pot_info,POTIONS);
		break;
	case T_SCROLL:
		thing->i_item = SCROLL;
		thing->i_type = pick_one(Scr_info,SCROLLS);
		break;
	case T_FOOD:
		thing->i_item = FOOD;
		No_food = 0;
		if(rnd(10)) {
			thing->i_type = 0;
		} else {
			thing->i_type = 1;
		}
		break;
	case T_WEAPON:
		init_weapon(thing,pick_one(Weap_info,WEAPONS));
		if((cursenc = rnd(100)) < 10) {
			thing->i_flags |= CURSED;
			thing->i_hitplus -= rnd(3)+1;
		} else if(cursenc < 15) {
			thing->i_hitplus += rnd(3)+1;
		}
		break;
	case T_ARMOUR:
		thing->i_item = ARMOUR;
		thing->i_type = pick_one(Arm_info,ARMOURS);
		thing->i_value = A_class[thing->i_type];
		if((cursenc = rnd(100)) < 20) {
			thing->i_flags |= CURSED;
			thing->i_value += rnd(3)+1;
		} else if(cursenc < 28) {
			thing->i_value -= rnd(3)+1;
		}
		break;
	case T_RING:
		thing->i_item = RING;
		thing->i_type = pick_one(Ring_info,RINGS);
		switch(thing->i_type) {
		case R_PROTECTION:
		case R_ADD_STRENGTH:
		case R_DEXTERITY:
		case R_INCREASE_DAMAGE:
			if((thing->i_value = rnd(3)) == 0) thing->i_value = -1;
			break;
		case R_AGGRAVATE_MONSTER:
		case R_TELEPORTATION:
			thing->i_flags |= CURSED;
		}
		break;
	case T_STICK:
		thing->i_item = STICK;
		thing->i_type = pick_one(Ws_info,STICKS);
		fix_stick(thing);
	}
	return(thing);
}

int pick_one(register struct info *inf,int nthings)
{
	register struct info *infend, *first;
	register int pct;
# ifdef DEBUG
	extern int Pick_this, Pick_type;
	if(inf == Things) {
		if(Pick_this >= 0) {
			pct = Pick_this;
			Pick_this = -1;
			return(pct);
		}
	} else {
		if(Pick_type >= 0) return(Pick_type);
	}
# endif 
	first = inf;
	infend = &inf[nthings];
	pct = rnd(100);
	while(inf < infend) {
		if(pct < inf->inf_prob) break;
		inf++;
	}
	if(inf == infend) inf = first;
	return(inf-first);
}

void discovered()
{
	char type;
	bool got;
	do {
		got = FALSE;
		if(Verbose) addmsg("for ");
		addmsg("what type");
		if(Verbose) addmsg(" of object do you want a list");
		msg("? (* for all)");
		type = readchar();
		switch(type) {
		case ESC:
			msg("");
			return;
		case POTION:
		case SCROLL:
		case RING:
		case STICK:
		case GOLD:
			got = TRUE;
			break;
		default:
			if(Terse) {
				msg("Not a type");
			} else {
				msg("Please type one of %c%c%c%c (ESCAPE to quit)",POTION,SCROLL,RING,STICK);
			}
		}
	} while(!got);
	if(type == '*') {
		print_disc(POTION);
		add_line("", 0);
		print_disc(SCROLL);
		add_line("", 0);
		print_disc(RING);
		add_line("", 0);
		print_disc(STICK);
	} else {
		print_disc(type);
	}
	end_line();
}

void print_disc(char item)
{
	register struct info *inf = NULL;
	register int type, ninfo = 0, count;
	switch(item) {
	case SCROLL:
		ninfo = SCROLLS;
		inf = Scr_info;
		break;
	case POTION:
		ninfo = POTIONS;
		inf = Pot_info;
		break;
	case RING:
		ninfo = RINGS;
		inf = Ring_info;
		break;
	case STICK:
		ninfo = STICKS;
		inf = Ws_info;
	}
	set_order(Order,ninfo);
	Dummy_item.i_number = 1;
	Dummy_item.i_flags = 0;
	count = 0;
	for(type = 0; type < ninfo; type++) {
		if(inf[Order[type]].inf_known || inf[Order[type]].inf_called) {
			Dummy_item.i_item = item;
			Dummy_item.i_type = Order[type];
			add_line("%s",inv_name(&Dummy_item,FALSE));
			count++;
		}
	}
	if(count == 0) add_line(nothing(item), 0);
}

void set_order(short *list, int size)
{
	register int i, sel, swap;
	for(i = 0; i < size; i++) list[i] = i;
	for(i = size; i > 0; i--) {
		sel = rnd(i);
		swap = list[i-1];
		list[i-1] = list[sel];
		list[sel] = swap;
	}
}
void doaddX(char *fmt, ...) 
{
	va_list ap;
	va_start(ap, fmt);
	doadd(fmt,ap);
	va_end(ap);
}

int add_line(char *line, char *arg)
{
	register WINDOW *win, *swin;
	register int x, y;
	register char *cont;

	cont = Pspace;
	if(Line_cnt == 0) {
		wclear(Hw);
		if(Inv_type == 1) Mpos = 0;
	}
	if(Inv_type == 1) {
		if(line && *line) {
		    doaddX(line, arg);
		    if(endmsg() == ESC) {
			return(ESC);
		    }
		}
		Line_cnt++;
	} else {
		if(Win_xstart < 0) Win_xstart = strlen(cont);
		if(Line_cnt >= LINES-1 || line == 0) {
			if(Inv_type == 0 && line == 0 && !Newpage) {
				msg("");
				refresh();

				win = newwin(0,0,0,COLS-Win_xstart-3);
				swin = subwin(win,0,0,0,COLS-Win_xstart-2);
				touchwin(Hw);
				copywin(Hw, swin, 0, 0, 0, 0, Line_cnt+1, COLS-Win_xstart+5, 0); 
//				overwrite(Hw,swin);
				wmove(win,Line_cnt,1);
				waddstr(win,cont);
				touchwin(swin);
				wrefresh(swin);
				touchwin(win);
				wrefresh(win);
				wait_for(' ');
#if 0
/*				if(CE) { */
					werase(win);
					leaveok(win,TRUE);
					wrefresh(win);
/*				} */
#endif
				delwin(win);
				touchwin(stdscr);
			} else {
				wmove(Hw,LINES-1,0);
				waddstr(Hw,cont);
				wrefresh(Hw);
				wait_for(' ');
				clearok(curscr,TRUE);
				wclear(Hw);
			}
			Newpage = TRUE;
			Line_cnt = 0;
			Win_xstart = strlen(cont);
		}
		if(line && (Line_cnt || *line)) {
		        wmove(Hw, Line_cnt++, 0);
			wprintw(Hw,line,arg);
			getyx(Hw,y,x);
			if(Win_xstart < x) Win_xstart = x;
			Lastfmt = line;
			Lastarg = arg;
		}
	}
	return(-28);
}

void end_line()
{
	if(Inv_type != 1) {
		if(Line_cnt == 1 && !Newpage) {
			Mpos = 0;
			msg(Lastfmt,Lastarg);
		} else {
			add_line(0, 0);
		}
	}
	Line_cnt = 0;
	Newpage = FALSE;
}

char *nothing(char item)
{
	register char *bufp, *tname;
	sprintf(Prbuf,Terse ? "Nothing" : "Haven't discovered anything");
	if(item != '*') {
		bufp = &Prbuf[strlen(Prbuf)];
		switch(item) {
		case POTION:
			tname = "potion";
			break;
		case SCROLL:
			tname = "scroll";
			break;
		case RING:
			tname = "ring";
			break;
		case STICK:
			tname = "stick";
			break;
		default:
		    tname = "???";
		    break;
		}
		sprintf(bufp," about any %ss",tname);
	}
	return(Prbuf);
}

void nameit(register struct item *thing,
	    char *item_name, char *desc,
	    register struct info *inf,
	    char *(*num)())
{
	register char *bufp;
	if(inf->inf_known || inf->inf_called) {
		if(thing->i_number == 1) {
			sprintf(Prbuf,"A %s ",item_name);
		} else {
			sprintf(Prbuf,"%d %ss ",thing->i_number,item_name);
		}
		bufp = &Prbuf[strlen(Prbuf)];
		if(inf->inf_known) {
			sprintf(bufp,"of %s%s(%s)",inf->inf_name,num(thing),desc);
		} else if(inf->inf_called) {
			sprintf(bufp,"called %s%s(%s)",inf->inf_called,num(thing),desc);
		}
	} else if(thing->i_number == 1) {
		sprintf(Prbuf,"A%s %s %s",vowelstr(desc),desc,item_name);
	} else {
		sprintf(Prbuf,"%d %s %ss",thing->i_number,desc,item_name);
	}
}

/* ARGSUSED */
char *nullstr(struct item *junk)
{
	return("");
}

