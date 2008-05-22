/*
 *	ROGUE
 *
 *	@(#)param.h	1.1	Strathclyde	10/31/84
 */

/*
 * Don't change the constants, since they are used for sizes in many
 * places in the program.
 */

# include <curses.h>
# include <ctype.h>
#include <stdlib.h>
#include <string.h>

# define	CLICK		64	/* beings/items allocated in CLICKs */

# define	MAXSTR		80	/* maximum length of strings */
# define	MAXLINES	32	/* maximum number of screen lines used */
# define	MAXCOLS		80	/* maximum number of screen columns used */
# define	MAXX		80	/* maximum X-coordinate */
# define	MAXY		23	/* maximum Y-coordinate */

# define	MAX_PACK		23
# define	DEF_VALUE	11

# define	ROOMS		9
# define	PASSAGES	13
# define	TRAPS		7
# define	OPTIONS		12
# define	THINGS		7
# define	POTIONS		14
# define	SCROLLS		18
# define	WEAPONS		9
# define	ARMOURS		8
# define	RINGS		14
# define	STICKS		14

# define	T_POTION	0
# define	T_SCROLL	1
# define	T_FOOD		2
# define	T_WEAPON	3
# define	T_ARMOUR	4
# define	T_RING		5
# define	T_STICK		6

# define	RN		(((Seed = Seed*11109+13849) >> 16) & 0xffff)

# define	AU		(rnd(Level*10+50)+2)

# ifdef CTRL
# undef CTRL
# endif
# define CTRL(c)		('c' & 037)

# define Verbose	(!Terse)

/* End types */

# define	END_KILLED		0
# define	END_QUIT		1
# define	END_WINNER		2
# define	END_KILLED_AMULET	3

/* Screen characters */

# define	FLOOR		'.'
# define	HWALL		'-'
# define	VWALL		'|'
# define	DOOR		'+'
# define	PASSAGE		'#'
# define	TRAP		'^'
# define	STAIRCASE	'%'
# define	POTION		'!'
# define	SCROLL		'?'
# define	RING		'='
# define	STICK		'/'
# define	FOOD		':'
# define	WEAPON		')'
# define	ARMOUR		']'
# define	GOLD		'*'
# define	AMULET		','
# define	PLAYER		'@'

/* Monster letters */

# define	AQUATOR		'A'
# define	BAT		'B'
# define	CENTAUR		'C'
# define	DRAGON		'D'
# define	EMU		'E'
# define	VENUS_FLYTRAP	'F'
# define	GRIFFIN		'G'
# define	HOBGOBLIN	'H'
# define	ICE_MONSTER	'I'
# define	JABBERWOCK	'J'
# define	KESTREL		'K'
# define	LEPRECHAUN	'L'
# define	MEDUSA		'M'
# define	NYMPH		'N'
# define	ORC		'O'
# define	PHANTOM		'P'
# define	QUAGGA		'Q'
# define	RATTLESNAKE	'R'
# define	SNAKE		'S'
# define	TROLL		'T'
# define	BLACK_UNICORN	'U'
# define	VAMPIRE		'V'
# define	WRAITH		'W'
# define	XEROC		'X'
# define	YETI		'Y'
# define	ZOMBIE		'Z'

/* Some manifests to do with monsters */

# define	DRAGON_RANGE		36

# define	RANDMONST	(rnd(26)+'A')
# define	mindex(m)	((m)-'A')

/* Status bits for b_flags
 * Some (e.g. LEVITATED/CANCELLED) differ in meaning
 * depending on whether they are applied to the player
 * or to monsters
 */

# define	MONST_CONFUSION	0x1
# define	SEE_INVISIBLE	0x2
# define	BLIND		0x4
# define	LEVITATED	0x8
# define	CANCELLED	0x8
# define	HAS_CONFUSED	0x10
# define	GUARD_GOLD	0x20
# define	FAST		0x40
# define	FIGHTING	0x80
# define	HELD		0x100
# define	CONFUSED	0x200
# define	INVISIBLE	0x400
# define	HALLUCINATING	0x800
# define	AGGRESSIVE	0x800
# define	FEARS_LIGHT	0x1000
# define	MOBILE		0x2000
# define	DETECTING	0x4000
# define	FAST_MOVE	0x4000
# define	SLOW		0x8000

/* Status bits for i_flags */

# define	CURSED		0x1
# define	KNOWN		0x2
# define	THROW		0x4
# define	GROUP		0x8
# define	DROPPED		0x10
# define	PROTECTED	0x20

/* potions */

# define	P_CONFUSION		0
# define	P_HALLUCINATION		1
# define	P_POISON		2
# define	P_GAIN_STRENGTH		3
# define	P_SEE_INVISIBLE		4
# define	P_HEALING		5
# define	P_MONSTER_DETECTION	6
# define	P_MAGIC_DETECTION	7
# define	P_RAISE_LEVEL		8
# define	P_EXTRA_HEALING		9
# define	P_HASTE_SELF		10
# define	P_RESTORE_STRENGTH	11
# define	P_BLINDNESS		12
# define	P_LEVITATION		13

/* scrolls */

# define	S_MONSTER_CONFUSION	0
# define	S_MAGIC_MAPPING		1
# define	S_HOLD_MONSTER		2
# define	S_SLEEP			3
# define	S_ENCHANT_ARMOUR	4
# define	S_IDENTIFY_POTION	5
# define	S_IDENTIFY_SCROLL	6
# define	S_IDENTIFY_WEAPON	7
# define	S_IDENTIFY_ARMOUR	8
# define	S_IDENTIFY_RWS		9
# define	S_SCARE_MONSTER		10
# define	S_FOOD_DETECTION	11
# define	S_TELEPORTATION		12
# define	S_ENCHANT_WEAPON	13
# define	S_CREATE_MONSTER	14
# define	S_REMOVE_CURSE		15
# define	S_AGGRAVATE_MONSTERS	16
# define	S_PROTECT_ARMOUR	17

/* rings */

# define	R_PROTECTION		0
# define	R_ADD_STRENGTH		1
# define	R_SUSTAIN_STRENGTH	2
# define	R_SEARCHING		3
# define	R_SEE_INVISIBLE		4
# define	R_ADORNMENT		5
# define	R_AGGRAVATE_MONSTER	6
# define	R_DEXTERITY		7
# define	R_INCREASE_DAMAGE	8
# define	R_REGENERATION		9
# define	R_SLOW_DIGESTION	10
# define	R_TELEPORTATION		11
# define	R_STEALTH		12
# define	R_MAINTAIN_ARMOUR	13

# define	LEFT	0
# define	RIGHT	1

# define lring(a)	(Cur_ring[LEFT] && Cur_ring[LEFT]->i_type == (a))
# define rring(a)	(Cur_ring[RIGHT] && Cur_ring[RIGHT]->i_type == (a))
# define ring(a)	(lring(a) || rring(a))

# define	TELECHANCE	50	/* When wearing ring */

#define WINCH(x) (winch(x)&A_CHARTEXT)
/* wands and staffs */

# define	WS_LIGHT		0
# define	WS_INVISIBILITY		1
# define	WS_LIGHTNING		2
# define	WS_FIRE			3
# define	WS_COLD			4
# define	WS_POLYMORPH		5
# define	WS_MAGIC_MISSILE	6
# define	WS_HASTE_MONSTER	7
# define	WS_SLOW_MONSTER		8
# define	WS_DRAIN_LIFE		9
# define	WS_NOTHING		10
# define	WS_TELEPORT_AWAY	11
# define	WS_TELEPORT_TO		12
# define	WS_CANCELLATION		13

/* weapons */

# define	MACE			0
# define	LONG_SWORD		1
# define	SHORT_BOW		2
# define	ARROW			3
# define	DAGGER			4
# define	TWO_HANDED_SWORD	5
# define	DART			6
# define	SHURIKEN		7
# define	SPEAR			8
# define	BOLT			9

/* Status bits for p_flags
 * The bottom four bits of p_flags are used
 * for a passage number or trap type number
 */

# define	P_PASSAGE	0x80
# define	P_DISCOVERED	0x40
# define	P_DROPPED	0x20
# define	P_KNOWN		0x10

# define	PASSNUM		0xf

/* Some manifests to do with places */

# define pch(x,y)		(Places[x][y].p_ch)
# define pflags(x,y)		(Places[x][y].p_flags)
# define pbeing(x,y)		(Places[x][y].p_being)
# define place_ch(x,y)		(pbeing(x,y) ? pbeing(x,y)->b_app : pch(x,y))

/* bits for r_flags */

# define	R_DARK		0x1
# define	R_PASSAGE	0x2
# define	R_MAZE		0x4

/* traps */

# define	TRAPDOOR		0
# define	ARROW_TRAP		1
# define	SLEEPING_GAS_TRAP	2
# define	BEAR_TRAP		3
# define	TELEPORT_TRAP		4
# define	POISON_DART_TRAP	5
# define	RUST_TRAP		6

# define	TMASK		0x7

/* coordinate stuff for conciseness */

# define ceq(a,b) ((a).x == (b).x && (a).y == (b).y)
# define cne(a,b) ((a).x != (b).x || (a).y != (b).y)

# define	xcof(a)	((a).x)
# define	ycof(a)	((a).y)

# define	ME	(Player.b_coords)

# define	bc(a)	((a)->b_coords)
# define	bx(a)	xcof(bc(a))
# define	by(a)	ycof(bc(a))

# define	MYX	(xcof(ME))
# define	MYY	(ycof(ME))

/* Characters */

# define	BEL	0x07
# define	BSP	0x08
# define	ESC	0x1b
# define	CTLH	0x08
# define	CTLJ	0x0a
# define	CTLK	0x0b
# define	CTLL	0x0c
# define	CTLY	0x19
# define	CTLU	0x15
# define	CTLB	0x02
# define	CTLN	0x0e
# define	CTLP	0x10
# define	CTLR	0x12

