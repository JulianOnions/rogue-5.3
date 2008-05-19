/*
 *	ROGUE
 *
 *	@(#)fight.c	1.1	Strathclyde	10/31/84
 */

# include "param.h"
# include "types.h"
# include "extern.h"

static void remove_item(register struct coords *position,
			register struct being *monst,
			bool drop_pack);
static void thunk(register struct item *weapon, 
		  register char *mname,
		  bool terse);
static int roll_em(struct being *attacker, 
		   struct being *victim,
		   struct item *weapon,
		   bool throw);
static void hit(register char *hitter, 
	 char *hitted,
	 bool terse);
static void miss(
    register char *misser, char *missed,
    bool terse);

static void bounce(register struct item *weapon,
		   char *missed,
		   bool terse);

char *H_names[8] = {
	" scored an excellent hit on ",
	" hit ",
	" have injured ",
	" swing and hit ",
	" scored an excellent hit on ",
	" hit ",
	" has injured ",
	" swings and hits "
};

char *M_names[8] = {
	" miss",
	" swing and miss",
	" barely miss",
	" don't hit",
	" misses",
	" swings and misses",
	" barely misses",
	" doesn't hit"
};

int Str_plus[32] = {
	-7,	-6,	-5,	-4,	-3,	-2,	-1,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	1,	1,	1,	1,	2,	2,	2,
	2,	2,	2,	2,	2,	2,	2,	3
};

int Add_dam[32] = {
	-7,	-6,	-5,	-4,	-3,	-2,	-1,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	1,	1,	2,	3,	3,	4,	5,	5,
	5,	5,	5,	5,	5,	5,	5,	6
};

int fight(register struct coords *mpos,
	  register struct item *weapon,
	  bool throw)
{
	bool confused = TRUE;
	char mch;
	register struct being *monst = Places[mpos->x][mpos->y].p_being;
	register char *mname;
	Count = 0;
	Quiet = 0;
	runto(mpos);
	mch = 0;
	if(monst->b_ch == XEROC && monst->b_app != XEROC && (Player.b_flags & BLIND) == 0) {
		monst->b_app = XEROC;
		if(Player.b_flags & HALLUCINATING) {
			mch = RANDMONST;
			mvaddch(by(monst),bx(monst),mch);
		}
		msg(choose_str("heavy!  That's a nasty critter!","wait!  That's a xeroc!"));
		if(!throw) return(FALSE);
	}
	mname = set_mname(monst);
	confused = FALSE;
	Has_hit = Terse && !To_death;
	if(roll_em(&Player,monst,weapon,throw)) {
		confused = FALSE;
		if(throw) {
			thunk(weapon,mname,Terse);
		} else {
			hit(0,mname,Terse);
		}
		if(Player.b_flags & MONST_CONFUSION) {
			confused = TRUE;
			monst->b_flags |= CONFUSED;
			Player.b_flags &= ~MONST_CONFUSION;
			endmsg();
			Has_hit = FALSE;
			msg("your hands stop glowing %s",pick_colour("red"));
		}
		if(monst->b_stats.hp <= 0) {
			killed(monst,TRUE);
		} else if(confused && (Player.b_flags & BLIND) == 0) {
			msg("%s appears confused",mname);
		}
		confused = TRUE;
	} else if(throw) {
		bounce(weapon,mname,Terse);
	} else {
		miss(0,mname,Terse);
	}
	return(confused);
}

int attack(register struct being *m)
{
	register char *mname;
	register int oldhp;
	register int ct;
	register struct item *find, *stolen;
	int retval = 0;

	Running = 0;
	Count = 0;
	Quiet = 0;
	if(To_death && (m->b_flags & FIGHTING) == 0) {
		To_death = FALSE;
		Kamikaze = FALSE;
	}
	if(m->b_ch == XEROC && m->b_app != XEROC && (Player.b_flags & BLIND) == 0) {
		m->b_app = XEROC;
		if(Player.b_flags & HALLUCINATING) {
			mvaddch(by(m),bx(m),RANDMONST);
		}
	}
	mname = set_mname(m);
	oldhp = Player.b_stats.hp;
	if(roll_em(m,&Player,0,FALSE)) {
		if(m->b_ch != ICE_MONSTER) {
			if(Has_hit) addmsg(".  ");
			hit(mname,0,FALSE);
		} else if(Has_hit) {
			endmsg();
		}
		Has_hit = FALSE;
		if(Player.b_stats.hp <= 0) {
			death(m->b_ch);
		} else if(!Kamikaze) {
			oldhp -= Player.b_stats.hp;
			if(oldhp > Max_hit) Max_hit = oldhp;
			if(Player.b_stats.hp <= Max_hit) To_death = FALSE;
		}
		if((m->b_flags & CANCELLED) == 0) {
			switch(m->b_ch) {
			case AQUATOR:
				rust_armor(Cur_armor);
				break;
			case ICE_MONSTER:
				if(Player.b_flags & BLIND) break;
				Player.b_flags &= ~MOBILE;
				if(No_command == 0) {
					addmsg("you are frozen");
					if(Verbose) {
						addmsg(" by the %s",mname);
					}
					endmsg();
				}
				No_command += rnd(2)+2;
				if(No_command > 50) death('h');
				break;
			case RATTLESNAKE:
				if(save(0)) break;
				if(!ring(R_SUSTAIN_STRENGTH)) {
					chg_str(-1);
					if(Verbose) {
						msg("you feel a bite in your leg and now feel weaker");
					} else {
						msg("a bite has weakened you");
					}
				} else if(!To_death) {
					if(Verbose) {
						msg("a bite momentarily weakens you");
					} else {
						msg("bite has no effect");
					}
				}
				break;
			case VAMPIRE:
			case WRAITH:
				if(rnd(100) < (m->b_ch == WRAITH ? 15 : 30)) {
					if(m->b_ch == WRAITH) {
						if(Player.b_stats.experience == 0) death(WRAITH);
						if(--Player.b_stats.level == 0) {
							Player.b_stats.experience = 0;
							Player.b_stats.level = 1;
						} else {
							Player.b_stats.experience = E_levels[Player.b_stats.level-1]+1;
						}
						ct = roll(1,10);
					} else {
						ct = roll(1,3);
					}
					Player.b_stats.hp -= ct;
					Player.b_stats.maxhp -= ct;
					if(Player.b_stats.hp <= 0) Player.b_stats.hp = 1;
					if(Player.b_stats.maxhp <= 0) death(m->b_ch);
					msg("you suddenly feel weaker");
				}
				break;
			case VENUS_FLYTRAP:
				Player.b_flags |= HELD;
				sprintf(Monsters[mindex(VENUS_FLYTRAP)].m_power,"%dx1",++Vf_hit);
				if(--Player.b_stats.hp <= 0) death(VENUS_FLYTRAP);
				break;
			case LEPRECHAUN:
				ct = Purse;
				Purse -= AU;
				if(!save(3)) Purse -= AU+AU+AU+AU;
				if(Purse < 0) Purse = 0;
				remove_item(&bc(m),m,FALSE);
				if(Purse != ct) msg("your purse feels lighter");
				retval = 1;
				break;
			case NYMPH:
				stolen = 0;
				ct = 0;
				for(find = Player.b_pack; find; find = find->i_link) {
					if(
						find != Cur_armor &&
						find != Cur_weapon &&
						find != Cur_ring[LEFT] &&
						find != Cur_ring[RIGHT] &&
						is_magic(find) &&
						rnd(++ct) == 0
					) {
						stolen = find;
					}
				}
				if(stolen) {
					remove_item(&bc(m),Places[bx(m)][by(m)].p_being,FALSE);
					leave_pack(stolen,FALSE,FALSE);
					msg("she stole %s!",inv_name(stolen,TRUE));
					discard(stolen);
				}
				retval = 1;
				break;
			}
		}
	} else if(m->b_ch != ICE_MONSTER) {
		if(Has_hit) {
			addmsg(".  ");
			Has_hit = FALSE;
		}
		if(m->b_ch == VENUS_FLYTRAP) {
			if((Player.b_stats.hp -= Vf_hit) <= 0) death(m->b_ch);
		}
		miss(mname,0,FALSE);
	}
	if(Fight_flush && !To_death) flush_type();
	Count = 0;
	status();
	return retval;
}

char *set_mname(m)
	register struct being *m;
{
	register int mix;
	register char *mname;
	static char buf[MAXSTR] = "the ";
	if(!see_monst(m) && (Player.b_flags & DETECTING) == 0) {
		return(Terse ? "it" : "something");
	}
	if(Player.b_flags & HALLUCINATING) {
		move(by(m),bx(m));
		mix = winch(stdscr);
		if(!isupper(mix)) {
			mix = rnd(26);
		} else {
			mix -= 'A';
		}
		mname = Monsters[mix].m_name;
	} else {
		mname = Monsters[mindex(m->b_ch)].m_name;
	}
	strcpy(&buf[4],mname);
	return(buf);
}

int swing(int level,int protection,int strength)
{
	register int r, x;
	r = rnd(20);
	x = 20-level-protection;
	return(r+strength >= x);
}

int roll_em(
    struct being *attacker, 
    struct being *victim,
    struct item *weapon,
    bool throw)
{
	bool has_hit = FALSE;
	register int *astrength = &attacker->b_stats.strength;
	register int *vstrength = &victim->b_stats.strength;
	register char *power;
	register int roll1, roll2, protection;
	int dexterity, damage, bang;
	if(weapon == 0) {
		power = attacker->b_stats.power;
		damage = 0;
		dexterity = 0;
	} else {
		dexterity = weapon ? weapon->i_hitplus : 0;
		damage = weapon ? weapon->i_damplus : 0;
		if(weapon == Cur_weapon) {
			if(lring(R_INCREASE_DAMAGE)) {
				damage += Cur_ring[LEFT]->i_value;
			} else if(lring(R_DEXTERITY)) {
				dexterity += Cur_ring[LEFT]->i_value;
			}
			if(rring(R_INCREASE_DAMAGE)) {
				damage += Cur_ring[RIGHT]->i_value;
			} else if(rring(R_DEXTERITY)) {
				dexterity += Cur_ring[RIGHT]->i_value;
			}
		}
		power = weapon->i_hpower;
		if(throw) {
			if((weapon->i_flags & THROW) && Cur_weapon && Cur_weapon->i_type == weapon->i_uses) {
				power = weapon->i_tpower;
				dexterity += Cur_weapon->i_hitplus;
				damage += Cur_weapon->i_damplus;
			} else if(weapon->i_uses < 0) {
				power = weapon->i_tpower;
			}
		}
	}
	if((victim->b_flags & MOBILE) == 0) dexterity += 4;
	protection = victim->b_stats.protection;
	if(vstrength == &Player.b_stats.strength) {
		if(Cur_armor) protection = Cur_armor->i_value;
		if(lring(R_PROTECTION)) protection -= Cur_ring[LEFT]->i_value;
		if(rring(R_PROTECTION)) protection -= Cur_ring[RIGHT]->i_value;
	}
	roll1 = atoi(power);
	while((power = index(power,'x'))) {
		roll2 = atoi(++power);
		if(swing(attacker->b_stats.level,protection,Str_plus[*astrength]+dexterity)) {
			victim->b_stats.hp -= (bang = roll(roll1,roll2)+Add_dam[*astrength]+damage) < 0 ? 0 : bang;
			has_hit = TRUE;
		}
		if((power = index(power,'/')) == 0) break;
		roll1 = atoi(++power);
	}
	return(has_hit);
}

char *prname(monstname,upper)
	register char *monstname;
	bool upper;
{
	static char buf[MAXSTR];
	buf[0] = 0;
	strcpy(buf,monstname == 0 ? "you" : monstname);
	if(upper) buf[0] -= 040;
	return(buf);
}

void thunk(register struct item *weapon, 
	   register char *mname,
	   bool terse)
{
	if(To_death) return;
	if(weapon->i_item == WEAPON) {
		addmsg("the %s hits ",Weap_info[weapon->i_type].inf_name);
	} else {
		addmsg("you hit ");
	}
	addmsg("%s",mname);
	if(!terse) endmsg();
}

void hit(register char *hitter, 
	 char *hitted,
	 bool terse)
{
	register int hindex;
	register char *hname;
	if(To_death) return;
	addmsg(prname(hitter,TRUE));
	if(Terse) {
		hname = " hit";
	} else {
		hindex = rnd(4);
		if(hitter) hindex += 4;
		hname = H_names[hindex];
	}
	addmsg(hname);
	if(Verbose) addmsg(prname(hitted,FALSE));
	if(!terse) endmsg();
}

void miss(
    register char *misser, char *missed,
    bool terse)
{
	register int mix;
	if(To_death) return;
	addmsg(prname(misser,TRUE));
	mix = Terse ? 0 : rnd(4);
	if(misser) mix += 4;
	addmsg(M_names[mix]);
	if(Verbose) addmsg(" %s",prname(missed,FALSE));
	if(!terse) endmsg();
}

void bounce(register struct item *weapon,
	    char *missed,
	    bool terse)
{
	if(To_death) return;
	if(weapon->i_item == WEAPON) {
		addmsg("the %s misses ",Weap_info[weapon->i_type].inf_name);
	} else {
		addmsg("you missed ");
	}
	addmsg(missed);
	if(!terse) endmsg();
}

void remove_item(register struct coords *position,
		 register struct being *monst,
		 bool drop_pack)
{
	register struct item *thing, *link;
	for(thing = monst->b_pack; thing;) {
		link = thing->i_link;
		thing->i_coords = monst->b_coords;
		_detach(&monst->b_pack,thing);
		if(drop_pack) {
			fall(thing,FALSE);
		} else {
			discard(thing);
		}
		thing = link;
	}
	Places[position->x][position->y].p_being = 0;
	mvaddch(position->y,position->x,monst->b_oldch);
	_detach(&Mlist,monst);
	if(monst->b_flags & FIGHTING) {
		Kamikaze = FALSE;
		To_death = FALSE;
		if(Fight_flush) flush_type();
	}
	discard(monst);
}

void killed(register struct being *monst,
	    bool notify)
{
	register char *mname;
	register struct item *thing;
	Player.b_stats.experience += monst->b_stats.experience;
	if(monst->b_ch == VENUS_FLYTRAP) {
		Player.b_flags &= ~HELD;
		Vf_hit = 0;
		strcpy(Monsters[mindex(VENUS_FLYTRAP)].m_power,"");
	} else if(monst->b_ch == LEPRECHAUN) {
		if(fallpos(&monst->b_coords,&monst->b_room->r_gold) && Level >= Max_level) {
			thing = new_item();
			thing->i_item = GOLD;
			thing->i_value = AU;
			if(save(3)) thing->i_value += AU+AU+AU+AU;
			_attach(&monst->b_pack,thing);
		}
	}
	mname = set_mname(monst);
	remove_item(&monst->b_coords,monst,TRUE);
	if(notify) {
		if(Has_hit) {
			addmsg(".  Defeated ");
			Has_hit = FALSE;
		} else {
			if(Verbose) addmsg("you have ");
			addmsg("defeated ");
		}
		msg(mname);
	}
	check_level();
	if(Fight_flush) flush_type();
}

