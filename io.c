/*
 *	ROGUE
 *
 *	@(#)io.c	1.1	Strathclyde	10/31/84
 */

# include "param.h"
# include "types.h"
# include "extern.h"
# include <stdarg.h>
#include <errno.h>
#include <unistd.h>


char	Msgbuf[MAXSTR];
int	Newpos = 0;

char	addbuf[MAXSTR];
FILE	addpr;

int	width = 0, last_purse = -1, last_exp = 0;
int	last_hunger, last_level, last_hp, protection, last_strength;

char	*Hunger[] = {
	"",
	"Hungry",
	"Weak",
	"Faint"
};

/* VARARGS 1 */
int msg(const char *fmt, ...)
{
	va_list ap;

	va_start (ap, fmt);
	if(*fmt == 0) {
		move(0,0);
		clrtoeol();
		Mpos = 0;
		return(-28);
	}
	doadd(fmt,ap);
	va_end (ap);
	return(endmsg());
}

/* VARARGS 1 */
void addmsg(const char *message, ...)
{
        va_list ap;
	va_start(ap, message);
	doadd(message,ap);
	va_end (ap);
}

int endmsg()
{
	char c;
	if(Save_msg) strcpy(Huh,Msgbuf);
	if(Mpos) {
		look(FALSE);
		mvaddstr(0,Mpos,"--More--");
		refresh();
		if(Msg_esc) {
			while((c = readchar()) != ' ') {
				if(c == ESC) {
					Msgbuf[0] = 0;
					Mpos = 0;
					Newpos = 0;
					Msgbuf[0] = 0;
					return(27);
				}
			}
		} else {
			wait_for(' ');
		}
	}
	if(islower(Msgbuf[0]) && !Lower_msg && Msgbuf[1] != ')') {
		Msgbuf[0] -= ('a'-'A');
	}
	mvaddstr(0,0,Msgbuf);
	clrtoeol();
	Mpos = Newpos;
	Newpos = 0;
	Msgbuf[0] = 0;
	refresh();
	return(-28);
}

void doadd(const char *message, va_list ap)
{
	vsprintf(addbuf, message, ap);
	if(strlen(addbuf)+Newpos < 76) {
		strcat(Msgbuf,addbuf);
		Newpos = strlen(Msgbuf);
	}
}

char quest(char *str)
{
	for(;;) {
		msg(str);
		Mpos = 0;
		switch(readchar()) {
		case ESC:
			return(ESC);
		case 'y':
		case 'Y':
			addstr("Yes");
			return('y');
		case 'n':
		case 'N':
			addstr("No");
			return('n');
		default:
			msg("Please answer Y or N");
		}
	}
}

void err_msg(char *str)
{
	msg("%s: %s",str,strerror(errno));
}

int step_ok(char floorch)
{
	if(floorch == ' ' || floorch == HWALL || floorch == VWALL) return(FALSE);
	return(!isalpha(floorch));
}

char readchar()
{
	register int failcount = 0;
	static char buf[1];
	while(read(0,buf,1) <= 0) if(failcount++ > 64) auto_save();
	return(buf[0]);
}

void status()
{
	register int (*print)(const char *fmt, ...);
	register int y, x, n;
	n = Cur_armor ? Cur_armor->i_value : Player.b_stats.protection;
	if(
		last_hp == Player.b_stats.hp &&
		last_exp == Player.b_stats.experience &&
		last_purse == Purse &&
		protection == n &&
		last_strength == Player.b_stats.strength &&
		last_level == Level &&
		last_hunger == Hungry_state &&
		!Stat_msg
	) return;
	protection = n;
	getyx(stdscr,y,x);
	if(last_hp != Player.b_stats.maxhp) {
		n = Player.b_stats.maxhp;
		last_hp = n;
		width = 0;
		while(n) {
			n /= 10;
			width++;
		}
	}
	last_level = Level;
	last_purse = Purse;
	last_hp = Player.b_stats.hp;
	last_strength = Player.b_stats.strength;
	last_exp = Player.b_stats.experience;
	last_hunger = Hungry_state;
	if(Stat_msg) {
		print = msg;
		move(0,0);
	} else {
		move(23,0);
		print = printw;
	}
	(*print)(
		"Level: %d  Gold: %-5d  Hp: %*d(%*d)  Str: %2d(%d)  Arm: %-2d  Exp: %d/%ld  %s",
		Level,
		Purse,
		width,
		Player.b_stats.hp,
		width,
		Player.b_stats.maxhp,
		Player.b_stats.strength,
		Max_stats.strength,
		10-protection,
		Player.b_stats.level,
		Player.b_stats.experience,
		Hunger[Hungry_state]
	);
	clrtoeol();
	move(y,x);
}

void wait_for(char c)
{
	char x;
	if(c == '\n') {
		while((x = readchar()) != '\n' && x != '\r');
	} else {
		while(readchar() != c);
	}
}

void show_win(char *message)
{
	register WINDOW *win;
	win = Hw;
	wmove(win,0,0);
	waddstr(win,message);
	touchwin(win);
	wmove(win,MYY,MYX);
	wrefresh(win);
	wait_for(' ');
	clearok(curscr,TRUE);
}

