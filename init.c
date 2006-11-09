/*
 *	ROGUE
 *
 *	@(#)init.c	1.1	Strathclyde	10/31/84
 */

# include "param.h"
# include "types.h"
# include "extern.h"

# define COLOURS	27
# define SYLLS		147
# define STONES		26
# define WOODS		33
# define METALS		22

char *Rainbow[COLOURS] = {
	"amber",
	"aquamarine",
	"black",
	"blue",
	"brown",
	"clear",
	"crimson",
	"cyan",
	"ecru",
	"gold",
	"green",
	"grey",
	"magenta",
	"orange",
	"pink",
	"plaid",
	"purple",
	"red",
	"silver",
	"tan",
	"tangerine",
	"topaz",
	"turquoise",
	"vermilion",
	"violet",
	"white",
	"yellow"
};

char *Sylls[SYLLS] = {
	"a",
	"ab",
	"ag",
	"aks",
	"ala",
	"an",
	"app",
	"arg",
	"arze",
	"ash",
	"bek",
	"bie",
	"bit",
	"bjor",
	"blu",
	"bot",
	"bu",
	"byt",
	"comp",
	"con",
	"cos",
	"cre",
	"dalf",
	"dan",
	"den",
	"do",
	"e",
	"eep",
	"el",
	"eng",
	"er",
	"ere",
	"erk",
	"esh",
	"evs",
	"fa",
	"fid",
	"fri",
	"fu",
	"gan",
	"gar",
	"glen",
	"gop",
	"gre",
	"ha",
	"hyd",
	"i",
	"ing",
	"ip",
	"ish",
	"it",
	"ite",
	"iv",
	"jo",
	"kho",
	"kli",
	"klis",
	"la",
	"lech",
	"mar",
	"me",
	"mi",
	"mic",
	"mik",
	"mon",
	"mung",
	"mur",
	"nej",
	"nelg",
	"nep",
	"ner",
	"nes",
	"nes",
	"nih",
	"nin",
	"o",
	"od",
	"ood",
	"org",
	"orn",
	"ox",
	"oxy",
	"pay",
	"ple",
	"plu",
	"po",
	"pot",
	"prok",
	"re",
	"rea",
	"rhov",
	"ri",
	"ro",
	"rog",
	"rok",
	"rol",
	"sa",
	"san",
	"sat",
	"sef",
	"seh",
	"shu",
	"ski",
	"sna",
	"sne",
	"snik",
	"sno",
	"so",
	"sol",
	"sri",
	"sta",
	"sun",
	"ta",
	"tab",
	"tem",
	"ther",
	"ti",
	"tox",
	"trol",
	"tue",
	"turs",
	"u",
	"ulk",
	"um",
	"un",
	"uni",
	"ur",
	"val",
	"viv",
	"vly",
	"vom",
	"wah",
	"wed",
	"werg",
	"wex",
	"whon",
	"wun",
	"xo",
	"y",
	"yot",
	"yu",
	"zant",
	"zeb",
	"zim",
	"zok",
	"zon",
	"zum"
};

struct {
	char	*stone;
	int	value;
} Stones[STONES] = {
{	"agate",		25},
{	"alexandrite",		40},
{	"amethyst",		50},
{	"carnelian",		40},
{	"diamond",		300},
{	"emerald",		300},
{	"germanium",		225},
{	"granite",		5},
{	"garnet",		50},
{	"jade",			150},
{	"kryptonite",		300},
{	"lapis lazuli",		50},
{	"moonstone",		50},
{	"obsidian",		15},
{	"onyx",			60},
{	"opal",			200},
{	"pearl",		220},
{	"peridot",		63},
{	"ruby",			350},
{	"sapphire",		285},
{	"stibotantalite",	200},
{	"tiger eye",		50},
{	"topaz",		60},
{	"turquoise",		70},
{	"taaffeite",		300},
{	"zircon",		80}
};

char *Wood[WOODS] = {
	"avocado wood",
	"balsa",
	"bamboo",
	"banyan",
	"birch",
	"cedar",
	"cherry",
	"cinnibar",
	"cypress",
	"dogwood",
	"driftwood",
	"ebony",
	"elm",
	"eucalyptus",
	"fall",
	"hemlock",
	"holly",
	"ironwood",
	"kukui wood",
	"mahogany",
	"manzanita",
	"maple",
	"oaken",
	"persimmon wood",
	"pecan",
	"pine",
	"poplar",
	"redwood",
	"rosewood",
	"spruce",
	"teak",
	"walnut",
	"zebrawood"
};

char *Metal[METALS] = {
	"aluminum",
	"beryllium",
	"bone",
	"brass",
	"bronze",
	"copper",
	"electrum",
	"gold",
	"iron",
	"lead",
	"magnesium",
	"mercury",
	"nickel",
	"pewter",
	"platinum",
	"steel",
	"silver",
	"silicon",
	"tin",
	"titanium",
	"tungsten",
	"zinc"
};

bool	Used[36];
bool	Used2[24];

void init_player()
{
	register struct item *thing;
	Player.b_stats = Max_stats;
	Food_left = 1300;
	thing = new_item();
	thing->i_item = FOOD;
	thing->i_number = 1;
	add_pack(thing,TRUE);
	thing = new_item();
	thing->i_item = ARMOUR;
	thing->i_type = 1;
	thing->i_value = A_class[1]-1;
	thing->i_flags |= KNOWN;
	thing->i_number = 1;
	Cur_armor = thing;
	add_pack(thing,TRUE);
	thing = new_item();
	init_weapon(thing,MACE);
	thing->i_hitplus = 1;
	thing->i_damplus = 1;
	thing->i_flags |= KNOWN;
	add_pack(thing,TRUE);
	Cur_weapon = thing;
	thing = new_item();
	init_weapon(thing,SHORT_BOW);
	thing->i_hitplus = 1;
	thing->i_flags |= KNOWN;
	add_pack(thing,TRUE);
	thing = new_item();
	init_weapon(thing,ARROW);
	thing->i_number += rnd(15)+25;
	thing->i_flags |= KNOWN;
	add_pack(thing,TRUE);
}

void init_colours()
{
	register int potion, colour;
	for(colour = 0; colour < COLOURS; colour++) Used[colour] = FALSE;
	for(potion = 0; potion < 14; potion++) {
		do {
			colour = rnd(COLOURS);
		} while(Used[colour]);
		Used[colour] = TRUE;
		P_colours[potion] = Rainbow[colour];
	}
}

void init_names()
{
	register int nsylls;
	register char *name, *syll;
	register int scroll, words;
	for(scroll = 0; scroll < 18; scroll++) {
		name = Prbuf;
		for(words = rnd(4)+2; words--;) {
			nsylls = rnd(3)+1;
			while(nsylls--) {
				syll = Sylls[rnd(SYLLS)];
				if(strlen(syll)+name > &Prbuf[40]) break;
				while(*syll) *name++ = *syll++;
			}
			*name++ = ' ';
		}
		*--name = 0;
		S_names[scroll] = malloc(strlen(Prbuf)+1);
		strcpy(S_names[scroll],Prbuf);
	}
}

void init_stones()
{
	register int rng, stone;
	for(stone = 0; stone < STONES; stone++) Used[stone] = FALSE;
	for(rng = 0; rng < 14; rng++) {
		do {
			stone = rnd(STONES);
		} while(Used[stone]);
		Used[stone] = TRUE;
		R_stones[rng] = Stones[stone].stone;
		Ring_info[rng].inf_value += Stones[stone].value;
	}
}

void init_materials()
{
	register int stick, material;
	register char *mname;
	for(material = 0; material < WOODS; material++) Used[material] = FALSE;
	for(material = 0; material < METALS; material++) Used2[material] = FALSE;
	for(stick = 0; stick < 14; stick++) {
		for(;;) {
			if(rnd(2)) {
				material = rnd(WOODS);
				if(Used[material]) continue;
				Ws_type[stick] = "staff";
				mname = Wood[material];
				Used[material] = TRUE;
				break;
			} else {
				material = rnd(METALS);
				if(Used2[material]) continue;
				Ws_type[stick] = "wand";
				mname = Metal[material];
				Used2[material] = TRUE;
				break;
			}
		}
		Ws_made[stick] = mname;
	}
}

void init_probs()
{
	sumprobs(Things,7);
	sumprobs(Pot_info,14);
	sumprobs(Scr_info,18);
	sumprobs(Ring_info,14);
	sumprobs(Ws_info,14);
	sumprobs(Weap_info,9);
	sumprobs(Arm_info,8);
}

void sumprobs(register struct info *inf, int size)
{
	register struct info *endinf = &inf[size];
	while(++inf < endinf) inf->inf_prob += inf[-1].inf_prob;
}

char *pick_colour(char *colour)
{
	return(Player.b_flags & HALLUCINATING ? Rainbow[rnd(COLOURS)] : colour);
}

