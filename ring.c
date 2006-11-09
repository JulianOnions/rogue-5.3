/*
 *	ROGUE
 *
 *	@(#)ring.c	1.1	Strathclyde	10/31/84
 */

# include "param.h"
# include "types.h"
# include "extern.h"

int	R_eat[] = {
	 1,	/* Ring of protection */
	 1,	/* Ring of add strength */
	 1,	/* Ring of sustain strength */
	-3,	/* Ring of searching */
	-5,	/* Ring of see invisible */
	 0,	/* Ring of adornment */
	 0,	/* Ring of aggravate monster */
	-3,	/* Ring of dexterity */
	-3,	/* Ring of increase damage */
	 2,	/* Ring of regeneration */
	-2,	/* Ring of slow digestion */
	 0,	/* Ring of teleportation */
	 1,	/* Ring of stealth */
	 1	/* Ring of maintain armor */
};

void ring_on()
{
	register struct item *rng;
	register int hand;
	if((rng = get_item("put on",RING)) == 0) return;
	if(rng->i_item != RING) {
		msg(Verbose ? "it would be difficult to wrap that around a finger" : "not a ring");
		return;
	}
	if(is_current(rng)) return;
	if(Cur_ring[LEFT] == 0 && Cur_ring[RIGHT] == 0) {
		if((hand = gethand()) < 0) return;
	} else if(Cur_ring[LEFT] == 0) {
		hand = LEFT;
	} else if(Cur_ring[RIGHT] == 0) {
		hand = RIGHT;
	} else {
		msg(Verbose ? "you already have a ring on each hand" : "wearing two");
		return;
	}
	Cur_ring[hand] = rng;
	switch(rng->i_type) {
	case R_ADD_STRENGTH:
		chg_str(rng->i_value);
		break;
	case R_SEE_INVISIBLE:
		invis_on();
		break;
	case R_AGGRAVATE_MONSTER:
		aggravate();
	}
	if(Verbose) addmsg("you are now wearing ");
	msg("%s (%c)",inv_name(rng,TRUE),rng->i_packchar);
}

void ring_off()
{
	register int hand;
	register struct item *rng;
	if(Cur_ring[LEFT] == 0 && Cur_ring[RIGHT] == 0) {
		msg(Terse ? "no rings" : "you aren't wearing any rings");
		return;
	}
	if(Cur_ring[LEFT] == 0) {
		hand = RIGHT;
	} else if(Cur_ring[RIGHT] == 0) {
		hand = LEFT;
	} else {
		if((hand = gethand()) < 0) return;
	}
	Mpos = 0;
	if((rng = Cur_ring[hand]) == 0) {
		msg("not wearing such a ring");
		return;
	}
	if(dropcheck(rng)) msg("was wearing %s(%c)",inv_name(rng,TRUE),rng->i_packchar);
}

int gethand()
{
	register int hand;
	for(;;) {
		msg(Terse ? "left or right ring? " : "left hand or right hand? ");
		if((hand = readchar()) == ESC) return(-1);
		Mpos = 0;
		if(hand == 'l' || hand == 'L') return(LEFT);
		if(hand == 'r' || hand == 'R') return(RIGHT);
		msg(Terse ? "L or R" : "please type L or R");
	}
}

int ring_eat(int hand)
{
	register struct item *rng;
	register int food;
	if((rng = Cur_ring[hand]) == 0) return(0);
	if((food = R_eat[rng->i_type]) < 0) food = rnd(-food) == 0;
	if(rng->i_type == R_SLOW_DIGESTION) food = -food;
	return(food);
}

char *ring_num(register struct item *rng)
{
	static char buf[12];
	if(rng->i_flags & KNOWN) {
		switch(rng->i_type) {
		case R_PROTECTION:
		case R_ADD_STRENGTH:
		case R_DEXTERITY:
		case R_INCREASE_DAMAGE:
			sprintf(buf," [%s]",num(rng->i_value,0,RING));
			return(buf);
		}
	}
	return("");
}

