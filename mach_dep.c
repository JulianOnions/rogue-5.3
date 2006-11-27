/*
 *	ROGUE
 *
 *	@(#)mach_dep.c	1.1	Strathclyde	10/31/84
 */

/*
 * The various tuneable defines are:
 *
 *	HOF		Filename of the hall of fame.
 *	TEMP		Filename of the monthly/weekly scores.
 *	PLAYERS		Score file is top n players, not top n scores.
 *	NUMSCORES	Number of scores/players per score file (default 20).
 *	NUMSTRING	String for NUMSCORES (default "Twenty").
 *	MAXDAYS		Maximum number of days before a score gets thrown
 *			off the monthly (weekly) score file (default 31).
 *	MAXSTRING	String for MAXDAYS (default "Month").
 *	MAXLOAD		The maximum load average at which people may play.
 *	LOADAV		The loadav() system call is not defined.
 *	UNIX		The system namelist for loadav() IF there is no system
 *			call.
 *	MAXUSERS	The maximum user count at which people may play.
 *	UCOUNT		The ucount() library routine is not defined.
 *	UTMP		The user list.
 *	FREQUENCY	How often to check during the game for high load
 *			average or high user count.
 */

# include <curses.h>
# include "param.h"
# include "mach_dep.h"
# include "types.h"
# include "extern.h"
# include <signal.h>
# include <sys/types.h>
# include <sys/stat.h>
#include <unistd.h>

# if defined(MAXLOAD) || defined(MAXUSERS)

#if 0
# ifdef LOADAV
# include <nlist.h>
struct nlist avenrun[2] = {
	"_avenrun"
};
# endif 
#endif

# ifdef UCOUNT
# include <utmp.h>
# endif 

# ifdef FREQUENCY
static int	Num_checks;		/* times we've gone over in bouncer() */
void	bouncer(int);
# endif 

# endif /*MAXLOAD || MAXUSERS*/

# if defined(HOF) || defined(TEMP)

# include <sys/file.h>

static char Lockfile[] =	"/tmp/rogue.lock";
# ifndef LOCK_EX
static char Lockfile[] =	"/tmp/rogue.lock";
# endif 

# ifndef NUMSCORES
# define	NUMSCORES	20
# define	NUMSTRING	"Twenty"
# endif 

unsigned int numscores = NUMSCORES;
char *Numname = NUMSTRING;

# ifndef MAXDAYS
# define	MAXDAYS		31
# define	MAXSTRING	"Month"
# endif 

unsigned int Temp_days = MAXDAYS;
char *Max_name = MAXSTRING;
int too_much();
void b_msg(char *fmt, ...);

# ifdef PLAYERS
bool Allscore = FALSE;
# else 
bool Allscore = TRUE;
# endif 

# endif /*HOF || TEMP*/

void init_check()
{
# if defined(MAXLOAD) || defined(MAXUSERS)
	if(too_much()) {
		printf("Sorry, %s, but the system is too loaded now.\n",Whoami);
		printf("Try again later. Meanwhile, why not enjoy a%s %s?\n",vowelstr(Fruit),Fruit);
		exit(1);
	}
# ifdef FREQUENCY
	signal(SIGALRM,bouncer);
	alarm(FREQUENCY*60);
	Num_checks = 0;
# endif 
# endif /*MAXLOAD || MAXUSERS*/
}

# if defined(MAXLOAD) || defined(MAXUSERS)

# ifdef FREQUENCY
void bouncer(int dummy)
/*
 *	Check to see if the load is too high
 */
{
	static char *msgs[] = {
	"The load is too high to be playing. Please leave in %0.1f minutes",
	"Please save your game. You have %0.1f minutes",
	"Last warning. You have %0.1f minutes to leave",
	};
	int checktime;
	if (too_much()) {
		if(Num_checks++ == 3) {
			b_msg("You took too long - you are dead\n");
			death(death_monst());
		}
		checktime = (FREQUENCY*60)/Num_checks;
		alarm(checktime);
		b_msg(msgs[Num_checks-1],((double)checktime/60.0));
	} else {
		if(Num_checks) {
			Num_checks = 0;
			b_msg("The load has dropped back down. You have a reprieve");
		}
		alarm(FREQUENCY*60);
	}
}

void b_msg(char *fmt, ...)
/*
 *	bouncer()'s version of msg. If we are in the middle of a
 *	shell, do a printf instead of a msg to avoid the refresh.
 */
{
    va_list ap;
    va_start(ap, fmt);
	if(In_shell) {
		printf(fmt,ap);
		putchar('\n');
		fflush(stdout);
	} else {
		msg(fmt,ap);
	}
	va_end(ap);
}
# endif /*FREQUENCY*/

int too_much()
/*
 *	See if either there are too many users, or the system
 *	is too heavily loaded.
 */
{
# ifdef MAXLOAD
	double avec[3];
	getloadavg(avec, 3);
	if(avec[1] > (double)MAXLOAD) {
		return(TRUE);
	}
# endif 
# ifdef MAXUSERS
	if(ucount() > MAXUSERS) {
		return(TRUE);
	}
# endif 
	return(FALSE);
}

#if 0
# ifdef LOADAV
loadav(avg)
	register double *avg;
/*
 *	Looking up load average in core (for system where the loadav()
 *	system call isn't defined
 */
{
	register int kmem;
	if((kmem = open("/dev/kmem",0)) >= 0) {
		nlist(UNIX,avenrun);
		if(avenrun[0].n_type) {
			lseek(kmem,(long)avenrun[0].n_value,0);
			read(kmem,(char *)avg,3*sizeof(double));
			close(kmem);
			return;
		}
	}
	close(kmem);
	avg[0] = 0.0;
	avg[1] = 0.0;
	avg[2] = 0.0;
}
# endif /*LOADAV*/
#endif

# ifdef UCOUNT
ucount()
/*
 *	Count number of users on the system
 */
{
	register struct utmp *up;
	register FILE *utmp;
	register int users;
	static struct utmp buf;
	if((utmp = fopen(UTMP,"r")) == NULL) {
		return(0);
	}
	up = &buf;
	users = 0;
	while(fread(up,1,sizeof(*up),utmp) > 0) {
		if(buf.ut_name[0] != '\0') users++;
	}
	fclose(utmp);
	return(users);
}
# endif /*UCOUNT*/

# endif /*MAXLOAD || MAXUSERS*/

void setup()
{
	__sighandler_t auto_save, quit, tstp;
	signal(SIGINT,quit);
	signal(SIGHUP,auto_save);
# if 0 //ndef DEBUG
	signal(SIGILL,auto_save);
	signal(SIGTRAP,auto_save);
	signal(SIGIOT,auto_save);
#ifdef SIGEMT
	signal(SIGEMT,auto_save);
#endif
	signal(SIGFPE,auto_save);
	signal(SIGBUS,auto_save);
	signal(SIGSEGV,auto_save);
	signal(SIGSYS,auto_save);
	signal(SIGTERM,auto_save);
	signal(SIGQUIT,quit);
# endif 
	cbreak();
	noecho();
	typeahead(-1);
# ifdef TIOCGLTC
	getltchars();
# endif 
}

void getltchars()
{
# ifdef TIOCGLTC
	ioctl(1,TIOCGLTC,&Ltc);
	Got_ltc = TRUE;
	Orig_dsusp = Ltc.t_dsuspc;
	Ltc.t_dsuspc = Ltc.t_suspc;
	ioctl(1,TIOCSLTC,&Ltc);
# endif 
}

void open_score()
{
# ifdef HOF
	Fd_hof = open(HOF,2);
	if(Fd_hof < 0) perror(HOF);
# else 
	Fd_hof = -1;
# endif
# ifdef TEMP
	Fd_temp = open(TEMP,2);
	if (Fd_temp < 0) perror(TEMP);
# else
	Fd_temp = -1;
# endif
	setuid(getuid());
	setgid(getgid());
}

void start_score()
{
# ifdef FREQUENCY
	signal(SIGALRM, SIG_IGN);
# endif 
}

int  lock_sc()
/*
 *	lock the score file. If it takes too long, ask the user if
 *	they care to wait. Return TRUE if the lock is successful.
 */
{
# if defined(HOF) || defined(TEMP)
	register int cnt;
	static struct stat sbuf;
	time_t time();
over:
	close(8);	/* just in case there are no files left */
	if(creat(Lockfile,0) >= 0) {
		return(TRUE);
	}
	for(cnt = 0; cnt < 5; cnt++) {
		sleep(1);
		if(creat(Lockfile,0) >= 0) {
			return(TRUE);
		}
	}
	if(stat(Lockfile,&sbuf) < 0) {
		creat(Lockfile,0);
		return(TRUE);
	}
	if(time(NULL)-sbuf.st_mtime > 10) {
		if(unlink(Lockfile) < 0) {
			return(FALSE);
		}
		goto over;
	} else {
		printf("The score file is very busy. Do you want to wait longer\n");
		printf("for it to become free so your score can get posted?\n");
		printf("If so, type \"y\"\n");
		fgets(Prbuf,MAXSTR,stdin);
		if(Prbuf[0] == 'y') {
			for(;;) {
				if(creat(Lockfile,0) >= 0) {
					return(TRUE);
				}
				if(stat(Lockfile,&sbuf) < 0) {
					creat(Lockfile,0);
					return TRUE;
				}
				if(time(NULL)-sbuf.st_mtime > 10) {
					if(unlink(Lockfile) < 0) {
						return FALSE;
					}
				}
				sleep(1);
			}
		} else {
			return FALSE;
		}
	}
# endif /*HOF || TEMP*/
}

void unlock_sc()
/*
 *	Unlock the score file
 */
{
# if defined(HOF) || defined(TEMP)
	unlink(Lockfile);
# endif /*HOF || TEMP*/
}

int issymlink(char *sp)
{
# ifdef S_IFLNK
	struct stat sbuf2;
	if(lstat(sp,&sbuf2) < 0) {
		return(FALSE);
	} else {
		return((sbuf2.st_mode & S_IFMT) != S_IFREG);
	}
# else 
	return(FALSE);
# endif
}

void flush_type()
/*
 *	Flush typeahead by forcing call of ioctl
 */
{
	crmode();
}

