/*
 *	ROGUE
 *
 *	@(#)misc.c	1.1	Strathclyde	10/31/84
 */

# include "param.h"
# include "types.h"
# include "extern.h"

char T_chars[] = "!?=/:)]%*,";

void eat()
{
	register struct item *food;
	if((food = get_item("eat",FOOD)) == 0) return;
	if(food->i_item != FOOD) {
		if(Verbose) {
			msg("ugh, you would get ill if you ate that");
		} else {
			msg("that's Inedible!");
		}
		return;
	}
	if(Food_left < 0) Food_left = 0;
	Food_left += rnd(400)+1100;
	if(Food_left > 2000) Food_left = 2000;
	Hungry_state = 0;
	if(food == Cur_weapon) Cur_weapon = 0;
	leave_pack(food,FALSE,FALSE);
	if(food->i_type == 1) {
		msg("my, that was a yummy %s",Fruit);
	} else {
		if(rnd(100) > 70) {
			Player.b_stats.experience++;
			msg("%s, this food tastes awful",choose_str("bummer","yuk"));
			check_level();
		} else {
			msg("%s, that tasted good",choose_str("oh, wow","yum"));
		}
	}
}

void check_level()
{
	register int level, extra_hp, oldlevel;
	for(level = 0; E_levels[level]; level++) {
		if(E_levels[level] > Player.b_stats.experience) break;
	}
	level++;
	oldlevel = Player.b_stats.level;
	Player.b_stats.level = level;
	if(level > oldlevel) {
		extra_hp = roll(level-oldlevel,10);
		Player.b_stats.maxhp += extra_hp;
		Player.b_stats.hp += extra_hp;
		msg("welcome to level %d",level);
	}
}

void chg_str(register int change)
{
	int strength;
	if(change == 0) return;
	add_str(&Player.b_stats.strength,change);
	strength = Player.b_stats.strength;
	if(lring(R_ADD_STRENGTH)) add_str(&strength,-Cur_ring[LEFT]->i_value);
	if(rring(R_ADD_STRENGTH)) add_str(&strength,-Cur_ring[RIGHT]->i_value);
	if(strength > Max_stats.strength) Max_stats.strength = strength;
}

void add_str(register int *strength,int change)
{
	*strength += change;
	if(*strength < 3) {
		*strength = 3;
	} else if(*strength > 31) {
		*strength = 31;
	}
}

int add_haste(bool temp)
{
	if(Player.b_flags & FAST) {
		No_command += rnd(8);
		Player.b_flags &= ~(MOBILE | FAST);
		extinguish(nohaste);
		msg("you faint from exhaustion");
		return(FALSE);
	}
	Player.b_flags |= FAST;
	if(temp) fuse(nohaste,0,rnd(4)+4,spread(2));
	return(TRUE);
}

void aggravate()
{
	register struct being *monst;
	for(monst = Mlist; monst; monst = monst->b_link) runto(&bc(monst));
}

char *vowelstr(register char *word)
{
	switch(*word) {
	case 'a':
	case 'e':
	case 'i':
	case 'o':
	case 'u':
	case 'A':
	case 'E':
	case 'I':
	case 'O':
	case 'U':
		return("n");
	}
	return("");
}

int is_current(register struct item *thing)
{
	if(thing == 0) return(FALSE);
	if(thing == Cur_armor || thing == Cur_weapon || thing == Cur_ring[LEFT] || thing == Cur_ring[RIGHT]) {
		if(Verbose) addmsg("That's already ");
		msg("in use");
		return(TRUE);
	} else {
		return(FALSE);
	}
}

int get_dir()
{
	register char *prompt;
	bool gotit;
	if(Verbose) {
		prompt = "which direction? ";
		msg(prompt);
	} else {
		prompt = "direction: ";
	}
	do {
		gotit = TRUE;
		switch(Dir_ch = readchar()) {
		case 'h':
		case 'H':
			Delta.y = 0;
			Delta.x = -1;
			break;
		case 'j':
		case 'J':
			Delta.y = 1;
			Delta.x = 0;
			break;
		case 'k':
		case 'K':
			Delta.y = -1;
			Delta.x = 0;
			break;
		case 'l':
		case 'L':
			Delta.y = 0;
			Delta.x = 1;
			break;
		case 'y':
		case 'Y':
			Delta.y = -1;
			Delta.x = -1;
			break;
		case 'u':
		case 'U':
			Delta.y = -1;
			Delta.x = 1;
			break;
		case 'b':
		case 'B':
			Delta.y = 1;
			Delta.x = -1;
			break;
		case 'n':
		case 'N':
			Delta.y = 1;
			Delta.x = 1;
			break;
		case ESC:
			return(FALSE);
		default:
			Mpos = 0;
			msg(prompt);
			gotit = FALSE;
		}
	} while(!gotit);
	if(isupper(Dir_ch)) Dir_ch += ('a'-'A');
	if((Player.b_flags & CONFUSED) && rnd(5) == 0) {
		do {
			Delta.y = rnd(3)-1;
			Delta.x = rnd(3)-1;
		} while(Delta.y == 0 && Delta.x == 0);
	}
	Mpos = 0;
	return(TRUE);
}

int sign(int x)
{
	if(x < 0) return(-1);
	return(x > 0);
}

int spread(int x)
{
	return(x-x/20+rnd(x/10));
}

void call_it(register struct info *inf)
{
	if(inf->inf_known) {
		if(inf->inf_called) {
			cfree(inf->inf_called);
			inf->inf_called = 0;
		}
	} else if(inf->inf_called == 0) {
		msg(Terse ? "call it: " : "what do you want to call it? ");
		if(get_str(Prbuf,stdscr) == 0) {
			if(inf->inf_called) cfree(inf->inf_called);
			inf->inf_called = malloc(strlen(Prbuf)+1);
			strcpy(inf->inf_called,Prbuf);
		}
	}
}

int rnd_thing()
{
	register int x;
	x = rnd(Level >= 26 ? 10 : 9);
	return(T_chars[x]);
}

char *choose_str(hip,straight)
	char *hip, *straight;
{
	return(Player.b_flags & HALLUCINATING ? hip : straight);
}

