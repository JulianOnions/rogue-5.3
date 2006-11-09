/*
 *	ROGUE
 *
 *	@(#)armor.c	1.1	Strathclyde	10/31/84
 */

# include "param.h"
# include "types.h"
# include "extern.h"

void wear()
{
	register struct item *armor;
	register char *name;
	if((armor = get_item("wear",ARMOUR)) == 0) return;
	if(armor->i_item != ARMOUR) {
		msg("you can't wear that");
		return;
	}
	if(Cur_armor && !take_off()) {
		After = FALSE;
		return;
	}
	waste_time();
	armor->i_flags |= KNOWN;
	name = inv_name(armor,TRUE);
	Cur_armor = armor;
	if(Verbose) addmsg("you are now ");
	msg("wearing %s",name);
}

int take_off()
{
	register struct item *armor;
	if((armor = Cur_armor) == 0) {
		After = FALSE;
		if(Terse) {
			msg("not wearing armor");
		} else {
			msg("you aren't wearing any armor");
		}
		return(FALSE);
	}
	if(!dropcheck(Cur_armor)) return(FALSE);
	Cur_armor = 0;
	msg("taken off %c) %s",armor->i_packchar,inv_name(armor,TRUE));
	return(TRUE);
}

