#	ROGUE
#
#	Makefile	1.1	Strathclyde	14/9/1984
#

OBJS=	armour.o chase.o command.o daemon.o fight.o init.o io.o item.o\
	main.o map.o misc.o monster.o move.o new_level.o option.o pack.o\
	passages.o potion.o ring.o score.o screen.o scribe.o scroll.o stick.o\
	thing.o throw.o weapon.o mach_dep.o

SRCS=	Data.c armour.c chase.c command.c daemon.c fight.c init.c io.c item.c\
	main.c map.c misc.c monster.c move.c new_level.c option.c pack.c\
	passages.c potion.c ring.c score.c screen.c scribe.c scroll.c stick.c\
	thing.c throw.c weapon.c mach_dep.c

CFLAGS=	-g -Wall -O 

rogue:  Data.o ${OBJS}
	${CC}   -g -o rogue Data.o ${OBJS} -lcurses
saber_src: $(SRCS)
	#load -C $(CFLAGS) $(SRCS)

$(OBJS): extern.h    mach_dep.h  param.h     types.h
debug:
	touch `grep -l DEBUG *.c`

clean:
	rm -rf *.o a.out core

echo:
	echo $(SRCS) *.h

lint: $(SRCS) extern.h param.h types.h
	lint -haxbc $(CFLAGS) $(SRCS) -lcurses -ltermcap

mach_dep.o: mach_dep.c mach_dep.h
