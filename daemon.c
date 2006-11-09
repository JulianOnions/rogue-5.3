/*
 *	ROGUE
 *
 *	@(#)daemon.c	1.1	Strathclyde	10/31/84
 */

# include "param.h"
# include "types.h"
# include "extern.h"

static void rollwand(int junk);

# define	SLOTS	20
# define	DAEMON	(-1)

struct daemon {
	int	d_spread;
        DFUNC	d_func;
	int	d_arg;
	int	d_count;
} d_list[SLOTS];

struct daemon *d_slot()
{
	register struct daemon *slot;
	for(slot = d_list; slot < &d_list[SLOTS]; slot++) {
		if(slot->d_spread == 0) return(slot);
	}
	return(0);
}

struct daemon *find_slot(DFUNC func)
{
	register struct daemon *slot;
	for(slot = d_list; slot < &d_list[SLOTS]; slot++) {
		if(slot->d_spread && func == slot->d_func) return(slot);
	}
	return(0);
}

void add_daemon(DFUNC func, int arg, int spread)
{
	register struct daemon *slot;
	slot = d_slot();
	slot->d_spread = spread;
	slot->d_func = func;
	slot->d_arg = arg;
	slot->d_count = DAEMON;
}

void kill_daemon(DFUNC func)
{
	register struct daemon *slot;
	if((slot = find_slot(func)) == 0) return;
	slot->d_spread = 0;
}

void do_daemons(register int spread)
{
	register struct daemon *slot;
	for(slot = d_list; slot < &d_list[SLOTS]; slot++) {
		if(slot->d_spread == spread && slot->d_count == DAEMON) {
			(slot->d_func)(slot->d_arg);
		}
	}
}

void fuse(DFUNC func, int arg, int count, int spread)
{
	register struct daemon *slot;
	slot = d_slot();
	slot->d_spread = spread;
	slot->d_func = func;
	slot->d_arg = arg;
	slot->d_count = count;
}

void lengthen(DFUNC func, int count)
{
	register struct daemon *slot;
	if((slot = find_slot(func)) == 0) return;
	slot->d_count += count;
}

void extinguish(DFUNC func)
{
	register struct daemon *slot;
	if((slot = find_slot(func)) == 0) return;
	slot->d_spread = 0;
}

void do_fuses(register int spread)
{
	register struct daemon *slot;
	for(slot = d_list; slot < d_list+SLOTS; slot++) {
		if(spread == slot->d_spread && slot->d_count > 0 && --slot->d_count == 0) {
			slot->d_spread = 0;
			(slot->d_func)(slot->d_arg);
		}
	}
}

/*ARGSUSED */
void doctor(int junk)
{
	register int level, oldhp;
	level = Player.b_stats.level;
	oldhp = Player.b_stats.hp;
	++Quiet;
	if(level < 8) {
		if(level*2+Quiet > 20) ++Player.b_stats.hp;
	} else {
		if(Quiet >= 3) Player.b_stats.hp += rnd(level-7)+1;
	}
	if(lring(R_REGENERATION)) ++Player.b_stats.hp;
	if(rring(R_REGENERATION)) ++Player.b_stats.hp;
	if(oldhp != Player.b_stats.hp) {
		if(Player.b_stats.hp > Player.b_stats.maxhp) Player.b_stats.hp = Player.b_stats.maxhp;
		Quiet = 0;
	}
}

/* ARGSUSED */
void swander(int junk)
{
    add_daemon(rollwand,0,spread(1));
}

/* ARGSUSED */
void rollwand(int junk)
{
	static int rollcount;
	if(++rollcount >= 4) {
		if(roll(1,6) == 4) {
			wanderer();
			kill_daemon(rollwand);
			fuse(swander,0,spread(70),spread(1));
		}
		rollcount = 0;
	}
}

void unconfuse()
{
	Player.b_flags &= ~CONFUSED;
	msg("you feel less %s now",choose_str("trippy","confused"));
}

void unsee()
{
	register struct being *monst;
	for(monst = Mlist; monst; monst = monst->b_link) {
		if((monst->b_flags & INVISIBLE) && see_monst(monst)) {
			mvaddch(bx(monst),by(monst),monst->b_oldch);
		}
	}
}

void sight()
{
	if(Player.b_flags & BLIND) {
		extinguish(sight);
		Player.b_flags &= ~BLIND;
		if((Player.b_room->r_flags & R_PASSAGE) == 0) enter_room(&ME);
		msg(choose_str("far out!  Everything is all cosmic again","the veil of darkness lifts"));
	}
}

void nohaste()
{
	Player.b_flags &= ~FAST;
	msg("you feel yourself slowing down");
}

/* ARGSUSED */
void stomach(int junk)
{
	register int food_before;
	if(Food_left <= 0) {
		if(Food_left-- < -850) death('s');
		if(No_command || rnd(5)) return;
		No_command += rnd(8)+4;
		Player.b_flags &= ~MOBILE;
		Running = FALSE;
		Count = 0;
		Hungry_state = 3;
		if(Verbose) addmsg(choose_str("the munchies overpower your motor capabilities.  ","you feel too weak from lack of food.  "));
		msg(choose_str("You freak out","You faint"));
	} else {
		food_before = Food_left;
		Food_left -= ring_eat(LEFT)+ring_eat(RIGHT)+1-Amulet;
		if(Food_left < 150 && food_before >= 150) {
			Hungry_state = 2;
			msg(choose_str("the munchies are interfering with your motor capabilites","you are starting to feel weak"));
		} else if(Food_left < 300 && food_before >= 300) {
			Hungry_state = 1;
			if(Terse) {
				msg(choose_str("getting the munchies","getting hungry"));
			} else {
				msg(choose_str("you are getting the munchies","you are starting to get hungry"));
			}
		}
	}
}

void come_down()
{
	register struct item *i;
	register struct being *m;
	bool detecting;

	if((Player.b_flags & HALLUCINATING) == 0) return;
	kill_daemon(visuals);
	Player.b_flags &= ~HALLUCINATING;
	if(Player.b_flags & BLIND) return;
	for(i = Lvl_obj; i; i = i->i_link) {
		if(cansee(i->i_coords.y,i->i_coords.x)) {
			mvaddch(i->i_coords.y,i->i_coords.x,i->i_item);
		}
	}
	detecting = (Player.b_flags & DETECTING) != 0;
	for(m = Mlist; m; m = m->b_link) {
		move(by(m),bx(m));
		if(cansee(by(m),bx(m))) {
			if((m->b_flags & INVISIBLE) == 0 || Player.b_flags & SEE_INVISIBLE) {
				addch(m->b_app);
			} else {
				addch(Places[bx(m)][by(m)].p_ch);
			}
			continue;
		}
		if(detecting) {
			standout();
			addch(m->b_ch);
			standend();
		}
	}
	msg("Everything looks SO boring now.");
}

void visuals()
{
	register struct item *i;
	register struct being *m;
	bool detecting;
	if(!After || (Running && Jump)) return;
	for(i = Lvl_obj; i; i = i->i_link) {
		if(cansee(i->i_coords.y,i->i_coords.x)) {
			mvaddch(i->i_coords.y,i->i_coords.x,rnd_thing());
		}
	}
	if(!Seenstairs && cansee(Stairs.y,Stairs.x)) {
		mvaddch(Stairs.y,Stairs.x,rnd_thing());
	}
	detecting = (Player.b_flags & DETECTING) != 0;
	for(m = Mlist; m; m = m->b_link) {
		move(by(m),bx(m));
		if(see_monst(m)) {
			if(m->b_ch == XEROC && m->b_app != XEROC) {
				addch(rnd_thing());
			} else {
				addch(RANDMONST);
			}
		} else if(detecting) {
			standout();
			addch(RANDMONST);
			standend();
		}
	}
}

/* ARGSUSED */
void land(int junk)
{
	Player.b_flags &= ~LEVITATED;
	msg(choose_str("bummer!  You've hit the ground","you float gently to the ground"));
}

