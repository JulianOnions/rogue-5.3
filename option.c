/*
 *	ROGUE
 *
 *	@(#)option.c	1.1	Strathclyde	10/31/84
 */

# include "param.h"
# include "types.h"
# include "extern.h"

static void pr_optname(register struct option *opt);

static void put_bool(void *b);
static int get_bool(void *b, WINDOW *win);

static void put_inv_t(void *invt);
static int get_inv_t(void *invt, WINDOW *win);

static void put_str(void *str);
int get_str(void *str, WINDOW *win);

void get_sf(void *sf, WINDOW *win);

struct option optlist[OPTIONS] = {
{	"terse",	"Terse output",				&Terse,			put_bool,	get_bool},
{	"flush",	"Flush typeahead during battle",	&Fight_flush,		put_bool,	get_bool},
{	"jump",		"Show position only at end of run",	&Jump,			put_bool,	get_bool},
{	"seefloor",	"Show the lamp-illuminated floor",	&See_floor,		put_bool,	get_bool},
{	"passgo",	"Follow turnings in passageways",	&Passgo,		put_bool,	get_bool},
{	"pickup",	"Pick up objects you move onto",	&Pick_up,		put_bool,	get_bool},
{	"showlevel",	"Show level before going down stairs",	&Show_level,		put_bool,	get_bool},
{	"tombstone",	"Print out tombstone when killed",	&Tombstone,		put_bool,	get_bool},
{	"inven",	"Inventory style",			(char *)(&Inv_type),	put_inv_t,	get_inv_t},
{	"name",		"Name",					Whoami,			put_str,	get_str},
{	"fruit",	"Fruit",				Fruit,			put_str,	get_str},
    {	"file",		"Save file",				File_name,		put_str,	get_str}
};

void option()
{
	register struct option *opt;
	register int ret;
	wclear(Hw);
	for(opt = optlist; opt < &optlist[OPTIONS]; opt++) {
		pr_optname(opt);
		(opt->o_put)(opt->o_arg);
		waddch(Hw,'\n');
	}
	wmove(Hw,0,0);
	for(opt = optlist; opt < &optlist[OPTIONS]; opt++) {
		pr_optname(opt);
		if((ret = (opt->o_get)(opt->o_arg,Hw)) == 0) continue;
		if(ret == 1) break;
		if(opt > optlist) {
			wmove(Hw,opt-optlist-1,0);
			opt -= 2;
		} else {
			putchar(BEL);
			wmove(Hw,0,0);
			opt--;
		}
	}
	wmove(Hw,LINES-1,0);
	waddstr(Hw,Pspace);
	wrefresh(Hw);
	wait_for(' ');
	clearok(curscr, TRUE);
	wclear(Hw);
	touchwin(stdscr);
	After = FALSE;
}

void pr_optname(register struct option *opt)
{
	wprintw(Hw,"%s (\"%s\"): ",opt->o_desc,opt->o_name);
}

void put_bool(void *v)
{
    bool  *b = (bool *)v;
	waddstr(Hw,*b ? "True" : "False");
}

void put_str(void *v)
{
    char  *str = (char *)v;
	waddstr(Hw,str);
}

void put_inv_t(void *v)
{
    int *invt = (int *)v;
	waddstr(Hw,Inv_t_name[*invt]);
}

int get_bool(void *v, WINDOW *win)
{
	register int y, x;
	bool *b = (bool *)v;
	bool not_got = TRUE;
	getyx(win,y,x);
	waddstr(win,*b ? "True" : "False");
	while(not_got) {
		wmove(win,y,x);
		wrefresh(win);
		switch(readchar()) {
		case 't':
		case 'T':
			*b = TRUE;
			not_got = FALSE;
			break;
		case 'f':
		case 'F':
			*b = FALSE;
		case '\r':
		case '\n':
			not_got = FALSE;
			break;
		case ESC:
			return(1);
		case '-':
			return(2);
		default:
			wmove(win,y,x+10);
			waddstr(win,"(T or F)");
		}
	}
	wmove(win,y,x);
	waddstr(win,*b ? "True" : "False");
	waddch(win,'\n');
	return(0);
}

void get_sf(void *v, WINDOW *win)
{
    bool *sf = (bool *)sf;
	bool seefloor = See_floor;
	get_bool(sf,win);
	if(seefloor != See_floor) {
		if(!See_floor) {
			See_floor = TRUE;
			erase_lamp(&ME,Player.b_room);
			See_floor = FALSE;
		} else {
			look(FALSE);
		}
	}
}

int get_str(void *v, WINDOW *win)
{
	register char *bufp;
	register int c, y, x, length;
	static char buf[MAXSTR];
	char *str = (char *)v;

	getyx(win,y,x);
	wrefresh(win);
	bufp = buf;
	while((c = readchar()) != '\n' && c != '\r' && c != ESC) {
		if(c != -1) {
			if(c == erasechar()) {
				if(bufp > buf) {
					length = strlen(unctrl(*--bufp));
					while(length) {
						waddch(win,BSP);
						length--;
					}
				}
			} else if(c == killchar()) {
				bufp = buf;
				wmove(win,y,x);
			} else {
				if(bufp == buf && c == '-' && win != stdscr) break;
				if(bufp == buf && c == '~') {
					strcpy(buf,Home);
					waddstr(win,Home);
					bufp += strlen(Home);
				} else if(bufp >= &buf[50] || (!isprint(c) && c != ' ')) {
					putchar(BEL);
				} else {
					*bufp++ = c;
					waddstr(win,unctrl(c));
				}
			}
		}
		wclrtoeol(win);
		wrefresh(win);
	}
	*bufp = 0;
	if(bufp > buf) strucpy(str,buf,strlen(buf));
	/*mvwprintw(win,y,x,"%s\n",str);*/
	wrefresh(win);
	if(win == stdscr) Mpos += bufp-buf;
	if(c == '-') return(2);
	if(c == ESC) return(1);
	return(0);
}

int get_inv_t(void *v, WINDOW *win)
{
	register int y, x;
	bool not_got = TRUE;
	int *invt = (int *)v;
	getyx(win,y,x);
	waddstr(win,Inv_t_name[*invt]);
	while(not_got) {
		wmove(win,y,x);
		wrefresh(win);
		switch(readchar()) {
		case ESC:
			return(1);
		case '-':
			return(2);
		case 's':
		case 'S':
			*invt = 1;
			not_got = FALSE;
			break;
		case 'c':
		case 'C':
			*invt = 2;
			not_got = FALSE;
			break;
		case 'o':
		case 'O':
			*invt = 0;
		case '\n':
		case '\r':
			not_got = FALSE;
			break;
		default:
			wmove(win,y,x+15);
			waddstr(win,"(O, S, or C)");
		}
	}
	mvwprintw(win,y,x,"%s\n",Inv_t_name[*invt]);
	return(0);
}

void parse_opts(register char *env)
{
	register char *optname;
	register struct option *opt;
	register int length;
	register char **invtype, *string;
	while(*env) {
		for(optname = env; isalpha(*optname); optname++);
		length = optname-env;
		for(opt = optlist; opt < &optlist[OPTIONS]; opt++) {
			if(strncmp(env,opt->o_name,length) == 0) {
				if(opt->o_put == put_bool) {
					*(opt->o_arg) = TRUE;
				} else {
					for(env = optname+1; *env == '='; env++);
					if(*env == '~') {
						strcpy(opt->o_arg,Home);
						string = &opt->o_arg[strlen(Home)];
						while(*++env == '/');
					} else {
						string = opt->o_arg;
					}
					for(optname = env+1; *optname && *optname != ','; optname++);
					if(opt->o_put == put_inv_t) {
						if(islower(*env)) *env -= ('a'-'A');
						for(invtype = Inv_t_name; invtype <= &Inv_t_name[2]; invtype++) {
							if(strncmp(env,*invtype,optname-env) == 0) {
								Inv_type = invtype-Inv_t_name;
								break;
							}
						}
					} else {
						strucpy(string,env,optname-env);
					}
				}
				break;
			} else if(
				opt->o_put == put_bool &&
				strncmp(env,"no",2) == 0 &&
				strncmp(env+2,opt->o_name,length-2) == 0
			) {
				*(opt->o_arg) = FALSE;
				break;
			}
		}
		while(*optname && !isalpha(*optname)) optname++;
		env = optname;
	}
}

void strucpy(register char *to, char *from, int max)
{
	if(max > 50) max = 50;
	while(max--) {
		if(isprint(*from) || *from == ' ') {
			*to++ = *from++;
		}
	}
	*to = 0;
}

