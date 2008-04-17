/*
 *	ROGUE
 *
 *	@(#)command.c	1.1	Strathclyde	10/31/84
 */

# include "param.h"
# include "types.h"
# include "extern.h"
#include <string.h>

static void help();
static void illcom(char c);
static void identify();
static void d_level();
static void u_level();
static int levit_check();
static void current(register struct item *thing,
		    register char *what, char *where);

struct Help {
	char	h_ch;
	char	*h_str;
	char	h_include;
} Helpstr[] = {
{	'?',	"\tprints help",				TRUE},
{	'/',	"\tidentify object",				TRUE},
{	'h',	"\tleft",					TRUE},
{	'j',	"\tdown",					TRUE},
{	'k',	"\tup",						TRUE},
{	'l',	"\tright",					TRUE},
{	'y',	"\tup & left",					TRUE},
{	'u',	"\tup & right",					TRUE},
{	'b',	"\tdown & left",				TRUE},
{	'n',	"\tdown & right",				TRUE},
{	'H',	"\trun left",					FALSE},
{	'J',	"\trun down",					FALSE},
{	'K',	"\trun up",					FALSE},
{	'L',	"\trun right",					FALSE},
{	'Y',	"\trun up & left",				FALSE},
{	'U',	"\trun up & right",				FALSE},
{	'B',	"\trun down & left",				FALSE},
{	'N',	"\trun down & right",				FALSE},
{	CTLH,	"\trun left until adjacent",			FALSE},
{	CTLJ,	"\trun down until adjacent",			FALSE},
{	CTLK,	"\trun up until adjacent",			FALSE},
{	CTLL,	"\trun right until adjacent",			FALSE},
{	CTLY,	"\trun up & left until adjacent",		FALSE},
{	CTLU,	"\trun up & right until adjacent",		FALSE},
{	CTLB,	"\trun down & left until adjacent",		FALSE},
{	CTLN,	"\trun down & right until adjacent",		FALSE},
{	0,	"\t<SHIFT><dir>: run that way",			TRUE},
{	0,	"\t<CTRL><dir>: run till adjacent",		TRUE},
{	'f',	"<dir>\tfight till death or near death",	TRUE},
{	'F',	"<dir>\tfight till either of you dies",		TRUE},
{	't',	"<dir>\tthrow something",			TRUE},
{	'm',	"<dir>\tmove onto without picking up",		TRUE},
{	'z',	"<dir>\tzap a wand in a direction",		TRUE},
{	'^',	"<dir>\tidentify trap type",			TRUE},
{	's',	"\tsearch for trap/secret door",		TRUE},
{	'>',	"\tgo down a staircase",			TRUE},
{	'<',	"\tgo up a staircase",				TRUE},
{	'.',	"\trest for a while",				TRUE},
{	'i',	"\tinventory",					TRUE},
{	'I',	"\tinventory single item",			TRUE},
{	'q',	"\tquaff potion",				TRUE},
{	'r',	"\tread paper",					TRUE},
{	'e',	"\teat food",					TRUE},
{	'w',	"\twield a weapon",				TRUE},
{	'W',	"\twear armor",				TRUE},
{	'T',	"\ttake armor off",				TRUE},
{	'P',	"\tput on ring",				TRUE},
{	'R',	"\tremove ring",				TRUE},
{	'd',	"\tdrop object",				TRUE},
{	'c',	"\tcall object",				TRUE},
{	')',	"\tprint current weapon",			TRUE},
{	']',	"\tprint current armor",			TRUE},
{	'=',	"\tprint current rings",			TRUE},
{	'@',	"\tprint current stats",			TRUE},
{	'D',	"\trecall what's been discovered",		TRUE},
{	'o',	"\texamine/set options",			TRUE},
{	CTLR,	"\tredraw screen",				TRUE},
{	CTLP,	"\trepeat last message",			TRUE},
{	ESC,	"\tcancel command",				TRUE},
{	'!',	"\tshell escape",				TRUE},
{	'S',	"\tsave game",					TRUE},
{	'Q',	"\tquit",					TRUE},
{	'v',	"\tprint version number",			FALSE},
{	0,	0,						0}
};

struct Help Identstr[] = {
{	VWALL,		"wall of a room",		0},
{	HWALL,		"wall of a room",		0},
{	GOLD,		"gold",				0},
{	STAIRCASE,	"a staircase",			0},
{	DOOR,		"door",				0},
{	FLOOR,		"room floor",			0},
{	PLAYER,		"you",				0},
{	PASSAGE,	"passage",			0},
{	TRAP,		"trap",				0},
{	POTION,		"potion",			0},
{	SCROLL,		"scroll",			0},
{	FOOD,		"food",				0},
{	WEAPON,		"weapon",			0},
{	' ',		"solid rock",			0},
{	ARMOUR,		"armor",			0},
{	AMULET,		"the Amulet of Yendor",		0},
{	RING,		"ring",				0},
{	STICK,		"wand or staff",		0},
{	0,		0,				0}
};

# ifdef DEBUG
int	Pick_this = -1;
int	Pick_type = -1;
char	Pick_save = 0;

extern bool	No_death;
# endif 

void command()
{
	register char *flags;
	register struct being *monst;
	char com;
	static char Old_dir;
	static bool First_rep;
	static char Last_com;
	Take = 0;
	if(No_command == 0) {
		if(Running || To_death) {
			com = Runch;
		} else if(Count) {
			com = Old_dir;
		} else {
			com = readchar();
			Move_on = FALSE;
			if(Mpos) msg("");
		}
	} else {
		com = '.';
	}
	if(No_command) {
		if(--No_command == 0) {
			Player.b_flags |= MOBILE;
			msg("you can move again");
		}
	} else {
		First_rep = FALSE;
		if(isdigit(com)) {
			Count = 0;
			First_rep = TRUE;
			while(isdigit(com)) {
				Count = Count*10+(com-'0');
				com = readchar();
			}
			Old_dir = com;
			switch(com) {
			default:
				Count = 0;
			case CTLH:
			case CTLJ:
			case CTLK:
			case CTLL:
			case CTLY:
			case CTLU:
			case CTLB:
			case CTLN:
			case 'h':
			case 'j':
			case 'k':
			case 'l':
			case 'y':
			case 'u':
			case 'b':
			case 'n':
			case 'H':
			case 'J':
			case 'K':
			case 'L':
			case 'Y':
			case 'U':
			case 'B':
			case 'N':
			case '.':
			case 's':
			case 'm':
			case 'C':
			case 'z':
			case 'q':
			case 'r':
			case 'I':
# ifdef DEBUG
			case 'A':
			case 'a':
				break;
			case 'G':
				if(Count) {
					Level = Count-1;
					Count = 0;
				}
# endif 
				break;
			}
		}
		if(Count && !Running) --Count;
	obey:
		switch(com) {
# ifdef DEBUG
		case 'A':
			for(;;) {
				switch(Pick_save ? Pick_save : (Pick_save = readchar())) {
				case GOLD:
					money(AU);
					goto brk;
				case POTION:
					Pick_this = T_POTION;
					break;
				case SCROLL:
					Pick_this = T_SCROLL;
					break;
				case FOOD:
					Pick_this = T_FOOD;
					break;
				case WEAPON:
					Pick_this = T_WEAPON;
					break;
				case ARMOUR:
					Pick_this = T_ARMOUR;
					break;
				case RING:
					Pick_this = T_RING;
					break;
				case STICK:
					Pick_this = T_STICK;
					break;
				default:
					Pick_save = 0;
					continue;
				}
				break;
			}
		case 'a':
			add_pack(new_thing(),FALSE);
			After = FALSE;
		brk:
			if(Count == 0) Pick_save = 0;
			break;
		case 'Z':
			Pick_type = readchar();
			if(islower(Pick_type)) {
				Pick_type -= 'a';
			} else {
				Pick_type = -1;
			}
			After = FALSE;
			break;
		case 'M':
			After = FALSE;
			show_level(TRUE);
			turn_see(FALSE);
			break;
		case 'G':
			After = FALSE;
			Level++;
			Seenstairs = FALSE;
			new_level();
			break;
		case 'C':
			After = FALSE;
			No_death = FALSE;
			break;
# endif 
		default:
			After = FALSE;
			illcom(com);
			break;
		case '!':
			shell();
			break;
		case '.':
			break;
		case 'h':
			do_move(0,-1);
			break;
		case 'j':
			do_move(1,0);
			break;
		case 'k':
			do_move(-1,0);
			break;
		case 'l':
			do_move(0,1);
			break;
		case 'y':
			do_move(-1,-1);
			break;
		case 'u':
			do_move(-1,1);
			break;
		case 'b':
			do_move(1,-1);
			break;
		case 'n':
			do_move(1,1);
			break;
		case 'H':
			do_run('h');
			break;
		case 'J':
			do_run('j');
			break;
		case 'K':
			do_run('k');
			break;
		case 'L':
			do_run('l');
			break;
		case 'Y':
			do_run('y');
			break;
		case 'U':
			do_run('u');
			break;
		case 'B':
			do_run('b');
			break;
		case 'N':
			do_run('n');
			break;
		case CTLH:
		case CTLJ:
		case CTLK:
		case CTLL:
		case CTLY:
		case CTLU:
		case CTLB:
		case CTLN:
			if((Player.b_flags & BLIND) == 0) {
				Door_stop = TRUE;
				Firstmove = TRUE;
			}
			if(Count && First_rep == 0) {
				com = Last_com;
			} else {
				com += ('H'-CTLH);
				Last_com = com;
			}
			goto obey;
		case 'F':
			Kamikaze = TRUE;
		case 'f':
			if(get_dir()) {
				Delta.y += MYY;
				Delta.x += MYX;
				if((monst = Places[Delta.x][Delta.y].p_being)) {
					if(see_monst(monst) || (Player.b_flags & DETECTING)) {
						if(!diag_ok(&ME,&Delta)) break;
						To_death = TRUE;
						Max_hit = 0;
						monst->b_flags |= FIGHTING;
						com = Dir_ch;
						Runch = com;
						goto obey;
					}
				}
				if(Verbose) addmsg("I see ");
				msg("no monster there");
			}
			After = FALSE;
			break;
		case ' ':
			After = FALSE;
			break;
		case 't':
			if(get_dir()) missile(Delta.y,Delta.x);
			else After = FALSE;
			break;
		case 'q':
			quaff();
			break;
		case 'Q':
			After = FALSE;
			Q_comm = TRUE;
			quit();
			Q_comm = FALSE;
			break;
		case 'i':
			After = FALSE;
			inventory(Player.b_pack,0);
			break;
		case 'I':
			After = FALSE;
			picky_inven();
			break;
		case 'd':
			drop();
			break;
		case 'r':
			read_scroll();
			break;
		case 'e':
			eat();
			break;
		case 'w':
			wield();
			break;
		case 'W':
			wear();
			break;
		case 'T':
			take_off();
			break;
		case 'P':
			ring_on();
			break;
		case 'R':
			ring_off();
			break;
		case 'o':
			option();
			break;
		case 'c':
			call();
			break;
		case '>':
			After = FALSE;
			d_level();
			break;
		case '<':
			After = FALSE;
			u_level();
			break;
		case '?':
			After = FALSE;
			help();
			break;
		case '/':
			After = FALSE;
			identify();
			break;
		case 's':
			search();
			break;
		case 'z':
			if(get_dir()) do_zap();
			else After = FALSE;
			break;
		case 'D':
			After = FALSE;
			discovered();
			break;
		case CTLP:
			After = FALSE;
			msg(Huh);
			break;
		case CTLR:
			After = FALSE;
			clearok(curscr,TRUE);
			wrefresh(curscr);
			break;
		case 'v':
			After = FALSE;
			msg("version %s. (ARB was here)",Release);
			break;
		case 'S':
			After = FALSE;
			save_game();
			break;
		case '^':
			After = FALSE;
			if(get_dir()) {
				Delta.y += MYY;
				Delta.x += MYX;
				flags = &Places[Delta.x][Delta.y].p_flags;
				if(Places[Delta.x][Delta.y].p_ch == TRAP) {
					if(Player.b_flags & HALLUCINATING) {
						msg(Tr_name[rnd(TRAPS)]);
					} else {
						msg(Tr_name[*flags & TMASK]);
						*flags |= P_DISCOVERED;
					}
				} else {
					msg("no trap there");
				}
			}
			break;
		case ESC:
			Door_stop = FALSE;
			Count = 0;
			After = FALSE;
			break;
		case 'm':
			Move_on = TRUE;
			if(get_dir()) {
				com = Dir_ch;
				Old_dir = Dir_ch;
				goto obey;
			} else {
				After = FALSE;
			}
			break;
		case 'p':
			pick_up();
			After = FALSE;
			break;
		case ')':
			current(Cur_weapon,"wielding",0);
			break;
		case ']':
			current(Cur_armor,"wearing",0);
			break;
		case '=':
			current(Cur_ring[LEFT],"wearing",Terse ? "(L)" : "on left hand");
			current(Cur_ring[RIGHT],"wearing",Terse ? "(R)" : "on right hand");
			break;
		case '@':
			Stat_msg = TRUE;
			status();
			Stat_msg = FALSE;
			After = FALSE;
		}
		if(!Running) Door_stop = FALSE;
	}
	if(Take) {
		if(!Pick_up && Take != GOLD) Move_on = TRUE;
		pick_up();
	}
}

void illcom(char c)
{
	Save_msg = FALSE;
	Count = 0;
	msg("illegal command '%s'",unctrl(c));
	Save_msg = TRUE;
}

void search()
{
	register int y, x;
	register unsigned char *flags;
	register int ylimit, xlimit, chance;
	bool found;
	if(Player.b_flags & BLIND) return;
	ylimit = MYY+1;
	xlimit = MYX+1;
	chance = Player.b_flags & HALLUCINATING ? 3 : 0;
	found = FALSE;
	for(y = (MYY-1)<0?0:(MYY-1); y <= ylimit; y++) {
		for(x = (MYX-1)<0 ? 0 : (MYX-1); x <= xlimit; x++) {
			if(y != MYY || x != MYX) {
				flags = &Places[x][y].p_flags;
				if(*flags & P_KNOWN) continue;
				switch(Places[x][y].p_ch) {
				case HWALL:
				case VWALL:
					if(rnd(chance+5)) continue;
					Places[x][y].p_ch = DOOR;
					break;
				case FLOOR:
					if(rnd(chance+2)) continue;
					Places[x][y].p_ch = TRAP;
					if(Verbose) addmsg("you found ");
					if(Player.b_flags & HALLUCINATING) {
						msg(Tr_name[rnd(TRAPS)]);
					} else {
						msg(Tr_name[*flags & TMASK]);
						*flags |= P_DISCOVERED;
					}
					break;
				case ' ':
					if(rnd(chance+3)) continue;
					Places[x][y].p_ch = PASSAGE;
					break;
				default:
					continue;
				}
				found = TRUE;
				*flags |= P_KNOWN;
				Count = 0;
				Running = FALSE;
			}
		}
	}
	if(found) look(FALSE);
}

void help()
{
	register struct Help *hstr;
	register int lines, lineno;
	char helpch;
	msg("character you want help for (* for all): ");
	helpch = readchar();
	Mpos = 0;
	if(helpch != '*') {
		move(0,0);
		for(hstr = Helpstr; hstr->h_str; hstr++) {
			if(hstr->h_ch == helpch) {
				Lower_msg = TRUE;
				msg("%s%s",unctrl(helpch),hstr->h_str);
				Lower_msg = FALSE;
				return;
			}
		}
		msg("unknown character '%s'",unctrl(helpch));
		return;
	}
	lines = 0;
	for(hstr = Helpstr; hstr->h_str; hstr++) {
		if(hstr->h_include) lines++;
	}
	if(lines & 1) lines++;
	lines /= 2;
	wclear(Hw);
	lineno = 0;
	for(hstr = Helpstr; hstr->h_str; hstr++) {
		if(hstr->h_include) {
			wmove(Hw,lineno % lines,lineno >= lines ? COLS/2 : 0);
			if(hstr->h_ch) waddstr(Hw,unctrl(hstr->h_ch));
			waddstr(Hw,hstr->h_str);
			lineno++;
		}
	}
	wmove(Hw,LINES-1,0);
	waddstr(Hw,Pspace);
	wrefresh(Hw);
	wait_for(' ');
	clearok(stdscr,TRUE);
	wclear(Hw);
	touchwin(stdscr);
	refresh();
}

void identify()
{
	register int ident;
	register struct Help *istr;
	register char *ans;
	msg("what do you want identified? ");
	ident = readchar();
	Mpos = 0;
	if(ident == ESC) {
		msg("");
		return;
	}
	if(isupper(ident)) {
		ans = Monsters[mindex(ident)].m_name;
	} else {
		ans = "unknown character";
		for(istr = Identstr; istr->h_ch; istr++) {
			if(istr->h_ch == ident) {
				ans = istr->h_str;
				break;
			}
		}
	}
	msg("'%s': %s",unctrl(ident),ans);
}

void d_level()
{
	if(levit_check()) return;
	if(Places[MYX][MYY].p_ch != STAIRCASE) {
		msg("I see no way down");
	} else {
		if(Show_level) {
			show_level(TRUE);
			msg(Pspace);
			wait_for(' ');
		}
		Level++;
		Seenstairs = FALSE;
		new_level();
	}
}

void u_level()
{
	if(levit_check()) return;
	if(Places[MYX][MYY].p_ch == STAIRCASE) {
		if(Amulet) {
			if(--Level == 0) total_winner();
			new_level();
			msg("you feel a wrenching sensation in your gut");
		} else {
			msg("your way is magically blocked");
		}
	} else {
		msg("I see no way up");
	}
}

int levit_check()
{
	if((Player.b_flags & LEVITATED) == 0) return(FALSE);
	msg("You can't.  You're floating off the ground!");
	return(TRUE);
}

void call()
{
	register struct item *thing;
	register struct info *inf = NULL;
	register char **called, *oldname;
	register bool *known;

	if((thing = get_item("call",-1)) == 0) return;
	switch(thing->i_item) {
	case RING:
		inf = &Ring_info[thing->i_type];
		oldname = R_stones[thing->i_type];
		goto lcb;
	case POTION:
		inf = &Pot_info[thing->i_type];
		oldname = P_colours[thing->i_type];
		goto lcb;
	case SCROLL:
		inf = &Scr_info[thing->i_type];
		oldname = S_names[thing->i_type];
		goto lcb;
	case STICK:
		inf = &Ws_info[thing->i_type];
		oldname = Ws_made[thing->i_type];
	lcb:
		known = &inf->inf_known;
		called = &inf->inf_called;
		if(*called) oldname = *called;
		break;
	case FOOD:
		msg("you can't call that anything");
		return;
	default:
		called = &thing->i_called;
		known = 0;
		oldname = thing->i_called;
	}
	if(known && *known) {
		msg("that has already been identified");
		return;
	}
	if(oldname && oldname == inf->inf_called) {
		if(Verbose) addmsg("Was ");
		msg("called \"%s\"",oldname);
	}
	if(Terse) {
		msg("call it: ");
	} else {
		msg("what do you want to call it? ");
	}
	if (oldname)
		strcpy(Prbuf,oldname);
	else	Prbuf[0] = '\0';
	if(get_str(Prbuf,stdscr) == 0) {
		if(*called) free(*called);
		*called = malloc(strlen(Prbuf)+1);
		strcpy(*called,Prbuf);
	}
}

void current(register struct item *thing,
	     register char *what, char *where)
{
	After = FALSE;
	if(thing) {
		if(Verbose) addmsg("you are %s (",what);
		Inv_describe = FALSE;
		addmsg("%c) %s",thing->i_packchar,inv_name(thing,TRUE));
		Inv_describe = TRUE;
		if(where) addmsg(" %s",where);
	} else {
		if(Verbose) addmsg("you are ");
		addmsg("%s nothing",what);
		if(where) addmsg(" %s",where);
	}
	endmsg();
}

