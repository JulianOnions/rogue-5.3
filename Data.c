/*
 *	ROGUE
 *
 *	@(#)Data.c	1.1	Strathclyde	10/31/84
 */

# include "param.h"
# include "types.h"

char	Release[]	=	"5.3";

char encstr[] = {
	0300, 0153, 0174, 0174, 0140, 0251, 0131, 0056, 0047, 0305,
	0321, 0201, 0053, 0277, 0176, 0162, 0042, 0135, 0023, 0070,
	0137, 0223, 0075, 0061, 0341, 0051, 0222, 0212, 0241, 0164,
	0073, 0011, 0044, 0270, 0314, 0057, 0074, 0043, 0201, 0254,
	0
};

char statlist[] = {
	0355, 0153, 0154, 0173, 0053, 0204, 0255, 0313, 0151, 0144,
	0112, 0361, 0214, 0075, 0064, 0072, 0311, 0271, 0341, 0167,
	0113, 0074, 0312, 0031, 0071, 0213, 0054, 0054, 0067, 0271,
	0057, 0122, 0153, 0045, 0010, 0312, 0014, 0246
};

char	version[] = "rogue (Strathclyde) 26/7/84";

bool	Amulet = FALSE, Door_stop = FALSE, Fight_flush = FALSE,
	Firstmove = FALSE, Got_ltc = FALSE, Has_hit = FALSE, In_shell = FALSE,
	Inv_describe = TRUE, Jump = FALSE, Kamikaze = FALSE, Lower_msg = FALSE,
	Move_on = FALSE, Msg_esc = FALSE, Passgo = FALSE, Playing = TRUE,
	Q_comm = FALSE, Running = FALSE, Save_msg = TRUE, See_floor = TRUE,
	Stat_msg = FALSE, Terse = FALSE, To_death = FALSE, Tombstone = TRUE,
	Pick_up = TRUE, Show_level = FALSE;

bool	After, Noscore, Seenstairs;

char	Dir_ch,
	File_name[MAXSTR],
	Huh[MAXSTR],
	*P_colours[POTIONS],
	Prbuf[MAXSTR],
	*R_stones[RINGS],
	Runch,
	*S_names[SCROLLS],
	Take,
	Whoami[MAXSTR],
	*Ws_made[STICKS],
	*Ws_type[STICKS],
	Orig_dsusp;

int	N_objs, Ntraps, Max_hit, Max_level, Food_left, Dnum, Seed;
int	Fd_temp, Fd_hof;

bool	Pack_used[26];

char	Fruit[MAXSTR] = "slime-mold";
char	Home[MAXSTR] = "";

char	*Inv_t_name[] = {
	"Overwrite",
	"Slow",
	"Clear"
};

char	*Tr_name[] = {
	"a trapdoor",
	"an arrow trap",
	"a sleeping gas trap",
	"a beartrap",
	"a teleport trap",
	"a poison dart trap",
	"a rust trap",
	"a trap"
};

int	Hungry_state = 0, Inpack = 0, Inv_type = 0, Level = 1, Mpos = 0,
	No_food = 0;

int	A_class[] = {
	8,	7,	7,	6,	5,	4,	4,	3
};

int	Count = 0, Lastscore = -1, No_command = 0, No_move = 0, Purse = 0,
	Quiet = 0, Vf_hit = 0;

int	E_levels[] = {
	     10,	     20,	     40,	     80,	    160,
	    320,	    640,	   1300,	   2600,	   5200,
	  13000,	  26000,	  50000,	 100000,	 200000,
	 400000,	 800000,	2000000,	4000000,	8000000,
	      0
};

struct item	*Lvl_obj;
struct being	*Mlist;

WINDOW		*Hw;

struct stats Max_stats = {
	16,	0,	1,	10,	12,
	"1x4",
	12
};

# define ZC	{0,0}
# define ZC12	{ZC,ZC,ZC,ZC,ZC,ZC,ZC,ZC,ZC,ZC,ZC,ZC}

struct room	Passages[PASSAGES] = {
{	0,	0,	0,	0,	ZC,	0,	3,	0,	ZC12},
{	0,	0,	0,	0,	ZC,	0,	3,	0,	ZC12},
{	0,	0,	0,	0,	ZC,	0,	3,	0,	ZC12},
{	0,	0,	0,	0,	ZC,	0,	3,	0,	ZC12},
{	0,	0,	0,	0,	ZC,	0,	3,	0,	ZC12},
{	0,	0,	0,	0,	ZC,	0,	3,	0,	ZC12},
{	0,	0,	0,	0,	ZC,	0,	3,	0,	ZC12},
{	0,	0,	0,	0,	ZC,	0,	3,	0,	ZC12},
{	0,	0,	0,	0,	ZC,	0,	3,	0,	ZC12},
{	0,	0,	0,	0,	ZC,	0,	3,	0,	ZC12},
{	0,	0,	0,	0,	ZC,	0,	3,	0,	ZC12},
{	0,	0,	0,	0,	ZC,	0,	3,	0,	ZC12},
    {	0,	0,	0,	0,	ZC,	0,	3,	0,	ZC12}
};

char venusstat[10] = "%%%x0";

struct monster	Monsters[26] = {
{"aquator",	0,
	AGGRESSIVE,
	10,	20,	5,	2,	1,	"0x0/0x0",	0},
{"bat",		0,
	FAST_MOVE,
	10,	1,	1,	3,	1,	"1x2",		0},
{"centaur",	15,
	0,
	10,	17,	4,	4,	1,	"1x2/1x5/1x5",	0},
{"dragon",	100,
	AGGRESSIVE,
	10,	5000,	10,	-1,	1,	"1x8/1x8/3x10",	0},
{"emu",		0,
	AGGRESSIVE,
	10,	2,	1,	7,	1,	"1x2",		0},
{"venus flytrap",0,
	AGGRESSIVE,
	10,	80,	8,	3,	1,	venusstat,	0},
{"griffin",	20,
	AGGRESSIVE | FEARS_LIGHT | FAST_MOVE,
	10,	2000,	13,	2,	1,	"4x3/3x5",	0},
{"hobgoblin",	0,
	AGGRESSIVE,
	10,	3,	1,	5,	1,	"1x8",		0},
{"ice monster",	0,
	0,
	10,	5,	1,	9,	1,	"0x0",		0},
{"jabberwock",	70,
	0,
	10,	3000,	15,	6,	1,	"2x12/2x4",	0},
{"kestrel",	0,
	AGGRESSIVE | FAST_MOVE,
	10,	1,	1,	7,	1,	"1x4",		0},
{"leprechaun",	0,
	0,
	10,	10,	3,	8,	1,	"1x1",		0},
{"medusa",	40,
	AGGRESSIVE,
	10,	200,	8,	2,	1,	"3x4/3x4/2x5",	0},
{"nymph",	100,
	0,
	10,	37,	3,	9,	1,	"0x0",		0},
{"orc",		15,
	GUARD_GOLD,
	10,	5,	1,	6,	1,	"1x8",		0},
{"phantom",	0,
	INVISIBLE,
	10,	120,	8,	3,	1,	"4x4",		0},
{"quagga",	0,
	AGGRESSIVE,
	10,	15,	3,	3,	1,	"1x5/1x5",	0},
{"rattlesnake",	0,
	AGGRESSIVE,
	10,	9,	2,	3,	1,	"1x6",		0},
{"snake",	0,
	AGGRESSIVE,
	10,	2,	1,	5,	1,	"1x3",		0},
{"troll",	50,
	AGGRESSIVE | FEARS_LIGHT,
	10,	120,	6,	4,	1,	"1x8/1x8/2x6",	0},
{"black unicorn",0,
	AGGRESSIVE,
	10,	190,	7,	-2,	1,	"1x9/1x9/2x9",	0},
{"vampire",	20,
	AGGRESSIVE | FEARS_LIGHT,
	10,	350,	8,	1,	1,	"1x10",		0},
{"wraith",	0,
	0,
	10,	55,	5,	4,	1,	"1x6",		0},
{"xeroc",	30,
	0,
	10,	50,	4,	6,	1,	"4x4",		0},
{"yeti",		30,
	0,
	10,	50,	4,	6,	1,	"1x6/1x6",	0},
{"zombie",	0,
	AGGRESSIVE,
     10,	6,	2,	8,	1,	"1x8",		0}
};

struct info	Things[] = {
{	0,	26,	0,	0,	0},
{	0,	36,	0,	0,	0},
{	0,	16,	0,	0,	0},
{	0,	7,	0,	0,	0},
{	0,	7,	0,	0,	0},
{	0,	4,	0,	0,	0},
    {	0,	4,	0,	0,	0}
};

struct info	Arm_info[] = {
{	"leather armor",		20,	20,	0,	0},
{	"ring mail",			15,	25,	0,	0},
{	"studded leather armor",	15,	20,	0,	0},
{	"scale mail",			13,	30,	0,	0},
{	"chain mail",			12,	75,	0,	0},
{	"splint mail",			10,	80,	0,	0},
{	"banded mail",			10,	90,	0,	0},
    {	"plate mail",			5,	150,	0,	0}
};

struct info	Pot_info[POTIONS] = {
{	"confusion",		7,	5,	0,	0},
{	"hallucination",	8,	5,	0,	0},
{	"poison",		8,	5,	0,	0},
{	"gain strength",	13,	150,	0,	0},
{	"see invisible",	3,	100,	0,	0},
{	"healing",		13,	130,	0,	0},
{	"monster detection",	6,	130,	0,	0},
{	"magic detection",	6,	105,	0,	0},
{	"raise level",		2,	250,	0,	0},
{	"extra healing",	5,	200,	0,	0},
{	"haste self",		5,	190,	0,	0},
{	"restore strength",	13,	130,	0,	0},
{	"blindness",		5,	5,	0,	0},
{	"levitation",		6,	75,	0,	0}
};

struct info	Ring_info[RINGS] = {
{	"protection",		9,	400,	0,	0},
{	"add strength",		9,	400,	0,	0},
{	"sustain strength",	5,	280,	0,	0},
{	"searching",		10,	420,	0,	0},
{	"see invisible",	10,	310,	0,	0},
{	"adornment",		1,	10,	0,	0},
{	"aggravate monster",	10,	10,	0,	0},
{	"dexterity",		8,	440,	0,	0},
{	"increase damage",	8,	400,	0,	0},
{	"regeneration",		4,	460,	0,	0},
{	"slow digestion",	9,	240,	0,	0},
{	"teleportation",	5,	30,	0,	0},
{	"stealth",		7,	470,	0,	0},
{	"maintain armor",	5,	380,	0,	0}
};

struct info	Scr_info[SCROLLS] = {
{	"monster confusion",		7,	140,	0,	0},
{	"magic mapping",		4,	150,	0,	0},
{	"hold monster",			2,	180,	0,	0},
{	"sleep",			3,	5,	0,	0},
{	"enchant armor",		7,	160,	0,	0},
{	"identify potion",		10,	80,	0,	0},
{	"identify scroll",		10,	80,	0,	0},
{	"identify weapon",		6,	80,	0,	0},
{	"identify armor",		7,	100,	0,	0},
{	"identify ring, wand or staff",	10,	115,	0,	0},
{	"scare monster",		3,	200,	0,	0},
{	"food detection",		3,	60,	0,	0},
{	"teleportation",		5,	165,	0,	0},
{	"enchant weapon",		8,	150,	0,	0},
{	"create monster",		4,	75,	0,	0},
{	"remove curse",			7,	105,	0,	0},
{	"aggravate monsters",		3,	20,	0,	0},
{	"protect armor",		2,	250,	0,	0}
};

struct info	Weap_info[] = {
{	"mace",			11,	8,	0,	0},
{	"long sword",		11,	15,	0,	0},
{	"short bow",		12,	15,	0,	0},
{	"arrow",		12,	1,	0,	0},
{	"dagger",		8,	3,	0,	0},
{	"two handed sword",	10,	75,	0,	0},
{	"dart",			12,	2,	0,	0},
{	"shuriken",		12,	5,	0,	0},
{	"spear",		12,	5,	0,	0},
{	0,			0,	0,	0,	0}
};

struct info	Ws_info[STICKS] = {
{	"light",		12,	250,	0,	0},
{	"invisibility",		6,	5,	0,	0},
{	"lightning",		3,	330,	0,	0},
{	"fire",			3,	330,	0,	0},
{	"cold",			3,	330,	0,	0},
{	"polymorph",		15,	310,	0,	0},
{	"magic missile",	10,	170,	0,	0},
{	"haste monster",	10,	5,	0,	0},
{	"slow monster",		11,	350,	0,	0},
{	"drain life",		9,	300,	0,	0},
{	"nothing",		1,	5,	0,	0},
{	"teleport away",	6,	340,	0,	0},
{	"teleport to",		6,	50,	0,	0},
{	"cancellation",		5,	280,	0,	0}
};

char	Preturn[] =	"[Press return to continue]";
char	Pspace[] =	"--Press space to continue--";

struct coords	Delta, Oldpos, Stairs;

struct place	Places[MAXCOLS][MAXLINES];

struct item	*Cur_armor, *Cur_ring[2], *Cur_weapon;

struct being	Player;

struct room	*Oldrp, Rooms[ROOMS];

#ifdef TIOCGLTC
struct ltchars	Ltc;
#endif

struct coords	nh, Ch_ret;

