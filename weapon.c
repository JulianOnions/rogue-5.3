/*
 *	ROGUE
 *
 *	@(#)weapon.c	1.1	Strathclyde	10/31/84
 */

# include "param.h"
# include "types.h"
# include "extern.h"

int	Group = 2;

void set_know(register struct item *thing, register struct info *inf);

struct idam {
	char	*id_hpower;
	char	*id_tpower;
	char	id_uses;
	int	id_flags;
} Init_dam[] = {
{	"2x4",	"1x3",	-1,		0}, /* MACE */
{	"3x4",	"1x2",	-1,		0}, /* LONG SWORD */
{	"1x1",	"1x1",	-1,		0}, /* SHORT BOW */
{	"1x1",	"2x3",	SHORT_BOW,	THROW | GROUP},	/* ARROW */
{	"1x6",	"1x4",	-1,		THROW},	/* DAGGER */
{	"4x4",	"1x2",	-1,		0}, /* TWO HANDED SWORD */
{	"1x1",	"1x3",	-1,		THROW | GROUP},	/* DART */
{	"1x2",	"2x4",	-1,		THROW | GROUP},	/* SHURIKEN */
    {	"2x3",	"1x6",	-1,		THROW} /* SPEAR */
};

struct tname {
	char	t_ch;
	char	*t_str;
	int	t_l8;
} Type_names[] = {
{	POTION,	"potion",			0},
{	SCROLL,	"scroll",			0},
{	FOOD,	"food",				0},
{	-2,	"ring, wand or staff",		0},
{	RING,	"ring",				0},
{	STICK,	"wand or staff",		0},
{	WEAPON,	"weapon",			0},
{	ARMOUR,	"suit of armor",		0},
    {	0,	0,				0}
};

void init_weapon(register struct item *weapon, int type)
{
	register struct idam *idam;
	weapon->i_item = WEAPON;
	weapon->i_type = type;
	idam = &Init_dam[type];
	weapon->i_hpower = idam->id_hpower;
	weapon->i_tpower = idam->id_tpower;
	weapon->i_uses = idam->id_uses;
	weapon->i_flags = idam->id_flags;
	weapon->i_hitplus = 0;
	weapon->i_damplus = 0;
	if(type == DAGGER) {
		weapon->i_number = rnd(4)+2;
		weapon->i_group = Group++;
	} else if(weapon->i_flags & GROUP) {
		weapon->i_number = rnd(8)+8;
		weapon->i_group = Group++;
	} else {
		weapon->i_number = 1;
		weapon->i_group = 0;
	}
}

int hit_monster(register int y, int x, struct item *weapon)
{
	static struct coords mpos;
	mpos.y = y;
	mpos.x = x;
	return fight(&mpos,weapon,TRUE);
}

char *num(register int num1, int num2, char item)
{
	static char buf[12];
	sprintf(buf,num1 < 0 ? "%d" : "+%d",num1);
	if(item == WEAPON) sprintf(&buf[strlen(buf)],num2 < 0 ? ",%d" : ",+%d",num2);
	return(buf);
}

void wield()
{
	register struct item *new, *old;
	register char *invname;
	old = Cur_weapon;
	if(!dropcheck(old)) {
		Cur_weapon = old;
		return;
	}
	Cur_weapon = old;
	if((new = get_item("wield",WEAPON)) == 0) {
		After = FALSE;
		return;
	}
	if(new->i_item == ARMOUR) {
		msg("you can't wield armor");
		After = FALSE;
		return;
	}
	if(is_current(new)) {
		After = FALSE;
		return;
	}
	invname = inv_name(new,TRUE);
	Cur_weapon = new;
	if(Verbose) addmsg("you are now ");
	msg("wielding %s (%c)",invname,new->i_packchar);
}

int fallpos(register struct coords *fall_from, struct coords *fall_at)
{
	register int y, x, count;
	register int floorch;
	count = 0;
	for(y = fall_from->y-1; y <= fall_from->y+1; y++) {
		for(x = fall_from->x-1; x <= fall_from->x+1; x++) {
			if(y != MYY || x != MYX) {
				floorch = Places[x][y].p_ch;
				if(floorch == FLOOR || floorch == PASSAGE) {
					if(rnd(++count) == 0) {
						fall_at->y = y;
						fall_at->x = x;
					}
				}
			}
		}
	}
	return(count != 0);
}

void whatis(bool check, int item)
{
	register struct item *thing;
	if(Player.b_pack == 0) {
		msg("you don't have anything in your pack to identify");
		return;
	}
	for(;;) {
		thing = get_item("identify",item);
		if(!check) break;
		if(N_objs == 0) return;
		if(thing) {
			if(
				item == 0 ||
				thing->i_item == item ||
				(item == -2 && (thing->i_item == RING || thing->i_item == STICK))
			) break;
			msg("you must identify a %s",type_name(item));
		} else {
			msg("you must identify something");
		}
	}
	if(thing) {
		switch(thing->i_item) {
		case SCROLL:
			set_know(thing,Scr_info);
			break;
		case POTION:
			set_know(thing,Pot_info);
			break;
		case STICK:
			set_know(thing,Ws_info);
			break;
		case WEAPON:
		case ARMOUR:
			thing->i_flags |= KNOWN;
			break;
		case RING:
			set_know(thing,Ring_info);
		}
		msg(inv_name(thing,FALSE));
	}
}

void set_know(register struct item *thing, register struct info *inf)
{
	register char **called;
	inf[thing->i_type].inf_known = TRUE;
	thing->i_flags |= KNOWN;
	called = &inf[thing->i_type].inf_called;
	if(*called) {
		free(*called);
		*called = 0;
	}
}

char *type_name(item)
	int item;
{
	register struct tname *type;
	for(type = Type_names; type->t_ch; type++) {
		if(item == type->t_ch) return(type->t_str);
	}
	return("whatsit");
}

void teleport()
{
	static struct coords newpos;
	mvaddch(MYY,MYX,floor_at());
	find_floor((struct room *)0,&newpos,0,TRUE);
	if(roomin(&newpos) != Player.b_room) {
		leave_room(&ME);
		ME = newpos;
		enter_room(&ME);
	} else {
		ME = newpos;
		look(TRUE);
	}
	mvaddch(MYY,MYX,PLAYER);
	if(Player.b_flags & HELD) {
		Player.b_flags &= ~HELD;
		Vf_hit = 0;
		strcpy(Monsters[mindex(VENUS_FLYTRAP)].m_power,"000x0");
	}
	No_move = 0;
	Count = 0;
	Running = FALSE;
	if (Fight_flush) flush_type();
}

