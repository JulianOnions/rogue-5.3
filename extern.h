/*
 *	ROGUE
 *
 *	@(#)extern.h	1.1	Strathclyde	10/31/84
 */

extern bool	Got_ltc, In_shell, Wizard, Noscore, Allscore;
extern bool	Playing, Q_comm;
extern bool	After, Running, Door_stop, Firstmove, Take, Move_on, Seenstairs;
extern bool	Terse, Jump, See_floor, Fight_flush, Passgo, Tombstone;
extern bool	Pick_up, Show_level;
extern bool	To_death, Kamikaze, Has_hit;
extern bool	Amulet;
extern bool	Inv_describe, Pack_used[];
extern bool	Save_msg, Lower_msg, Stat_msg, Msg_esc;

extern char	Release[], version[], encstr[], statlist[];
extern char	Dir_ch, Runch;
extern char	Fruit[], Orig_dsusp, Prbuf[];
extern char	Whoami[], Home[], File_name[], **environ;
extern char	*P_colours[], *S_names[], *R_stones[], *Ws_made[], *Ws_type[];
extern char	*Tr_name[], *Inv_t_name[];
extern char	Huh[], Preturn[], Pspace[];
extern char	*Numname, *Max_name;

extern int	Purse, Lastscore;
extern int	Max_hit, Vf_hit;
extern int	No_command, Count, No_move;
extern int	Quiet, Food_left, Hungry_state;
extern int	Inpack, N_objs;
extern int	Inv_type;
extern int	Fd_temp, Fd_hof;
extern int	Mpos;
extern int	No_food, Ntraps;
extern int	Level, Max_level, E_levels[];
extern int	A_class[];
extern int	Dnum, Seed;

extern unsigned int	numscores, Temp_days;

extern struct stats	Max_stats;
extern struct being	Player, *Mlist;

extern struct monster	Monsters[];

extern struct coords	nh;
extern struct coords	Ch_ret;
extern struct coords	Delta;
extern struct coords	Stairs;
extern struct coords	Oldpos;

extern struct place	Places[MAXCOLS][MAXLINES];
extern struct room	Rooms[ROOMS];
extern struct room	Passages[PASSAGES];

extern struct room	*Oldrp;

extern struct item *Lvl_obj;
extern struct item	*Cur_weapon, *Cur_armor, *Cur_ring[2];

extern struct info	Pot_info[], Scr_info[], Weap_info[], Arm_info[],
			Ring_info[], Ws_info[], Things[];

extern struct option	optlist[];

extern WINDOW	*Hw;

#ifdef TIOCGLTC
extern struct ltchars	Ltc;
#endif

/*
 * Function types
 */

char	*charge_str(), *choose_str(), *ctime(), *getenv(),
	*killname(), *nullstr(), *pick_colour(), *ring_num(), *set_mname(),
	*type_name(), *vowelstr(),
	*index(), *prname(), *missile_name();

char *nothing(char item);
char *inv_name(register struct item *thing, bool lower);

void 	leave(), quit();
int	endit(); 
void    tstp();

char *num(register int num1, int num2, char item);
char *pick_colour(char *colour);
char *ring_num(register struct item *rng);
void _attach(register struct item **list, struct item *thing);
void _detach(register struct item **list, struct item *thing);
void _free_list(register struct item **list);
void add_daemon(DFUNC func, int arg, int spread);
int add_haste(bool temp);
int add_line(char *line, ...);
void add_pack(register struct item *thing, bool silent);
void add_str(register int *strength,int change);
void addmsg(const char *message, ...);
void aggravate();
int attack(struct being *m);
void auto_save();
int be_trapped(register struct coords *pos);
struct being *wake_monster(int y, int x);
void call();
void call_it(struct info *inf);
int cansee(register int y, int x);
void check_level();
void chg_str(register int change);
void come_down();
void command();
void conn(int startnum, int destnum);
struct coords *rndmove(struct being *b);
void death(char cause);
int death_monst();
int diag_ok(register struct coords *from, struct coords *to);
void discard(register struct item *thing);
void discovered();
int dist(int y1, int x1, int y2, int x2);
int dist_cp(register struct coords *a, struct coords *b);
void do_daemons(register int spread);
void do_fuses(register int spread);
void do_motion(struct item *thing, int yinc, int xinc);
void do_move(int yinc,int xinc);
void do_passages();
void do_pot(int which, bool known);
void do_rooms();
void do_run(int dir);
void do_zap();
void doadd(const char *message, va_list ap);
void doctor(int junk);
void door(struct room *r, struct coords *pos);
void door_open(register struct room *r);
void draw_room(register struct room *room);
void drop();
int dropcheck(struct item *thing);
void eat();
void end_line();
int endmsg();
void enter_room(register struct coords *door);
void erase_lamp(register struct coords *oldpos, struct room *oldrp);
void err_msg(char *str);
int exp_add(struct being *monst);
void extinguish(DFUNC func);
void fall(register struct item *thing, bool mine);
int fallpos(register struct coords *fall_from, struct coords *fall_at);
int fight(struct coords *mpos, struct item *weapon, bool throw);
int find_floor(struct room *room,struct coords *c,int max_tries,bool monst);
void fire_bolt(struct coords *start, struct coords *delta,char *boltname);
void fix_stick(register struct item *stick);
char floor_at();
char floor_ch();
void flush_type();
void fuse(DFUNC func, int arg, int count, int spread);
int get_dir();
int get_str(void *str, WINDOW *win);
int gethand();
void give_pack( struct being *monst);
int hit_monster(register int y, int x, struct item *weapon);
void init_check();
void init_colours();
void init_materials();
void init_names();
void init_player();
void init_probs();
void init_stones();
void init_weapon(register struct item *weapon, int type);
int inventory(register struct item *list, register int type);
void invis_on();
int is_current(register struct item *thing);
int is_magic(register struct item *thing);
int issymlink(char *name);
struct item *leave_pack(struct item *thing, bool one, bool all);
void killed(register struct being *monst, bool notify);
void land(int junk);
void leave_room(register struct coords *door);
void lengthen(DFUNC func, int count);
int  lock_sc();
void look(bool wake);
void missile(int yinc, int xinc);
void money(register int amount);
int msg(const char *fmt, ...);
void my_exit(int status);
void new_level();
void new_monster(struct being *monst, char ch, struct coords *pos);
void nohaste();
void numpass(int y, int x);
void open_score();
void option();
char pack_char();
int pack_room(bool picked_up, struct item *thing);
void parse_opts(register char *env);
void passnum();
void pick_up();
void picky_inven();
void put_things();
void putpass(struct coords *pos);
void quaff();
char quest(char *str);
void raise_level();
int randmonster(bool wander);
void rd_score(struct scorefile *buf, int fd);
void read_scroll();
void read_scroll();
char readchar();
void restore(register char *filename);
int ring_eat(int hand);
int ring_eat(int hand);
void ring_off();
void ring_on();
int rnd(register int mod);
int rnd_room();
int rnd_thing();
int roll(register int times, int limit);
void runners(int junk);
void runto(register struct coords *c);
void rust_armor(register struct item *armor);
int save(register int v);
void save_game();
int save_throw(int v, struct being *b);
void score(int player_score, int end_type, char killtype);
void search();
int see_monst(register struct being *monst);
int seen_stairs();
void setup();
void shell();
int show_floor();
void show_level(bool things);
void show_win(char *message);
void sight();
int sign(int n);
int spread (int x);
void start_score();
void status();
int step_ok(char floorch);
void stomach(int junk);
void strucpy(register char *to, char *from, int max);
void sumprobs(register struct info *inf, int size);
void swander(int junk);
int swing(int level,int protection,int strength);
int take_off();
void teleport();
void total_winner();
void treas_room();
char trip_ch(int y, int x, char ch);
int turn_ok(int y, int x);
int turn_see(bool off);
void turnref();
void unconfuse();
void unlock_sc();
void unsee();
void visuals();
void wait_for(char c);
void wanderer();
void waste_time();
void wear();
void whatis(bool check, int item);
void wield();
void wr_score(struct scorefile *buf, FILE *f);

struct coords	*find_dest();

struct room	*roomin();

struct item	*get_item(), *new_item(), *new_thing(), *find_obj();

#ifdef CHECKTIME
int	checkout();
#endif


