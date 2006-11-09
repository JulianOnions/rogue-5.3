/*
 *	ROGUE
 *
 *	@(#)types.h	1.1	Strathclyde	10/31/84
 */

typedef void (*DFUNC)(int param);
struct coords {
	int	x;
	int	y;
};

struct room {
	int		r_xstart;
	int		r_ystart;
	int		r_xsize;
	int		r_ysize;
	struct coords	r_gold;
	int		r_value;
	short		r_flags;
	int		r_dct;
	struct coords	r_doors[12];
};

struct stats {
	int		strength;
	int		experience;
	int		level;
	int		protection;
	short		hp;
	char		*power;
	short		maxhp;
};

struct being {
	struct being	*b_link;
	struct being	*b_back;
	struct coords	b_coords;
	bool		b_moved;
	char		b_ch;
	char		b_app;
	char		b_oldch;
	struct coords	*b_dest;
	short		b_flags;
	struct stats	b_stats;
	struct room	*b_room;
	struct item	*b_pack;
};

struct place {
	char		p_ch;
	unsigned char	p_flags;
	struct being	*p_being;
};

struct item {
	struct item	*i_link;
	struct item	*i_back;
	int		i_item;
	struct coords	i_coords;
	int		i_l14;
	char		i_uses;
	char		i_packchar;
	char		*i_hpower;
	char		*i_tpower;
	int		i_number;
	int		i_type;
	int		i_hitplus;
	int		i_damplus;
	short		i_value;
	short		i_flags;
	int		i_group;
	char		*i_called;
};

struct info {
	char	*inf_name;
	int	inf_prob;
	short	inf_value;
	char	*inf_called;
	bool	inf_known;
};

struct monster {
	char	*m_name;
	int	m_take;
	short	m_flags;
	int	m_strength;
	int	m_experience;
	int	m_level;
	int	m_protection;
	int	m_l1c;
	char	*m_power;
	int	m_l24;
};

struct option {
	char	*o_name;
	char	*o_desc;
	char	*o_arg;
	void	(*o_put)(void *str);
	int	(*o_get)(void *str, WINDOW *win);
};

struct scorefile {
	int	s_uid;
	int	s_score;
	int	s_endtype;
	short	s_killed;
	char	s_name[MAXSTR];
	short	s_level;
	int	s_days;
};

