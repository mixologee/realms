/**************************************************************************
 * #   #   #   ##   #  #  ###   ##   ##  ###       http://www.lyonesse.it *
 * #    # #   #  #  ## #  #    #    #    #                                *
 * #     #    #  #  # ##  ##    #    #   ##   ## ##  #  #  ##             *
 * #     #    #  #  # ##  #      #    #  #    # # #  #  #  # #            *
 * ###   #     ##   #  #  ###  ##   ##   ###  #   #  ####  ##    Ver. 1.0 *
 *                                                                        *
 * -Based on CircleMud & Smaug-     Copyright (c) 2001-2002 by Mithrandir *
 *                                                                        *
 * ********************************************************************** */
/* ************************************************************************
*  File: events.c                                                         *
*                                                                         *
*  Usage: Contains routines to handle events                              *
*                                                                         *
*  Written by Eric Green (ejg3@cornell.edu)                               *
*                                                                         *
*  Changes:                                                               *
*      3/6/98 ejg:  Changed event_process to check return value on the    *
*                   event function.  If > 0, reenqueue to expire in       *
*                   retval time.                                          *
*                   Added check in event_cancel to make sure event_obj    *
*                   is non-NULL.                                          *
*                   Moved struct event definition from events.h.          *
************************************************************************ */

#include "conf.h"
#include "sysdep.h"


#include "structs.h"
#include "utils.h"

/* external variables */
extern unsigned long pulse;

/* external functions */
QUEUE_DATA *queue_init( void );
Q_ELEM_DATA *queue_enq( QUEUE_DATA *q, void *data, long key );
long queue_key( QUEUE_DATA *q );
long queue_elmt_key( Q_ELEM_DATA *qe );
void queue_deq( QUEUE_DATA *q, Q_ELEM_DATA *qe );
void *queue_head( QUEUE_DATA *q );
void queue_free( QUEUE_DATA *q );

/* globals */
QUEUE_DATA *event_q;          /* the event queue */


/* initializes the event queue */
void event_init(void)
{
	event_q = queue_init();
}

/* creates an event and returns it */
EVENT_DATA *event_create(EVENTFUNC(*func), void *event_obj, long when)
{
	EVENT_DATA *new_event;
	
	if (when < 1) /* make sure its in the future */
		when = 1;
	
	CREATE(new_event, EVENT_DATA, 1);
	new_event->func = func;
	new_event->event_obj = event_obj;
	new_event->q_el = queue_enq(event_q, new_event, when + pulse);
	
	return (new_event);
}


/* removes the event from the system */
void event_cancel(EVENT_DATA *event)
{
	if (!event)
	{
		log("SYSERR:  Attempted to cancle a NULL event");
		return;
	}
	
	queue_deq(event_q, event->q_el);
	
	if (event->event_obj)
		free(event->event_obj);
	free(event);
}


/* Process any events whose time has come. */
void event_process(void)
{
	EVENT_DATA *the_event;
	long new_time;
	
	while ((long) pulse >= queue_key(event_q))
	{
		if (!(the_event = (EVENT_DATA *) queue_head(event_q)))
		{
			log("SYSERR: Attempt to get a NULL event");
			return;
		}
		
		/* call event func, reenqueue event if retval > 0 */
		if ((new_time = (the_event->func)(the_event->event_obj)) > 0)
			the_event->q_el = queue_enq(event_q, the_event, new_time + pulse);
		else
			free(the_event);
	}
}


/* returns the time remaining before the event */
long event_time(EVENT_DATA *event)
{
	long when;
	
	when = queue_elmt_key(event->q_el);
	
	return (when - pulse);
}


/* frees all events in the queue */
void event_free_all(void)
{
	EVENT_DATA *the_event;
	
	while ((the_event = (EVENT_DATA *) queue_head(event_q)))
	{
		if (the_event->event_obj)
			free(the_event->event_obj);
		free(the_event);
	}
	
	queue_free(event_q);
}

