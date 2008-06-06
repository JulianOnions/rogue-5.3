/*
 *	ROGUE
 *
 *	@(#)item.c	1.1	Strathclyde	10/31/84
 */

# include "param.h"
# include "types.h"
# include "extern.h"

void _detach(register struct item **list, struct item  *thing)
{
	if(*list == thing) *list = thing->i_link;
	if(thing->i_back) thing->i_back->i_link = thing->i_link;
	if(thing->i_link) thing->i_link->i_back = thing->i_back;
	thing->i_link = 0;
	thing->i_back = 0;
}

void _attach(register struct item **list,struct item  *thing)
{
	if(*list) {
		thing->i_link = *list;
		(*list)->i_back = thing;
	} else {
		thing->i_link = 0;
	}
	thing->i_back = 0;
	*list = thing;
}

void _free_list(register struct item **list)
{
	register struct item *thing;
	while(*list) {
		thing = *list;
		*list = thing->i_link;
		discard(thing);
	}
}

void discard_monster(struct being *thing)
{
}

void discard_item(struct item *thing)
{
    //   discard(thing);
}

void discard(register struct item *thing)
{
	free(thing);
}

struct item *new_item()
{
	register struct item *thing;
	thing = (struct item *)(calloc(1,CLICK));
	thing->i_link = 0;
	thing->i_back = 0;
	return(thing);
}

