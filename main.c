/*
 *	ROGUE
 *
 *	@(#)main.c	1.1	Strathclyde	10/31/84
 */

# include "param.h"
# include "types.h"
# include "extern.h"
# include <pwd.h>
# include <signal.h>
# include <unistd.h>
#include <sys/wait.h>

static const char *Def_term = "xterm";

int main(int argc, char *argv[])
{
	register char *env;
	register struct passwd *pass;
	register int tim;
	register const char *term;
	register int ccount;
# if 0 //ndef DEBUG
	signal(SIGQUIT,exit);
	signal(SIGILL,exit);
	signal(SIGTRAP,exit);
	signal(SIGIOT,exit);
#ifdef SIGEMT
	signal(SIGEMT,exit);
#endif
	signal(SIGFPE,exit);
	signal(SIGBUS,exit);
	signal(SIGSEGV,exit);
	signal(SIGSYS,exit);
# endif
	if((env = getenv("HOME")) != NULL) {
		strcpy(Home,env);
	} else if((pass = getpwuid(getuid())) != NULL) {
		strcpy(Home,pass->pw_dir);
	}
	if(Home[0]) strcat(Home,"/");
	strcpy(File_name,Home);
	strcat(File_name,"rogue.save");
	if((env = getenv("ROGUEOPTS")) != NULL) parse_opts(env);
	if(Whoami[0] == 0) {
		if((pass = getpwuid(getuid())) == 0) {
			printf("Say, who the hell are you?\n");
			exit(1);
		} else {
			strucpy(Whoami,pass->pw_name,strlen(pass->pw_name));
		}
	}
	tim = time(0);
	Dnum = getpid()+tim;
	Seed = Dnum;
	open_score();
	if(argc > 1) {
		if(strcmp(argv[1],"-s") == 0) {
			Noscore = TRUE;
			score(0,-1,0);
			exit(0);
		} else if(strcmp(argv[1],"-d") == 0) {
			Dnum = rnd(100);
			while(--Dnum) rnd(100);
			Purse = rnd(100)+1;
			Level = rnd(100)+1;
			initscr();
			getltchars();
			death(death_monst());
		}
	}
	init_check();
	if(argc > 1) {
		restore(argv[1]);
		gettmode();
		if((term = getenv("TERM")) == 0) term = Def_term;
		setterm(term);
		setup();
		clearok(curscr,TRUE);
		msg("file name: %s",File_name);
	} else {
		printf("Hello %s, just a moment while I dig the dungeon...",Whoami);
		fflush(stdout);
		initscr();
		init_probs();
		init_player();
		init_names();
		init_colours();
		init_stones();
		init_materials();
		setup();
		if(LINES < 24 || COLS < 80) {
			printf("\\Sorry, the screen must be at least %dx%d\n",24,80);
			endwin();
			my_exit(1);
		}
		Hw = newwin(LINES,COLS,0,0);
		new_level();
		add_daemon(runners,0,spread(2));
		add_daemon(doctor,0,spread(2));
		fuse(swander,0,spread(70),spread(2));
		add_daemon(stomach,0,spread(2));
	}
#ifdef SLOW_TERMINAL
	if(_tty.sg_ospeed <= 9) {
		Terse = TRUE;
		Jump = TRUE;
		See_floor = FALSE;
	}
#endif
/*	if(CE == 0) Inv_type = 2; */
	if((env = getenv("ROGUEOPTS")) != NULL) parse_opts(env);
	Oldpos = ME;
	Oldrp = roomin(&ME);
	for(;;) {
		ccount = 1;
		if(Player.b_flags & FAST) ccount++;
		do_daemons(spread(1));
		do_fuses(spread(1));
		while(ccount--) {
			if(Has_hit) {
				endmsg();
				Has_hit = FALSE;
			}
			look(TRUE);
			if(!Running) Door_stop = FALSE;
			status();
			Lastscore = Purse;
			move(MYY,MYX);
			if((!Running && Count == 0) || !Jump) refresh();
			After = TRUE;
			command();
			if(!Running) Door_stop = FALSE;
			if(!After) ccount++;
		}
		do_daemons(spread(2));
		do_fuses(spread(2));
		if(lring(R_SEARCHING)) {
			search();
		} else if(lring(R_TELEPORTATION) && rnd(TELECHANCE) == 0) {
			teleport();
		}
		if(rring(R_SEARCHING)) {
			search();
		} else if(rring(R_TELEPORTATION) && rnd(TELECHANCE) == 0) {
			teleport();
		}
	}
}

int rnd(register int mod)
{
	if(mod == 0) {
		return(0);
	} else {
		return(abs(RN) % mod);
	}
}

int roll(register int times, int limit)
{
	register int acc = 0;
	while(times--) acc += rnd(limit)+1;
	return(acc);
}

void quit()
{
	register int y, x;
	if(!Q_comm) Mpos = 0;
	getyx(curscr,y,x);
	msg("really quit? ");
	if(readchar() == 'y') {
		signal(SIGINT,leave);
		clear();
		mvprintw(LINES-2,0,"You quit with %d gold pieces",Purse);
		move(LINES-1,0);
		refresh();
		score(Purse,END_QUIT, 0);
		my_exit(0);
	} else {
		move(0,0);
		clrtoeol();
		status();
		move(y,x);
		refresh();
		Mpos = 0;
		Count = 0;
		To_death = FALSE;
	}
}

void leave()
{
	static char buf[1024];
	setbuf(stdout,buf);
	mvcur(0,COLS-1,LINES-1,0);
	endwin();
	putchar('\n');
	my_exit(0);
}

void shell()
{
	register int pid;
	register char *sh;
	int status;
	move(LINES-1,0);
	refresh();
	endwin();
#ifdef TIOCSLTC
	Ltc.t_dsuspc = Orig_dsusp;
	ioctl(1,TIOCSLTC,&Ltc);
#endif
	putchar('\n');
	In_shell = TRUE;
	After = FALSE;
	sh = getenv("SHELL");
	fflush(stdout);
	while((pid = fork()) < 0) sleep(1);
	if(pid == 0) {
		execl(sh == 0 ? "/bin/sh" : sh,"shell","-i",0);
		perror("No shell");
		exit(-1);
	} else {
		signal(SIGINT,SIG_IGN);
		signal(SIGQUIT,SIG_IGN);
		while(wait(&status) != pid);
		signal(SIGINT,quit);
		signal(SIGQUIT,quit);
		printf("\n%s",Preturn);
		fflush(stdout);
		noecho();
		crmode();
#ifdef TIOCSLTC
		Ltc.t_dsuspc = Ltc.t_suspc;
		ioctl(1,TIOCSLTC,&Ltc);
#endif
		In_shell = FALSE;
		wait_for('\n');
		clearok(stdscr,TRUE);
	}
}

void my_exit(int status)
{
#ifdef TIOCSLTC
	if(Got_ltc) {
		Ltc.t_dsuspc = Orig_dsusp;
		ioctl(1,TIOCSLTC,&Ltc);
	}
#endif
	exit(status);
}

