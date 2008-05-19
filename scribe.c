/*
 *	ROGUE
 *
 *	@(#)scribe.c	1.1	Strathclyde	10/31/84
 */

# include "param.h"
# include "types.h"
# include "extern.h"
# include <signal.h>
# include <sys/types.h>
# include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

char	Frob;

static void save_file(register FILE *sf);
void encread(register char *buf, int nbytes, int fd);
void encwrite(register char *buf, int nbytes, FILE *f);
struct stat	Sbuf;
#ifndef TIOCSLTC
void brk(char *str)
{
abort();
}
#endif

void save_game()
{
	register FILE *sf;
	char filename[MAXSTR];
	char tbuf[80];
	for(;;) {
	    sprintf(tbuf, "Save file (%s)? ", File_name);
		switch(quest(tbuf)) {
			case ESC:
				msg("");
				return;
			case 'y':
				strcpy(filename, File_name);
				break;
			default:
				msg("save file:  ");
				if(get_str(filename,stdscr) == 1) {
					msg("");
					return;
				}
				Mpos = 0;
				if(filename[0] == 0) strcpy(filename,File_name);
		}
		if(stat(filename,&Sbuf) >= 0) {
			switch(quest("File exists. Do you wish to overwrite it? ")) {
				case ESC:
					msg("");
					return;
				case 'y':
					if(unlink(filename) >= 0) break;
					err_msg(filename);
				default:
					continue;
			}
		}
		strcpy(File_name,filename);
		if((sf = fopen(File_name,"w")) == NULL) {
			err_msg(File_name);
		} else {
			msg("file name: %s",filename);
			save_file(sf);
		}
	}
}

void auto_save()
{
	register FILE *sf;
	register int sig;
	for(sig = 0; sig < NSIG; sig++) signal(sig,SIG_IGN);
	if(File_name[0]) {
		unlink(File_name);
		if((sf = fopen(File_name,"w")) != NULL) {
			save_file(sf);
		}
	}
	exit(0);
}

void save_file(register FILE *sf)
{
	extern int _putchar();
	mvcur(0,COLS-1,LINES-1,0);
	clrtoeol();
	endwin();
#ifdef TIOCSLTC
	Ltc.t_dsuspc = Orig_dsusp;
	ioctl(1,TIOCSLTC,&Ltc);
#endif
	chmod(File_name,0400);
	Frob = 0;
	fwrite(&Frob,1,1,sf);
	fstat(fileno(sf),&Sbuf);
	encwrite(version,(char *)sbrk(0)-version,sf);
	fseek(sf,(char *)(&Sbuf.st_ctime)-version+1,0);
	fflush(sf);
	fstat(fileno(sf),&Sbuf);
	fwrite(&Sbuf.st_ctime,sizeof(int),1,sf);
	fclose(sf);
	exit(0);
}

void restore(register char *filename)
{
	register int fd;
	 void *act;
	char frob;
	char vers[MAXSTR];
	struct stat statbuf;

	if(strcmp(filename,"-r") == 0) filename = File_name;
	act = signal(SIGTSTP,SIG_IGN);
	if((fd = open(filename,O_RDONLY, 0)) < 0) {
		perror(filename);
		exit(1);
	}
	fstat(fd,&statbuf);
	if(statbuf.st_nlink != 1 || issymlink(filename)) {
		printf("Cannot restore from a linked file\n");
		exit(1);
	}
	if(unlink(filename) < 0) {
		printf("Cannot unlink file\n");
		exit(1);
	}
	fflush(stdout);
	read(fd,&Frob,1);
	frob = Frob;
	encread(vers,strlen(version)+1,fd);
	if(strcmp(vers,version)) {
		printf("Sorry, saved game is out of date.\n");
		exit(1);
	}
	brk(version+statbuf.st_size);
	lseek(fd,1,0);
	Frob = frob;
	encread(version,statbuf.st_size,fd);
	lseek(fd,(char *)(&Sbuf.st_ctime)-version+1,0);
	read(fd,&Sbuf.st_ctime,4);
	if(statbuf.st_ino != Sbuf.st_ino || statbuf.st_dev != Sbuf.st_dev) {
		printf("Sorry, saved game is not in the same file.\n");
		exit(1);
	}
	if(statbuf.st_ctime-Sbuf.st_ctime > 15) {
		printf("Sorry, file has been touched, so this score won't be recorded\n");
		Noscore = TRUE;
	}
	Mpos = 0;
	mvprintw(0,0,"%s: %s",filename,ctime(&statbuf.st_mtime));
	if(Player.b_stats.hp <= 0) {
		printf("\"He's dead, Jim\"\n");
		exit(1);
	}
	signal(SIGTSTP,act);
	strcpy(File_name,filename);
}

void encwrite(register char *buf, int nbytes, FILE *f)
{
	register char *enc, *stat;
	register int prod;
	char frob;
	enc = encstr;
	stat = statlist;
	frob = Frob;
	while(nbytes--) {
		putc(*buf++ ^ *enc ^ *stat ^ frob,f);
		prod = *enc++;
		frob += prod*(*stat++);
		if(*enc == 0) enc = encstr;
		if(*stat == 0) stat = statlist;
	}
}

void encread(register char *buf, int nbytes, int fd)
{
	register char *enc, *stat;
	register int prod;
	register int size;
	char frob;
	frob = Frob;
	if((size = read(fd,buf,nbytes)) == -1) return;
	enc = encstr;
	stat = statlist;
	while(size--) {
		*buf++ ^= *enc ^ *stat ^ frob;
		prod = *enc++;
		frob += prod*(*stat++);
		if(*enc == 0) enc = encstr;
		if(*stat == 0) stat = statlist;
	}
}

void rd_score(struct scorefile *buf, int fd)
{
	encread((char *)buf,numscores*sizeof(struct scorefile),fd);
}

void wr_score(struct scorefile *buf, FILE *f)
{
	encwrite((char *)buf,numscores*sizeof(struct scorefile),f);
}

