#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "screen.h"
#include "conf.h"
#include "sysdep.h"
#include "structs.h"
#include "comm.h"
#include "interpreter.h"
#include "spells.h"
#include "utils.h"
#include "db.h"
#include "boards.h"
#include "constants.h"
#include "shop.h"
#include "genolc.h"
#include "genobj.h"
#include "genzon.h"
#include "oasis.h"
#include "improved-edit.h"
#include "dg_olc.h"
#include "screen.h"
#include "handler.h"
#include "house.h"
#include "warzone.h"
#include "clan.h"
#include "arena.h"

#include "mapper.h"


/* The map itself */
struct map_type map[ASCIIMAPX + 1][ASCIIMAPY + 1];

/* Take care of some repetitive code for later */
void get_exit_dir( int dir, int *x, int *y, int xorig, int yorig )
{
	/* Get the next coord based on direction */
	switch( dir )
	{
		case 0: /* North */
		*x = xorig;
		*y = yorig - 1;
		break;
		case 1: /* East */
		*x = xorig + 1;
		*y = yorig;
		break;
		case 2: /* South */
		*x = xorig;
		*y = yorig + 1;
		break;
		case 3: /* West */
		*x = xorig - 1;
		*y = yorig;
		break;
		case 4: /* UP */
		break;
		case 5: /* DOWN */
		break;
		case 6: /* NE */
		*x = xorig + 1;
		*y = yorig - 1;
		break;
		case 7: /* NW */
		*x = xorig - 1;
		*y = yorig - 1;
		break;
		case 8: /* SE */
		*x = xorig + 1;
		*y = yorig + 1;
		break;
		case 9: /* SW */
		*x = xorig - 1;
		*y = yorig + 1;
		break;
		default:
		*x = -1;
		*y = -1;
		break;
	}
}

char *get_exits(struct char_data *ch)
{
	static char buf[MSL];
	EXIT_DATA *pexit;
	bool found;

	buf[0] = '\0';

        if (AFF_FLAGGED(i, AFF_BLIND))
	return buf;

	if( MXP_ON( ch ) )
	send_to_char( MXP_TAG_ROOMEXIT, ch );

	set_char_color( AT_EXITS, ch );

	mudstrlcpy( buf, "[Exits:", MSL );

	found = FALSE;
	for( pexit = ch->in_room->first_exit; pexit; pexit = pexit->next )
	{
		if( IS_IMMORTAL(ch) )
		/* Immortals see all exits, even secret ones */
		{
			if( pexit->to_room )
			{
				found = TRUE;
				mudstrlcat( buf, " ", MSL );

				if( MXP_ON( ch ) )
				mudstrlcat( buf, "<Ex>", MSL );

				mudstrlcat( buf, capitalize( dir_name[pexit->vdir] ), MSL );

				if( MXP_ON( ch ) )
				mudstrlcat( buf, "</Ex>", MSL );

				if( IS_EXIT_FLAG(pexit, EX_OVERLAND) )
				mudstrlcat( buf, "->(Overland)", MSL );


				/* New code added to display closed, or otherwise invisible exits to immortals */
				/* Installed by Samson 1-25-98 */
				if( IS_EXIT_FLAG( pexit, EX_CLOSED ) )
				mudstrlcat( buf, "->(Closed)", MSL );
				if( IS_EXIT_FLAG( pexit, EX_WINDOW ) )
				mudstrlcat( buf, "->(Window)", MSL );
				if( IS_EXIT_FLAG( pexit, EX_HIDDEN ) )
				mudstrlcat( buf, "->(Hidden)", MSL );

			}
		}
		else
		{
			if( pexit->to_room
			&& !IS_EXIT_FLAG( pexit, EX_SECRET )
			&& ( !IS_EXIT_FLAG( pexit, EX_WINDOW ) || IS_EXIT_FLAG( pexit, EX_ISDOOR ) )
			&& !IS_EXIT_FLAG( pexit, EX_HIDDEN )
			&& !IS_EXIT_FLAG( pexit, EX_FORTIFIED ) /* Checks for walls, Marcus */
			&& !IS_EXIT_FLAG( pexit, EX_HEAVY )
			&& !IS_EXIT_FLAG( pexit, EX_MEDIUM )
			&& !IS_EXIT_FLAG( pexit, EX_LIGHT )
			&& !IS_EXIT_FLAG( pexit, EX_CRUMBLING ) )
			{
				found = TRUE;
				mudstrlcat( buf, " ", MSL );

				if( MXP_ON( ch ) )
				mudstrlcat( buf, "<Ex>", MSL );

				mudstrlcat( buf, capitalize( dir_name[pexit->vdir] ), MSL );

				if( MXP_ON( ch ) )
				mudstrlcat( buf, "</Ex>", MSL );

				if( IS_EXIT_FLAG( pexit, EX_CLOSED ) )
				mudstrlcat( buf, "->(Closed)", MSL );
			}
		}
	}

	if( !found )
	{
		mudstrlcat( buf, " none]", MSL );
		if( MXP_ON( ch ) )
		mudstrlcat( buf, MXP_TAG_ROOMEXIT_CLOSE, MSL );
	}
	else
	{
		mudstrlcat( buf, "]", MSL );
		if( MXP_ON( ch ) )
		mudstrlcat( buf, MXP_TAG_ROOMEXIT_CLOSE, MSL );
	}

	mudstrlcat( buf, "\n\r", MSL );
	return buf;
}


/* Clear one map coord */
void clear_coord( int x, int y )
{
	map[x][y].tegn = ' ';
	map[x][y].vnum = 0;
	map[x][y].depth = 0;
	xCLEAR_BITS( map[x][y].info );
	map[x][y].can_see = TRUE;
}

/* Clear all exits for one room */
void clear_room( int x, int y )
{
	int dir, exitx, exity;

	/* Cycle through the four directions */
	for( dir = 0; dir < 4; dir++ )
	{
		/* Find next coord in this direction */
		get_exit_dir( dir, &exitx, &exity, x, y );

		/* If coord is valid, clear it */
		if ( !BOUNDARY( exitx, exity ) ) clear_coord( exitx, exity );
	}
}

/* This function is recursive, ie it calls itself */
void map_exits(CHAR_DATA *ch, ROOM_INDEX_DATA *pRoom, int x, int y, int depth)
{
	static char map_chars [11] = "|-|-UD/\\\\/";
	int door;
	int exitx = 0, exity = 0;
	int roomx = 0, roomy = 0;
	char buf[200]; // bugs
	EXIT_DATA *pExit;

	/* Setup this coord as a room */
	switch(pRoom->sector_type)
	{
		case SECT_CITY:
		case SECT_INDOORS:
		map[x][y].tegn = 'O';
		break;
		case SECT_FIELD:
		case SECT_FOREST:
		case SECT_HILLS:
		map[x][y].tegn = '*';
		break;
		case SECT_MOUNTAIN:
		map[x][y].tegn = '@';
		break;
		case SECT_WATER_SWIM:
		case SECT_WATER_NOSWIM:
		map[x][y].tegn = '=';
		break;
		case SECT_AIR:
		map[x][y].tegn = '~';
		break;
		case SECT_DESERT:
		map[x][y].tegn = '+';
		break;
		default:
		map[x][y].tegn = 'O';
		sprintf(buf, "Map_exits: Bad sector type (%d) in room %d.",
		pRoom->sector_type, pRoom->vnum);
		bug(buf, 0);
		break;
	}
	map[x][y].vnum = pRoom->vnum;
	map[x][y].depth = depth;
	map[x][y].info = pRoom->room_flags;
	map[x][y].can_see = room_is_dark( pRoom, ch );

	/* Limit recursion */
	if ( depth > MAXDEPTH ) return;

	/* This room is done, deal with it's exits */
	for( door = 0; door < 10; door++ )
	{
		/* Skip if there is no exit in this direction */
		if ( ( pExit = get_exit( pRoom, door ) ) == NULL )
		continue;

		/* Skip up and down until I can figure out a good way to display it */
		if (door == 4 || door == 5)
		continue;

		/* Get the coords for the next exit and room in this direction */
		get_exit_dir( door, &exitx, &exity, x, y );
		get_exit_dir( door, &roomx, &roomy, exitx, exity );

		/* Skip if coords fall outside map */
		if ( BOUNDARY( exitx, exity ) || BOUNDARY( roomx, roomy )) continue;

		/* Skip if there is no room beyond this exit */
		if ( pExit->to_room == NULL ) continue;

		/* Ensure there are no clashes with previously defined rooms */
		if ( ( map[roomx][roomy].vnum != 0 ) &&
		( map[roomx][roomy].vnum != pExit->to_room->vnum ))
		{
			/* Use the new room if the depth is higher */
			if ( map[roomx][roomy].depth <= depth ) continue;

			/* It is so clear the old room */
			clear_room( roomx, roomy );
		}

		/* No exits at MAXDEPTH */
		if ( depth == MAXDEPTH ) continue;

		/* No need for exits that are already mapped */
		if ( map[exitx][exity].depth > 0 ) continue;

		/* Fill in exit */
		map[exitx][exity].depth = depth;
		map[exitx][exity].vnum = pExit->to_room->vnum;
		map[exitx][exity].info = pExit->exit_info;
		map[exitx][exity].tegn = map_chars[door];

		/* More to do? If so we recurse */
		if ( ( depth < MAXDEPTH ) &&
		( ( map[roomx][roomy].vnum == pExit->to_room->vnum ) ||
		( map[roomx][roomy].vnum == 0 ) ) )
		{
			/* Depth increases by one each time */
			map_exits( ch, pExit->to_room, roomx, roomy, depth + 1 );
		}
	}
}

/* Reformat room descriptions to exclude undesirable characters */
void reformat_desc( char *desc )
{
	/* Index variables to keep track of array/pointer elements */
	unsigned int i;
	int j;
	char buf[MAX_STRING_LENGTH], *p;

	i = 0;
	j = 0;
	buf[0] = '\0';

	if ( !desc  ) return;

	/* Replace all "\n" and "\r" with spaces */
	for( i = 0; i <= strlen( desc ); i++ )
	{
		if ( ( desc[i] == '\n' ) || ( desc[i] == '\r' ) ) desc[i] = ' ';
	}

	/* Remove multiple spaces */
	for( p = desc; *p != '\0'; p++ )
	{
		buf[j] = *p;
		j++;

		/* Two or more consecutive spaces? */
		if ( ( *p == ' ' ) && ( *( p + 1 ) == ' ' ) )
		{
			do
			{
				p++;
			}
			while( *(p + 1) == ' ' );
		}
	}

	buf[j] = '\0';

	/* Copy to desc */
	mudstrlcpy( desc, buf, MIL );
}

int get_line( char *desc, int max_len )
{
	int i, j = 0;

	/* Return if it's short enough for one line */
	if ( (int)strlen( desc ) <= max_len ) return 0;

	/* Calculate end point in string without color */
	for( i = 0; i <= (int)strlen( desc ); i++ )
	{
		/* Here you need to skip your color sequences */
		j++;

		if ( j > max_len ) break;
	}

	/* End point is now in i, find the nearest space */
	for( j = i; j > 0; j-- )
	{
		if ( desc[j] == ' ' ) break;
	}

	/* There could be a problem if there are no spaces on the line */
	return j + 1;
}

/* Display the map to the player */
void show_map( CHAR_DATA *ch, char *text )
{
	char buf[MAX_STRING_LENGTH * 2];
	int x, y, pos;
	char *p;
	bool alldesc = FALSE; /* Has desc been fully displayed? */

	if ( !text ) alldesc = TRUE;

	pos = 0;
	p = text;
	buf[0] = '\0';

  //if (y == 0 && IS_PLR_FLAG( ch, PLR_AUTOEXIT))  /* the autoexits */
  //{
  //  sprintf(buf + strlen( buf ), "%s", get_exits(ch));
  //  continue;
  //}


	/* Top of map frame */
	if (IS_PLR_FLAG( ch, PLR_AUTOEXIT )) /* Show exits */
    	sprintf( buf, "&z+-----------+&w %s%s", color_str( AT_EXITS, ch ), get_exits(ch) );
	else
 		sprintf( buf, "&z+-----------+&w\n\r" );   

	/* Write out the main map area with text */
	for( y = 0; y <= ASCIIMAPY; y++ )
	{
		mudstrlcat( buf, "&z|&D", MSL );

		for( x = 0; x <= ASCIIMAPX; x++ )
		{
			switch(map[x][y].tegn)
			{
				case '-':
				case '|':
				case '\\':
				case '/':
					sprintf(buf + strlen(buf), "&O%c&D", map[x][y].tegn);
					break;
				case 'X':
					sprintf(buf + strlen(buf), "&R%c&d", map[x][y].tegn);
					break;
				case '*':
					sprintf(buf + strlen(buf), "&g%c&d", map[x][y].tegn);
					break;
				case '@':
					sprintf(buf + strlen(buf), "&r%c&d", map[x][y].tegn);
					break;
				case '=':
					sprintf(buf + strlen(buf), "&B%c&d", map[x][y].tegn);
					break;
				case '~':
					sprintf(buf + strlen(buf), "&C%c&d", map[x][y].tegn);
					break;
				case '+':
					sprintf(buf + strlen(buf), "&Y%c&d", map[x][y].tegn);
					break;
				case 'O':
					sprintf(buf + strlen(buf), "&w%c&d", map[x][y].tegn);
					break;
				default:
					sprintf( buf + strlen( buf ), "%c", map[x][y].tegn );
				}
			}
			mudstrlcat( buf, "&z|&D ", MSL );

			//if (y == 0 && IS_PLR_FLAG( ch, PLR_AUTOEXIT))  /* the autoexits */
			//{
			//	sprintf(buf + strlen( buf ), "%s", get_exits(ch));
			//	continue;
			//}

			/* Add the text, if necessary */
			if ( !alldesc )
			{
				pos = get_line( p, 63 );
				if ( pos > 0 )
				{
					mudstrlcat( buf, color_str(AT_RMDESC, ch), MSL);
					strncat( buf, p, pos );
					p += pos;
				}
				else
				{
					mudstrlcat( buf, color_str(AT_RMDESC, ch), MSL);
					mudstrlcat( buf, p, MSL );
					alldesc = TRUE;
				}
			}
			mudstrlcat( buf, "\n\r", MSL );
			}

			/* Finish off map area */
			mudstrlcat( buf, "&z+-----------+&D ", MSL );
			if ( !alldesc )
			{
				pos = get_line( p, 63 );
				if ( pos > 0 )
				{
					mudstrlcat( buf, color_str(AT_RMDESC, ch), MSL);
					strncat( buf, p, pos );
					p += pos;
				}
				else
				{
					mudstrlcat( buf, color_str(AT_RMDESC, ch), MSL);
					mudstrlcat( buf, p, MSL );
					alldesc = TRUE;
				}
			}

			/* Deal with any leftover text */
			if ( !alldesc )
			{
				do
				{
					/* Note the number - no map to detract from width */
					pos = get_line( p, 78 );
					if ( pos > 0 )
					{
						mudstrlcat( buf, color_str(AT_RMDESC, ch), MSL);
						strncat( buf, p, pos );
						p += pos;
					}
					else
					{
						mudstrlcat( buf, color_str(AT_RMDESC, ch), MSL);
						mudstrlcat( buf, p, MSL );
						alldesc = TRUE;
					}
				}
				while( !alldesc );
			}
			mudstrlcat(buf, "&D\n\r", MSL);
			send_to_char(buf, ch);
		}

/* Clear, generate and display the map */
void draw_map( CHAR_DATA *ch, const char *desc )
{
	int x, y;
	static char buf[MAX_STRING_LENGTH];

	mudstrlcpy( buf, desc, MIL );
	/* Remove undesirable characters */
	reformat_desc( buf );

	/* Clear map */
	for( y = 0; y <= ASCIIMAPY; y++ )
	{
		for( x = 0; x <= ASCIIMAPX; x++ )
		{
			clear_coord( x, y );
		}
	}

	/* Start with players pos at centre of map */
	x = ASCIIMAPX / 2;
	y = ASCIIMAPY / 2;

	map[x][y].vnum = ch->in_room->vnum;
	map[x][y].depth = 0;

	/* Generate the map */
	map_exits( ch, ch->in_room, x, y, 0 );

	/* Current position should be a "X" */
	map[x][y].tegn = 'X';
	/* Send the map */
	show_map( ch, buf );
}

