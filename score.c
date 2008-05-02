/*
 *	ROGUE
 *
 *	@(#)score.c	1.1	Strathclyde	10/31/84
 */

# include "param.h"
# include "types.h"
# include "extern.h"
# include <signal.h>
# include <sys/types.h>
# include <time.h>
#include <unistd.h>

# define	SECSPERDAY	86400

char	*Rip[] = {
	"                       __________\n",
	"                      /          \\\n",
	"                     /    REST    \\\n",
	"                    /      IN      \\\n",
	"                   /     PEACE      \\\n",
	"                  /                  \\\n",
	"                  |                  |\n",
	"                  |                  |\n",
	"                  |   killed by a    |\n",
	"                  |                  |\n",
	"                  |       2000       |\n",
	"                 *|     *  *  *      | *\n",
	"         ________)/\\\\_//(\\/(/\\)/\\//\\/|_)_______\n",
	0
};

char *End_types[] = {
	"killed",
	"quit",
	"A total winner",
	"killed with Amulet"
};

struct killtype {
	char	k_ch;
	char	*k_name;
	bool	k_article;
} Kill_types[] = {
{	'a',	"arrow",	TRUE},
{	'b',	"bolt",		TRUE},
{	'd',	"dart",		TRUE},
{	'h',	"hypothermia",	FALSE},
{	's',	"starvation",	FALSE},
    {	0,	0,		0}
};

char Rnd_death[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabhds";

# ifdef DEBUG
bool	No_death = FALSE;
# endif 

static int centre(const char *str);
void do_score(int player_score, int end_type,
	      char killtype,
	      int fd, int max_days,
	      char *type_str);

void score(int player_score, int end_type, char killtype)
{
	static char buf[MAXSTR];
	start_score();
	if(end_type >= 0) {
		endwin();
#ifdef TIOCSLTC
		Ltc.t_dsuspc = Orig_dsusp;
		ioctl(1,TIOCSLTC,&Ltc);
#endif
		delwin(stdscr);
		delwin(curscr);
		if(Hw) delwin(Hw);
	}
	sprintf(buf,"for this %s",Max_name);
	do_score(player_score,end_type,killtype,Fd_temp,Temp_days,buf);
	if(end_type < 0) {
		printf(Preturn);
		fflush(stdout);
		fgets(Prbuf, MAXSTR, stdin);
	}
	do_score(player_score,end_type,killtype,Fd_hof,0,"of All Time");
}

void do_score(player_score,end_type,killtype,fd,max_days,type_str)
	int player_score, end_type;
	char killtype;
	int fd, max_days;
	char *type_str;
{
	register struct scorefile *position;
	register int x;
	register struct scorefile *remove, *sfile, *eof;
	struct scorefile *oldest;
	FILE *score_fd;
	int uid, this_day, limit;
	extern int	_putchar();
	void *action;

	if(fd < 0) return;
	this_day = max_days ? time(0)/SECSPERDAY : 0;
	limit = this_day-max_days;
	score_fd = fdopen(fd,"w");
	sfile = (struct scorefile *)(malloc(numscores*sizeof(struct scorefile)));
	eof = &sfile[numscores];
	for(position = sfile; position < eof; position++) {
		position->s_score = 0;
		for(x = 0; x < MAXSTR; x++) position->s_name[x] = rnd(0377);
		position->s_endtype = RN;
		position->s_level = RN;
		position->s_killed = RN;
		position->s_uid = RN;
	}
	signal(SIGINT,SIG_DFL);
	if(end_type >= 0) {
		printf(Preturn);
		fflush(stdout);
		fgets(Prbuf, MAXSTR, stdin);
	}
	rd_score(sfile,fd);
	remove = 0;
	if(!Noscore && player_score > 0) {
		uid = getuid();
		oldest = 0;
		x = limit;
		for(position = eof-1; position >= sfile; position--) {
			if(position->s_days < x) {
				x = position->s_days;
				oldest = position;
			}
		}
		position = 0;
		for(remove = sfile; remove < eof; remove++) {
			if(position == 0 && player_score > remove->s_score) position = remove;
			if(
				!Allscore &&
				remove->s_uid == uid &&
				end_type != END_WINNER &&
				remove->s_endtype != END_WINNER
			) break;
		}
		if(remove < eof) {
			if(position == 0 && remove->s_days < limit) position = remove;
		} else if(oldest) {
			remove = oldest;
			if(position == 0) position = eof-1;
		} else if(position) {
			remove = eof-1;
		}
		if(position) {
			if(remove < position) {
				position--;
				while(remove < position) {
					*remove = *(remove+1);
					remove++;
				}
			} else {
				while(remove > position) {
					*remove = *(remove-1);
					remove--;
				}
			}
			position->s_score = player_score;
			strncpy(position->s_name,Whoami,MAXSTR);
			position->s_endtype = end_type;
			if(end_type == END_WINNER) {
				position->s_level = Max_level;
			} else {
				position->s_level = Level;
			}
			position->s_killed = killtype;
			position->s_uid = uid;
			position->s_days = this_day;
			fseek(score_fd,0,0);
			if(lock_sc()) {
				action = signal(SIGINT,SIG_IGN);
				wr_score(sfile,score_fd);
				unlock_sc();
				signal(SIGINT,action);
			}
			fclose(score_fd);
		}
	}
	if(end_type >= 0) putchar('\n');
	printf("Top %s %s %s:\n",Numname,Allscore ? "Scores" : "Rogueists",type_str);
	printf("Rank\tScore\tName\n");
	for(position = sfile; position < eof; position++) {
		if(position->s_score == 0) break;
		if(remove == position) vidattr(A_STANDOUT); /*tputs(SO,0,_putchar);*/
		printf(
			"%c%d\t%d\t%s: %s on level %d",
			position->s_days < limit ? '*' : ' ',
			position-sfile+1,
			position->s_score,
			position->s_name,
			End_types[position->s_endtype],
			position->s_level
		);
		if(
			position->s_endtype == END_KILLED ||
			position->s_endtype == END_KILLED_AMULET
		) printf(" by %s",killname(position->s_killed & 0377,TRUE));
		printf(".\n");
		if(remove == position) vidattr(A_NORMAL); /*tputs(SE,0,_putchar);*/
	}
}

void death(char cause)
{
	register char **tomb, *kname;
	register struct tm *tvec;
	static time_t tloc;
	if(Player.b_stats.hp < 0) Player.b_stats.hp = 0;
	status();
	refresh();
# ifdef DEBUG
	if(No_death) return;
	msg("Do you want a reprieve?");
	for(;;) {
		switch(readchar()) {
		case 'y':
		case 'Y':
			No_death = TRUE;
			return;
		case 'n':
		case 'N':
			break;
		default:
			continue;
		}
		break;
	}
# endif 
	signal(SIGINT,SIG_IGN);
	Purse -= Purse/10;
	signal(SIGINT,leave);
	clear();
	kname = killname(cause,FALSE);
	if(!Tombstone) {
		mvprintw(LINES-2,0,"Killed by ");
		kname = killname(cause,FALSE);
		if(cause != 's' && cause != 'h') printw("a%s ",vowelstr(kname));
		printw("%s with %d gold",kname,Purse);
	} else {
		time(&tloc);
		tvec = localtime(&tloc);
		move(8,0);
		for(tomb = Rip; *tomb;) addstr(*tomb++);
		mvaddstr(17,centre(kname),kname);
		if(cause == 's' || cause == 'h') {
			mvaddch(16,32,' ');
		} else {
			mvaddstr(16,33,vowelstr(kname));
		}
		mvaddstr(14,centre(Whoami),Whoami);
		sprintf(Prbuf,"%d Au",Purse);
		mvaddstr(15,centre(Prbuf),Prbuf);
		sprintf(Prbuf,"%4d",tvec->tm_year+1900);
		mvaddstr(18,26,Prbuf);
	}
	move(LINES-1,0);
	refresh();
	score(Purse,Amulet ? END_KILLED_AMULET : END_KILLED,cause);
	my_exit(0);
}

int centre(const char *str)
{
	int length = (strlen(str)+1)/2;
	return(28-length);
}

void total_winner()
{
	register struct item *thing;
	register struct info *inf;
	register int value = 0, purse;
	clear();
	standout();
	addstr("                                                               \n");
	addstr("  @   @               @   @           @          @@@  @     @  \n");
	addstr("  @   @               @@ @@           @           @   @     @  \n");
	addstr("  @   @  @@@  @   @   @ @ @  @@@   @@@@  @@@      @  @@@    @  \n");
	addstr("   @@@@ @   @ @   @   @   @     @ @   @ @   @     @   @     @  \n");
	addstr("      @ @   @ @   @   @   @  @@@@ @   @ @@@@@     @   @     @  \n");
	addstr("  @   @ @   @ @  @@   @   @ @   @ @   @ @         @   @  @     \n");
	addstr("   @@@   @@@   @@ @   @   @  @@@@  @@@@  @@@     @@@   @@   @  \n");
	addstr("                                                               \n");
	addstr("     Congratulations, you have made it to the light of day!    \n");
	standend();
	addstr("\nYou have joined the elite ranks of those who have escaped the\n");
	addstr("Dungeons of Doom alive.  You journey home and sell all your loot at\n");
	addstr("a great profit and are admitted to the Fighters' Guild.\n");
	mvaddstr(LINES-1,0,Pspace);
	refresh();
	wait_for(' ');
	clear();
	mvaddstr(0,0,"   Worth  Item\n");
	purse = Purse;
	for(thing = Player.b_pack; thing; thing = thing->i_link) {
		switch(thing->i_item) {
		case FOOD:
			value = thing->i_number << 1;
			break;
		case WEAPON:
			value = Weap_info[thing->i_type].inf_value;
			value *= (thing->i_hitplus+thing->i_damplus)*3+thing->i_number;
			thing->i_flags |= KNOWN;
			break;
		case ARMOUR:
			value = Arm_info[thing->i_type].inf_value;
			value += (9-thing->i_value)*100;
			value += (A_class[thing->i_type]-thing->i_value)*10;
			break;
		case SCROLL:
			value = Scr_info[thing->i_type].inf_value;
			value *= thing->i_number;
			inf = &Scr_info[thing->i_type];
			if(!inf->inf_known) value /= 2;
			inf->inf_known = TRUE;
			break;
		case POTION:
			value = Pot_info[thing->i_type].inf_value;
			value *= thing->i_number;
			inf = &Pot_info[thing->i_type];
			if(!inf->inf_known) value /= 2;
			inf->inf_known = TRUE;
			break;
		case RING:
			inf = &Ring_info[thing->i_type];
			value = inf->inf_value;
			if(
				thing->i_type == R_ADD_STRENGTH ||
				thing->i_type == R_INCREASE_DAMAGE ||
				thing->i_type == R_PROTECTION ||
				thing->i_type == R_DEXTERITY
			) {
				if(thing->i_value > 0) {
					value += thing->i_value*100;
				} else {
					value = 10;
				}
			}
			if((thing->i_flags & KNOWN) == 0) value /= 2;
			thing->i_flags |= KNOWN;
			inf->inf_known = TRUE;
			break;
		case STICK:
			inf = &Ws_info[thing->i_type];
			value = inf->inf_value;
			value += thing->i_value*20;
			if((thing->i_flags & KNOWN) == 0) value /= 2;
			thing->i_flags |= KNOWN;
			inf->inf_known = TRUE;
			break;
		case AMULET:
			value = 1000;
		}
		if(value < 0) value = 0;
		printw("%c) %5d  %s\n",thing->i_packchar,value,inv_name(thing,FALSE));
		Purse += value;
	}
	printw("   %5d  Gold Pieces          ",purse);
	refresh();
	score(Purse,END_WINNER,0);
	my_exit(0);
}

char *killname(cause,line)
	char cause;
	bool line;
{
	register struct killtype *ktype;
	register char *kname;
	bool article;
	if(isupper(cause)) {
		kname = Monsters[mindex(cause)].m_name;
		article = TRUE;
	} else {
		kname = "Wally the Wonder Badger";
		article = FALSE;
		for(ktype = Kill_types; ktype->k_ch; ktype++) {
			if(ktype->k_ch == cause) {
				kname = ktype->k_name;
				article = ktype->k_article;
				break;
			}
		}
	}
	if(line && article) {
		sprintf(Prbuf,"a%s ",vowelstr(kname));
	} else {
		Prbuf[0] = 0;
	}
	strcat(Prbuf,kname);
	return(Prbuf);
}

int death_monst()
{
	return(Rnd_death[rnd(32)]);
}

