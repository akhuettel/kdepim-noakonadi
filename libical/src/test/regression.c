/* -*- Mode: C -*-
  ======================================================================
  FILE: regression.c
  CREATOR: eric 03 April 1999
  
  DESCRIPTION:
  
  $Id$
  $Locker:  $

  (C) COPYRIGHT 1999 Eric Busboom 
  http://www.softwarestudio.org

  The contents of this file are subject to the Mozilla Public License
  Version 1.0 (the "License"); you may not use this file except in
  compliance with the License. You may obtain a copy of the License at
  http://www.mozilla.org/MPL/
 
  Software distributed under the License is distributed on an "AS IS"
  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
  the License for the specific language governing rights and
  limitations under the License.

  The original author is Eric Busboom
  The original code is regression.c

    
  ======================================================================*/

#include "ical.h"
#include "icalss.h"

#include <assert.h>
#include <string.h> /* for strdup */
#include <stdlib.h> /* for malloc */
#include <stdio.h> /* for printf */
#include <time.h> /* for time() */


/* This example creates and minipulates the ical object that appears
 * in rfc 2445, page 137 */

char str[] = "BEGIN:VCALENDAR\
PRODID:\"-//RDU Software//NONSGML HandCal//EN\"\
VERSION:2.0\
BEGIN:VTIMEZONE\
TZID:US-Eastern\
BEGIN:STANDARD\
DTSTART:19981025T020000\
RDATE:19981025T020000\
TZOFFSETFROM:-0400\
TZOFFSETTO:-0500\
TZNAME:EST\
END:STANDARD\
BEGIN:DAYLIGHT\
DTSTART:19990404T020000\
RDATE:19990404T020000\
TZOFFSETFROM:-0500\
TZOFFSETTO:-0400\
TZNAME:EDT\
END:DAYLIGHT\
END:VTIMEZONE\
BEGIN:VEVENT\
DTSTAMP:19980309T231000Z\
UID:guid-1.host1.com\
ORGANIZER;ROLE=CHAIR:MAILTO:mrbig@host.com\
ATTENDEE;RSVP=TRUE;ROLE=REQ-PARTICIPANT;CUTYPE=GROUP:MAILTO:employee-A@host.com\
DESCRIPTION:Project XYZ Review Meeting\
CATEGORIES:MEETING\
CLASS:PUBLIC\
CREATED:19980309T130000Z\
SUMMARY:XYZ Project Review\
DTSTART;TZID=US-Eastern:19980312T083000\
DTEND;TZID=US-Eastern:19980312T093000\
LOCATION:1CP Conference Room 4350\
END:VEVENT\
BEGIN:BOOGA\
DTSTAMP:19980309T231000Z\
X-LIC-FOO:Booga\
DTSTOMP:19980309T231000Z\
UID:guid-1.host1.com\
END:BOOGA\
END:VCALENDAR";


icalcomponent* create_simple_component()
{

    icalcomponent* calendar;
    struct icalperiodtype rtime;

    rtime.start = icaltime_from_timet( time(0),0);
    rtime.end = icaltime_from_timet( time(0),0);

    rtime.end.hour++;



    /* Create calendar and add properties */
    calendar = icalcomponent_new(ICAL_VCALENDAR_COMPONENT);

    
    icalcomponent_add_property(
	calendar,
	icalproperty_new_version("2.0")
	);

    printf("%s\n",icalcomponent_as_ical_string(calendar));
 
    return calendar;
	   
}

/* Create a new component */
icalcomponent* create_new_component()
{

    icalcomponent* calendar;
    icalcomponent* timezone;
    icalcomponent* tzc;
    icalcomponent* event;
    struct icaltimetype atime = icaltime_from_timet( time(0),0);
    struct icalperiodtype rtime;
    icalproperty* property;

    rtime.start = icaltime_from_timet( time(0),0);
    rtime.end = icaltime_from_timet( time(0),0);

    rtime.end.hour++;



    /* Create calendar and add properties */
    calendar = icalcomponent_new(ICAL_VCALENDAR_COMPONENT);

    
    icalcomponent_add_property(
	calendar,
	icalproperty_new_version("2.0")
	);
    
    icalcomponent_add_property(
	calendar,
	icalproperty_new_prodid("-//RDU Software//NONSGML HandCal//EN")
	);
    
    /* Create a timezone object and add it to the calendar */

    timezone = icalcomponent_new(ICAL_VTIMEZONE_COMPONENT);

    icalcomponent_add_property(
	timezone,
	icalproperty_new_tzid("US_Eastern")
	);

    /* Add a sub-component of the timezone */
    tzc = icalcomponent_new(ICAL_XDAYLIGHT_COMPONENT);

    icalcomponent_add_property(
	tzc, 
	icalproperty_new_dtstart(atime)
	);

    icalcomponent_add_property(
	tzc, 
	icalproperty_new_rdate(rtime)
	);
	    
    icalcomponent_add_property(
	tzc, 
	icalproperty_new_tzoffsetfrom(-4.0)
	);

    icalcomponent_add_property(
	tzc, 
	icalproperty_new_tzoffsetto(-5.0)
	);

    icalcomponent_add_property(
	tzc, 
	icalproperty_new_tzname("EST")
	);

    icalcomponent_add_component(timezone,tzc);

    icalcomponent_add_component(calendar,timezone);

    /* Add a second subcomponent */
    tzc = icalcomponent_new(ICAL_XSTANDARD_COMPONENT);

    icalcomponent_add_property(
	tzc, 
	icalproperty_new_dtstart(atime)
	);

    icalcomponent_add_property(
	tzc, 
	icalproperty_new_rdate(rtime)
	);
	    
    icalcomponent_add_property(
	tzc, 
	icalproperty_new_tzoffsetfrom(-4.0)
	);

    icalcomponent_add_property(
	tzc, 
	icalproperty_new_tzoffsetto(-5.0)
	);

    icalcomponent_add_property(
	tzc, 
	icalproperty_new_tzname("EST")
	);

    icalcomponent_add_component(timezone,tzc);

    /* Add an event */

    event = icalcomponent_new(ICAL_VEVENT_COMPONENT);

    icalcomponent_add_property(
	event,
	icalproperty_new_dtstamp(atime)
	);

    icalcomponent_add_property(
	event,
	icalproperty_new_uid("guid-1.host1.com")
	);

    /* add a property that has parameters */
    property = icalproperty_new_organizer("mrbig@host.com");
    
    icalproperty_add_parameter(
	property,
	icalparameter_new_role(ICAL_ROLE_CHAIR)
	);

    icalcomponent_add_property(event,property);

    /* add another property that has parameters */
    property = icalproperty_new_attendee("employee-A@host.com");
    
    icalproperty_add_parameter(
	property,
	icalparameter_new_role(ICAL_ROLE_REQPARTICIPANT)
	);

    icalproperty_add_parameter(
	property,
	icalparameter_new_rsvp(1)
	);

    icalproperty_add_parameter(
	property,
	icalparameter_new_cutype(ICAL_CUTYPE_GROUP)
	);

    icalcomponent_add_property(event,property);


    /* more properties */

    icalcomponent_add_property(
	event,
	icalproperty_new_description("Project XYZ Review Meeting")
	);

    icalcomponent_add_property(
	event,
	icalproperty_new_categories("MEETING")
	);

    icalcomponent_add_property(
	event,
	icalproperty_new_class("PUBLIC")
	);
    
    icalcomponent_add_property(
	event,
	icalproperty_new_created(atime)
	);

    icalcomponent_add_property(
	event,
	icalproperty_new_summary("XYZ Project Review")
	);


    property = icalproperty_new_dtstart(atime);
    
    icalproperty_add_parameter(
	property,
	icalparameter_new_tzid("US-Eastern")
	);

    icalcomponent_add_property(event,property);


    property = icalproperty_new_dtend(atime);
    
    icalproperty_add_parameter(
	property,
	icalparameter_new_tzid("US-Eastern")
	);

    icalcomponent_add_property(event,property);

    icalcomponent_add_property(
	event,
	icalproperty_new_location("1CP Conference Room 4350")
	);

    icalcomponent_add_component(calendar,event);

    printf("%s\n",icalcomponent_as_ical_string(calendar));

    icalcomponent_free(calendar);

    return 0;
}


/* Create a new component, using the va_args list */

icalcomponent* create_new_component_with_va_args()
{

    icalcomponent* calendar;
    struct icaltimetype atime = icaltime_from_timet( time(0),0);
    struct icalperiodtype rtime;
    
    rtime.start = icaltime_from_timet( time(0),0);
    rtime.end = icaltime_from_timet( time(0),0);

    rtime.end.hour++;

    calendar = 
	icalcomponent_vanew(
	    ICAL_VCALENDAR_COMPONENT,
	    icalproperty_new_version("2.0"),
	    icalproperty_new_prodid("-//RDU Software//NONSGML HandCal//EN"),
	    icalcomponent_vanew(
		ICAL_VTIMEZONE_COMPONENT,
		icalproperty_new_tzid("US_Eastern"),
		icalcomponent_vanew(
		    ICAL_XDAYLIGHT_COMPONENT,
		    icalproperty_new_dtstart(atime),
		    icalproperty_new_rdate(rtime),
		    icalproperty_new_tzoffsetfrom(-4.0),
		    icalproperty_new_tzoffsetto(-5.0),
		    icalproperty_new_tzname("EST"),
		    0
		    ),
		icalcomponent_vanew(
		    ICAL_XSTANDARD_COMPONENT,
		    icalproperty_new_dtstart(atime),
		    icalproperty_new_rdate(rtime),
		    icalproperty_new_tzoffsetfrom(-5.0),
		    icalproperty_new_tzoffsetto(-4.0),
		    icalproperty_new_tzname("EST"),
		    0
		    ),
		0
		),
	    icalcomponent_vanew(
		ICAL_VEVENT_COMPONENT,
		icalproperty_new_dtstamp(atime),
		icalproperty_new_uid("guid-1.host1.com"),
		icalproperty_vanew_organizer(
		    "mrbig@host.com",
		    icalparameter_new_role(ICAL_ROLE_CHAIR),
		    0
		    ),
		icalproperty_vanew_attendee(
		    "employee-A@host.com",
		    icalparameter_new_role(ICAL_ROLE_REQPARTICIPANT),
		    icalparameter_new_rsvp(1),
		    icalparameter_new_cutype(ICAL_CUTYPE_GROUP),
		    0
		    ),
		icalproperty_new_description("Project XYZ Review Meeting"),
		icalproperty_new_categories("MEETING"),
		icalproperty_new_class("PUBLIC"),
		icalproperty_new_created(atime),
		icalproperty_new_summary("XYZ Project Review"),
		icalproperty_vanew_dtstart(
		    atime,
		    icalparameter_new_tzid("US-Eastern"),
		    0
		    ),
		icalproperty_vanew_dtend(
		    atime,
		    icalparameter_new_tzid("US-Eastern"),
		    0
		    ),
		icalproperty_new_location("1CP Conference Room 4350"),
		0
		),
	    0
	    );
	
    printf("%s\n",icalcomponent_as_ical_string(calendar));
    

    icalcomponent_free(calendar);

    return 0;
}


/* Return a list of all attendees who are required. */
   
char** get_required_attendees(icalproperty* event)
{
    icalproperty* p;
    icalparameter* parameter;

    char **attendees;
    int max = 10;
    int c = 0;

    attendees = malloc(max * (sizeof (char *)));

    assert(event != 0);
    assert(icalcomponent_isa(event) == ICAL_VEVENT_COMPONENT);
    
    for(
	p = icalcomponent_get_first_property(event,ICAL_ATTENDEE_PROPERTY);
	p != 0;
	p = icalcomponent_get_next_property(event,ICAL_ATTENDEE_PROPERTY)
	) {
	
	parameter = icalproperty_get_first_parameter(p,ICAL_ROLE_PARAMETER);

	if ( icalparameter_get_role(parameter) == ICAL_ROLE_REQPARTICIPANT) 
	{
	    attendees[c++] = strdup(icalproperty_get_attendee(p));

            if (c >= max) {
                max *= 2; 
                attendees = realloc(attendees, max * (sizeof (char *)));
            }

	}
    }

    return attendees;
}

/* If an attendee has a PARTSTAT of NEEDSACTION or has no PARTSTAT
   parameter, change it to TENTATIVE. */
   
void update_attendees(icalproperty* event)
{
    icalproperty* p;
    icalparameter* parameter;


    assert(event != 0);
    assert(icalcomponent_isa(event) == ICAL_VEVENT_COMPONENT);
    
    for(
	p = icalcomponent_get_first_property(event,ICAL_ATTENDEE_PROPERTY);
	p != 0;
	p = icalcomponent_get_next_property(event,ICAL_ATTENDEE_PROPERTY)
	) {
	
	parameter = icalproperty_get_first_parameter(p,ICAL_PARTSTAT_PARAMETER);

	if (parameter == 0) {

	    icalproperty_add_parameter(
		p,
		icalparameter_new_partstat(ICAL_PARTSTAT_TENTATIVE)
		);

	} else if (icalparameter_get_partstat(parameter) == ICAL_PARTSTAT_NEEDSACTION) {

	    icalproperty_remove_parameter(p,ICAL_PARTSTAT_PARAMETER);
	    
	    icalparameter_free(parameter);

	    icalproperty_add_parameter(
		p,
		icalparameter_new_partstat(ICAL_PARTSTAT_TENTATIVE)
		);
	}

    }
}


void test_values()
{
    icalvalue *v; 
    icalvalue *copy; 

    v = icalvalue_new_caladdress("cap://value/1");
    printf("caladdress 1: %s\n",icalvalue_get_caladdress(v));
    icalvalue_set_caladdress(v,"cap://value/2");
    printf("caladdress 2: %s\n",icalvalue_get_caladdress(v));
    printf("String: %s\n",icalvalue_as_ical_string(v));
    
    copy = icalvalue_new_clone(v);
    printf("Clone: %s\n",icalvalue_as_ical_string(v));
    icalvalue_free(v);
    icalvalue_free(copy);


    v = icalvalue_new_boolean(1);
    printf("caladdress 1: %d\n",icalvalue_get_boolean(v));
    icalvalue_set_boolean(v,2);
    printf("caladdress 2: %d\n",icalvalue_get_boolean(v));
    printf("String: %s\n",icalvalue_as_ical_string(v));

    copy = icalvalue_new_clone(v);
    printf("Clone: %s\n",icalvalue_as_ical_string(v));
    icalvalue_free(v);
    icalvalue_free(copy);


    v = icalvalue_new_date(icaltime_from_timet( time(0),0));
    printf("date 1: %s\n",icalvalue_as_ical_string(v));
    icalvalue_set_date(v,icaltime_from_timet( time(0)+3600,0));
    printf("date 2: %s\n",icalvalue_as_ical_string(v));

    copy = icalvalue_new_clone(v);
    printf("Clone: %s\n",icalvalue_as_ical_string(v));
    icalvalue_free(v);
    icalvalue_free(copy);


    v = icalvalue_new(-1);

    printf("Invalid type: %p\n",v);

    if (v!=0) icalvalue_free(v);


    /*    v = icalvalue_new_caladdress(0);

    printf("Bad string: %p\n",v);

    if (v!=0) icalvalue_free(v); */

}

void test_properties()
{
    icalproperty *prop;
    icalparameter *param;

    icalproperty *clone;

    prop = icalproperty_vanew_comment(
	"Another Comment",
	icalparameter_new_cn("A Common Name 1"),
	icalparameter_new_cn("A Common Name 2"),
	icalparameter_new_cn("A Common Name 3"),
       	icalparameter_new_cn("A Common Name 4"),
	0); 

    for(param = icalproperty_get_first_parameter(prop,ICAL_ANY_PARAMETER);
	param != 0; 
	param = icalproperty_get_next_parameter(prop,ICAL_ANY_PARAMETER)) {
						
	printf("Prop parameter: %s\n",icalparameter_get_cn(param));
    }    

    printf("Prop value: %s\n",icalproperty_get_comment(prop));


    printf("As iCAL string:\n %s\n",icalproperty_as_ical_string(prop));
    
    clone = icalproperty_new_clone(prop);

    printf("Clone:\n %s\n",icalproperty_as_ical_string(prop));
    
    icalproperty_free(clone);
    icalproperty_free(prop);

    prop = icalproperty_new(-1);

    printf("Invalid type: %p\n",prop);

    if (prop!=0) icalproperty_free(prop);

    /*
    prop = icalproperty_new_method(0);

    printf("Bad string: %p\n",prop);
   

    if (prop!=0) icalproperty_free(prop);
    */
}

void test_parameters()
{
    icalparameter *p;

    p = icalparameter_new_cn("A Common Name");

    printf("Common Name: %s\n",icalparameter_get_cn(p));

    printf("As String: %s\n",icalparameter_as_ical_string(p));

    icalparameter_free(p);
}


void test_components()
{

    icalcomponent* c;
    icalcomponent* child;

    c = icalcomponent_vanew(
	ICAL_VCALENDAR_COMPONENT,
	icalproperty_new_version("2.0"),
	icalproperty_new_prodid("-//RDU Software//NONSGML HandCal//EN"),
	icalproperty_vanew_comment(
	    "A Comment",
	    icalparameter_new_cn("A Common Name 1"),
	    0),
	icalcomponent_vanew(
	    ICAL_VEVENT_COMPONENT,
	    icalproperty_new_version("2.0"),
	    icalproperty_new_description("This is an event"),
	    icalproperty_vanew_comment(
		"Another Comment",
		icalparameter_new_cn("A Common Name 1"),
		icalparameter_new_cn("A Common Name 2"),
		icalparameter_new_cn("A Common Name 3"),
		icalparameter_new_cn("A Common Name 4"),
		0),
	    icalproperty_vanew_xlicerror(
		"This is only a test",
		icalparameter_new_xlicerrortype(ICAL_XLICERRORTYPE_COMPONENTPARSEERROR),
		0),
	    
	    0
	    ),
	0
	);

    printf("Original Component:\n%s\n\n",icalcomponent_as_ical_string(c));

    child = icalcomponent_get_first_component(c,ICAL_VEVENT_COMPONENT);

    printf("Child Component:\n%s\n\n",icalcomponent_as_ical_string(child));
    
    icalcomponent_free(c);

}

void test_memory()
{
    size_t bufsize = 256;
    int i;
    char *p;

    char S1[] = "1) When in the Course of human events, ";
    char S2[] = "2) it becomes necessary for one people to dissolve the political bands which have connected them with another, ";
    char S3[] = "3) and to assume among the powers of the earth, ";
    char S4[] = "4) the separate and equal station to which the Laws of Nature and of Nature's God entitle them, ";
    char S5[] = "5) a decent respect to the opinions of mankind requires that they ";
    char S6[] = "6) should declare the causes which impel them to the separation. ";
    char S7[] = "7) We hold these truths to be self-evident, ";
    char S8[] = "8) that all men are created equal, ";

/*    char S9[] = "9) that they are endowed by their Creator with certain unalienable Rights, ";
    char S10[] = "10) that among these are Life, Liberty, and the pursuit of Happiness. ";
    char S11[] = "11) That to secure these rights, Governments are instituted among Men, ";
    char S12[] = "12) deriving their just powers from the consent of the governed. "; 
*/


    char *f, *b1, *b2, *b3, *b4, *b5, *b6, *b7, *b8;

#define BUFSIZE 1024

    f = icalmemory_new_buffer(bufsize);
    p = f;
    b1 = icalmemory_tmp_buffer(BUFSIZE);
    strcpy(b1, S1);
    icalmemory_append_string(&f, &p, &bufsize, b1);

    b2 = icalmemory_tmp_buffer(BUFSIZE);
    strcpy(b2, S2);
    icalmemory_append_string(&f, &p, &bufsize, b2);

    b3 = icalmemory_tmp_buffer(BUFSIZE);
    strcpy(b3, S3);
    icalmemory_append_string(&f, &p, &bufsize, b3);

    b4 = icalmemory_tmp_buffer(BUFSIZE);
    strcpy(b4, S4);
    icalmemory_append_string(&f, &p, &bufsize, b4);

    b5 = icalmemory_tmp_buffer(BUFSIZE);
    strcpy(b5, S5);
    icalmemory_append_string(&f, &p, &bufsize, b5);

    b6 = icalmemory_tmp_buffer(BUFSIZE);
    strcpy(b6, S6);
    icalmemory_append_string(&f, &p, &bufsize, b6);

    b7 = icalmemory_tmp_buffer(BUFSIZE);
    strcpy(b7, S7);
    icalmemory_append_string(&f, &p, &bufsize, b7);

    b8 = icalmemory_tmp_buffer(BUFSIZE);
    strcpy(b8, S8);
    icalmemory_append_string(&f, &p, &bufsize, b8);


    printf("1: %p %s \n",b1,b1);
    printf("2: %p %s\n",b2,b2);
    printf("3: %p %s\n",b3,b3);
    printf("4: %p %s\n",b4,b4);
    printf("5: %p %s\n",b5,b5);
    printf("6: %p %s\n",b6,b6);
    printf("7: %p %s\n",b7,b7);
    printf("8: %p %s\n",b8,b8);

    
    printf("Final: %s\n", f);

    printf("Final buffer size: %d\n",bufsize);

    free(f);
    
    bufsize = 4;

    f = icalmemory_new_buffer(bufsize);

    memset(f,0,bufsize);
    p = f;

    icalmemory_append_char(&f, &p, &bufsize, 'a');
    printf("Char-by-Char buffer: %s\n", f);
    icalmemory_append_char(&f, &p, &bufsize, 'b');
    printf("Char-by-Char buffer: %s\n", f);
    icalmemory_append_char(&f, &p, &bufsize, 'c');
    printf("Char-by-Char buffer: %s\n", f);
    icalmemory_append_char(&f, &p, &bufsize, 'd');
    printf("Char-by-Char buffer: %s\n", f);
    icalmemory_append_char(&f, &p, &bufsize, 'e');
    printf("Char-by-Char buffer: %s\n", f);
    icalmemory_append_char(&f, &p, &bufsize, 'f');
    printf("Char-by-Char buffer: %s\n", f);
    icalmemory_append_char(&f, &p, &bufsize, 'g');
    printf("Char-by-Char buffer: %s\n", f);
    icalmemory_append_char(&f, &p, &bufsize, 'h');
    printf("Char-by-Char buffer: %s\n", f);
    icalmemory_append_char(&f, &p, &bufsize, 'i');
    printf("Char-by-Char buffer: %s\n", f);
    icalmemory_append_char(&f, &p, &bufsize, 'j');
    printf("Char-by-Char buffer: %s\n", f);
    icalmemory_append_char(&f, &p, &bufsize, 'a');
    printf("Char-by-Char buffer: %s\n", f);
    icalmemory_append_char(&f, &p, &bufsize, 'b');
    printf("Char-by-Char buffer: %s\n", f);
    icalmemory_append_char(&f, &p, &bufsize, 'c');
    printf("Char-by-Char buffer: %s\n", f);
    icalmemory_append_char(&f, &p, &bufsize, 'd');
    printf("Char-by-Char buffer: %s\n", f);
    icalmemory_append_char(&f, &p, &bufsize, 'e');
    printf("Char-by-Char buffer: %s\n", f);
    icalmemory_append_char(&f, &p, &bufsize, 'f');
    printf("Char-by-Char buffer: %s\n", f);
    icalmemory_append_char(&f, &p, &bufsize, 'g');
    printf("Char-by-Char buffer: %s\n", f);
    icalmemory_append_char(&f, &p, &bufsize, 'h');
    printf("Char-by-Char buffer: %s\n", f);
    icalmemory_append_char(&f, &p, &bufsize, 'i');
    printf("Char-by-Char buffer: %s\n", f);
    icalmemory_append_char(&f, &p, &bufsize, 'j');
    printf("Char-by-Char buffer: %s\n", f);
   
    for(i=0; i<100; i++){
	f = icalmemory_tmp_buffer(bufsize);
	
	assert(f!=0);

	memset(f,0,bufsize);
	sprintf(f,"%d",i);
    }
}


int test_store()
{

    icalcomponent *c, *gauge;
    icalerrorenum error;
    icalcomponent *next, *itr;
    icalfileset* cluster;
    struct icalperiodtype rtime;
    icaldirset *s = icaldirset_new("store");
    int i;

    rtime.start = icaltime_from_timet( time(0),0);

    cluster = icalfileset_new("clusterin.vcd");

    if (cluster == 0){
	printf("Failed to create cluster: %s\n",icalerror_strerror(icalerrno));
	return 0;
    }

#define NUMCOMP 4

    /* Duplicate every component in the cluster NUMCOMP times */

    icalerror_clear_errno();

    for (i = 1; i<NUMCOMP+1; i++){

	/*rtime.start.month = i%12;*/
	rtime.start.month = i;
	rtime.end = rtime.start;
	rtime.end.hour++;
	
	for (itr = icalfileset_get_first_component(cluster);
	     itr != 0;
	     itr = icalfileset_get_next_component(cluster)){
	    icalcomponent *clone;
	    icalproperty *p;

	    
	    if(icalcomponent_isa(itr) != ICAL_VEVENT_COMPONENT){
		continue;
	    }

	    assert(itr != 0);
	    
	    /* Change the dtstart and dtend times in the component
               pointed to by Itr*/

	    clone = icalcomponent_new_clone(itr);
	    assert(icalerrno == ICAL_NO_ERROR);
	    assert(clone !=0);

	    /* DTSTART*/
	    p = icalcomponent_get_first_property(clone,ICAL_DTSTART_PROPERTY);
	    assert(icalerrno  == ICAL_NO_ERROR);

	    if (p == 0){
		p = icalproperty_new_dtstart(rtime.start);
		icalcomponent_add_property(clone,p);
	    } else {
		icalproperty_set_dtstart(p,rtime.start);
	    }
	    assert(icalerrno  == ICAL_NO_ERROR);

	    /* DTEND*/
	    p = icalcomponent_get_first_property(clone,ICAL_DTEND_PROPERTY);
	    assert(icalerrno  == ICAL_NO_ERROR);

	    if (p == 0){
		p = icalproperty_new_dtstart(rtime.end);
		icalcomponent_add_property(clone,p);
	    } else {
		icalproperty_set_dtstart(p,rtime.end);
	    }
	    assert(icalerrno  == ICAL_NO_ERROR);
	    
	    printf("\n----------\n%s\n---------\n",icalcomponent_as_ical_string(clone));

	    error = icaldirset_add_component(s,clone);
	    
	    assert(icalerrno  == ICAL_NO_ERROR);

	}

    }
    
    gauge = 
	icalcomponent_vanew(
	    ICAL_VCALENDAR_COMPONENT,
	    icalcomponent_vanew(
		ICAL_VEVENT_COMPONENT,  
		icalproperty_vanew_summary(
		    "Submit Income Taxes",
		    icalparameter_new_xliccomparetype(ICAL_XLICCOMPARETYPE_EQUAL),
		    0),
		0),
	    icalcomponent_vanew(
		ICAL_VEVENT_COMPONENT,  
		icalproperty_vanew_summary(
		    "Bastille Day Party",
		    icalparameter_new_xliccomparetype(ICAL_XLICCOMPARETYPE_EQUAL),
		    0),
		0),
	    0);

#if 0


    icaldirset_select(s,gauge);

    for(c = icaldirset_first(s); c != 0; c = icaldirset_next(s)){
	
	printf("Got one! (%d)\n", count++);
	
	if (c != 0){
	    printf("%s", icalcomponent_as_ical_string(c));;
	    if (icaldirset_store(s2,c) == 0){
		printf("Failed to write!\n");
	    }
	    icalcomponent_free(c);
	} else {
	    printf("Failed to get component\n");
	}
    }


    icaldirset_free(s2);
#endif


    for(c = icaldirset_get_first_component(s); 
	c != 0; 
	c =  next){
	
	next = icaldirset_get_next_component(s);

	if (c != 0){
	    /*icaldirset_remove_component(s,c);*/
	    printf("%s", icalcomponent_as_ical_string(c));;
	} else {
	    printf("Failed to get component\n");
	}


    }

    icaldirset_free(s);
    return 0;
}

int test_compare()
{
    icalvalue *v1, *v2;

    v1 = icalvalue_new_caladdress("cap://value/1");
    v2 = icalvalue_new_clone(v1);

    printf("%d\n",icalvalue_compare(v1,v2));

    v1 = icalvalue_new_caladdress("A");
    v2 = icalvalue_new_caladdress("B");

    printf("%d\n",icalvalue_compare(v1,v2));

    v1 = icalvalue_new_caladdress("B");
    v2 = icalvalue_new_caladdress("A");

    printf("%d\n",icalvalue_compare(v1,v2));

    v1 = icalvalue_new_integer(5);
    v2 = icalvalue_new_integer(5);

    printf("%d\n",icalvalue_compare(v1,v2));

    v1 = icalvalue_new_integer(5);
    v2 = icalvalue_new_integer(10);

    printf("%d\n",icalvalue_compare(v1,v2));

    v1 = icalvalue_new_integer(10);
    v2 = icalvalue_new_integer(5);

    printf("%d\n",icalvalue_compare(v1,v2));

    return 0;
}

void test_restriction()
{
    icalcomponent *comp;
    struct icaltimetype atime = icaltime_from_timet( time(0),0);
    int valid; 

    struct icalperiodtype rtime;

    rtime.start = icaltime_from_timet( time(0),0);
    rtime.end = icaltime_from_timet( time(0),0);

    rtime.end.hour++;

    comp = 
	icalcomponent_vanew(
	    ICAL_VCALENDAR_COMPONENT,
	    icalproperty_new_version("2.0"),
	    icalproperty_new_prodid("-//RDU Software//NONSGML HandCal//EN"),
	    icalproperty_new_method(ICAL_METHOD_REQUEST),
	    icalcomponent_vanew(
		ICAL_VTIMEZONE_COMPONENT,
		icalproperty_new_tzid("US_Eastern"),
		icalcomponent_vanew(
		    ICAL_XDAYLIGHT_COMPONENT,
		    icalproperty_new_dtstart(atime),
		    icalproperty_new_rdate(rtime),
		    icalproperty_new_tzoffsetfrom(-4.0),
		    icalproperty_new_tzoffsetto(-5.0),
		    icalproperty_new_tzname("EST"),
		    0
		    ),
		icalcomponent_vanew(
		    ICAL_XSTANDARD_COMPONENT,
		    icalproperty_new_dtstart(atime),
		    icalproperty_new_rdate(rtime),
		    icalproperty_new_tzoffsetfrom(-5.0),
		    icalproperty_new_tzoffsetto(-4.0),
		    icalproperty_new_tzname("EST"),
		    0
		    ),
		0
		),
	    icalcomponent_vanew(
		ICAL_VEVENT_COMPONENT,
		icalproperty_new_dtstamp(atime),
		icalproperty_new_uid("guid-1.host1.com"),
		icalproperty_vanew_organizer(
		    "mrbig@host.com",
		    icalparameter_new_role(ICAL_ROLE_CHAIR),
		    0
		    ),
		icalproperty_vanew_attendee(
		    "employee-A@host.com",
		    icalparameter_new_role(ICAL_ROLE_REQPARTICIPANT),
		    icalparameter_new_rsvp(1),
		    icalparameter_new_cutype(ICAL_CUTYPE_GROUP),
		    0
		    ),
		icalproperty_new_description("Project XYZ Review Meeting"),
		icalproperty_new_categories("MEETING"),
		icalproperty_new_class("PUBLIC"),
		icalproperty_new_created(atime),
		icalproperty_new_summary("XYZ Project Review"),
/*		icalproperty_vanew_dtstart(
		    atime,
		    icalparameter_new_tzid("US-Eastern"),
		    0
		    ),*/
		icalproperty_vanew_dtend(
		    atime,
		    icalparameter_new_tzid("US-Eastern"),
		    0
		    ),
		icalproperty_new_location("1CP Conference Room 4350"),
		0
		),
	    0
	    );

    valid = icalrestriction_check(comp);

    printf("#### %d ####\n%s\n",valid, icalcomponent_as_ical_string(comp));

}

#if 0
void test_calendar()
{
    icalcomponent *comp;
    icalfileset *c;
    icaldirset *s;
    icalcalendar* calendar = icalcalendar_new("calendar");
    icalerrorenum error;
    struct icaltimetype atime = icaltime_from_timet( time(0),0);

    comp = icalcomponent_vanew(
	ICAL_VEVENT_COMPONENT,
	icalproperty_new_version("2.0"),
	icalproperty_new_description("This is an event"),
	icalproperty_new_dtstart(atime),
	icalproperty_vanew_comment(
	    "Another Comment",
	    icalparameter_new_cn("A Common Name 1"),
	    icalparameter_new_cn("A Common Name 2"),
	    icalparameter_new_cn("A Common Name 3"),
		icalparameter_new_cn("A Common Name 4"),
	    0),
	icalproperty_vanew_xlicerror(
	    "This is only a test",
	    icalparameter_new_xlicerrortype(ICAL_XLICERRORTYPE_COMPONENTPARSEERROR),
	    0),
	
	0);
	
	
    s = icalcalendar_get_booked(calendar);

    error = icaldirset_add_component(s,comp);
    
    assert(error == ICAL_NO_ERROR);

    c = icalcalendar_get_properties(calendar);

    error = icalfileset_add_component(c,icalcomponent_new_clone(comp));

    assert(error == ICAL_NO_ERROR);

    icalcalendar_free(calendar);

}
#endif

void test_increment(void);

void print_occur(struct icalrecurrencetype recur, struct icaltimetype start)
{
    struct icaltimetype next;
    icalrecur_iterator* ritr;
	
    time_t tt = icaltime_as_timet(start);

    printf("#### %s\n",icalrecurrencetype_as_string(&recur));
    printf("#### %s\n",ctime(&tt ));
    
    for(ritr = icalrecur_iterator_new(recur,start),
	    next = icalrecur_iterator_next(ritr); 
	!icaltime_is_null_time(next);
	next = icalrecur_iterator_next(ritr)){
	
	tt = icaltime_as_timet(next);
	
	printf("  %s",ctime(&tt ));		
	
    }

    icalrecur_iterator_free(ritr);
}

void test_recur()
{
   struct icalrecurrencetype rt;
   struct icaltimetype start;
   time_t array[25];
   int i;

   rt = icalrecurrencetype_from_string("FREQ=MONTHLY;UNTIL=19971224T000000Z;INTERVAL=1;BYDAY=TU,2FR,3SA");
   start = icaltime_from_string("19970905T090000Z");

   print_occur(rt,start);   

   printf("\n  Using icalrecur_expand_recurrence\n");

   icalrecur_expand_recurrence("FREQ=MONTHLY;UNTIL=19971224T000000Z;INTERVAL=1;BYDAY=TU,2FR,3SA",
			       icaltime_as_timet(start),
			       25,
			       array);

   for(i =0; array[i] != 0 && i < 25 ; i++){
       
       printf("  %s",ctime(&(array[i])));
   }

   
/*    test_increment();*/

}


enum byrule {
    NO_CONTRACTION = -1,
    BY_SECOND = 0,
    BY_MINUTE = 1,
    BY_HOUR = 2,
    BY_DAY = 3,
    BY_MONTH_DAY = 4,
    BY_YEAR_DAY = 5,
    BY_WEEK_NO = 6,
    BY_MONTH = 7,
    BY_SET_POS
};

struct icalrecur_iterator_impl {
	
	struct icaltimetype dtstart; 
	struct icaltimetype last; /* last time return from _iterator_next*/
	int occurrence_no; /* number of step made on this iterator */
	struct icalrecurrencetype rule;

	short days[366];
	short days_index;

	enum byrule byrule;
	short by_indices[9];


	short *by_ptrs[9]; /* Pointers into the by_* array elements of the rule */
};

void icalrecurrencetype_test()
{
    icalvalue *v = icalvalue_new_from_string(
	ICAL_RECUR_VALUE,
	"FREQ=YEARLY;UNTIL=20060101T000000;INTERVAL=2;BYDAY=SU,WE;BYSECOND=15,30; BYMONTH=1,6,11");

    struct icalrecurrencetype r = icalvalue_get_recur(v);
    struct icaltimetype t = icaltime_from_timet( time(0), 0);
    struct icaltimetype next;
    time_t tt;

    struct icalrecur_iterator_impl* itr 
	= (struct icalrecur_iterator_impl*) icalrecur_iterator_new(r,t);

    do {

	next = icalrecur_iterator_next(itr);
	tt = icaltime_as_timet(next);

	printf("%s",ctime(&tt ));		

    } while( ! icaltime_is_null_time(next));
 
}

/* From Federico Mena Quintero <federico@helixcode.com>    */
void test_recur_parameter_bug(){

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
 
#include <stdio.h>
#include <stdlib.h>
#include <ical.h>
 
    static const char test_icalcomp_str[] =
"BEGIN:VEVENT\r\n"
"RRULE\r\n"
" ;X-EVOLUTION-ENDDATE=20030209T081500\r\n"
" :FREQ=DAILY;COUNT=10;INTERVAL=6\r\n"
"END:VEVENT\r\n";
  
    icalcomponent *icalcomp;
    icalproperty *prop;
    struct icalrecurrencetype recur;
    int n_errors;
    
    icalcomp = icalparser_parse_string ((char *) test_icalcomp_str);
    if (!icalcomp) {
	fprintf (stderr, "main(): could not parse the component\n");
	exit (EXIT_FAILURE);
    }
    
    printf("%s\n\n",icalcomponent_as_ical_string(icalcomp));

    n_errors = icalcomponent_count_errors (icalcomp);
    if (n_errors) {
	icalproperty *p;
	
	for (p = icalcomponent_get_first_property (icalcomp,
						   ICAL_XLICERROR_PROPERTY);
	     p;
	     p = icalcomponent_get_next_property (icalcomp,
						  ICAL_XLICERROR_PROPERTY)) {
	    char *str;
	    
	    str = icalproperty_as_ical_string (p);
	    fprintf (stderr, "error: %s\n", str);
	}
    }
    
    prop = icalcomponent_get_first_property (icalcomp, ICAL_RRULE_PROPERTY);
    if (!prop) {
	fprintf (stderr, "main(): could not get the RRULE property");
	exit (EXIT_FAILURE);
    }
    
    recur = icalproperty_get_rrule (prop);
    
    printf("%s\n",icalrecurrencetype_as_string(&recur));

}


void test_duration()
{

    icalvalue *v;

    v = icalvalue_new_from_string(ICAL_DURATION_VALUE,
				  "PT8H30M");

    printf("%s\n",icalvalue_as_ical_string(v));


    v = icalvalue_new_from_string(ICAL_DURATION_VALUE,
				  "-PT8H30M");

    printf("%s\n",icalvalue_as_ical_string(v));

    icalvalue_free(v);
    v = icalvalue_new_from_string(ICAL_PERIOD_VALUE,
				  "19971015T050000Z/PT8H30M");

    printf("%s\n",icalvalue_as_ical_string(v));

    icalvalue_free(v);
    v = icalvalue_new_from_string(ICAL_PERIOD_VALUE,
				  "19971015T050000Z/19971015T060000Z");

    printf("%s\n",icalvalue_as_ical_string(v));
    icalvalue_free(v);


}


void test_strings(){

    icalvalue *v;

    v = icalvalue_new_text("foo;bar;bats");
    
    printf("%s\n",icalvalue_as_ical_string(v));

    icalvalue_free(v);

    v = icalvalue_new_text("foo\\;b\nar\\;ba\tts");
    
    printf("%s\n",icalvalue_as_ical_string(v));
    
    icalvalue_free(v);


}

void test_requeststat()
{
  icalrequeststatus s;
  struct icalreqstattype st, st2;
  char temp[1024];

  s = icalenum_num_to_reqstat(2,1);

  assert(s == ICAL_2_1_FALLBACK_STATUS);

  assert(icalenum_reqstat_major(s) == 2);
  assert(icalenum_reqstat_minor(s) == 1);

  printf("2.1: %s\n",icalenum_reqstat_desc(s));

  st.code = s;
  st.debug = "booga";
  st.desc = 0;

  printf("%s\n",icalreqstattype_as_string(st));

  st.desc = " A non-standard description";

  printf("%s\n",icalreqstattype_as_string(st));


  st.desc = 0;

  sprintf(temp,"%s\n",icalreqstattype_as_string(st));
  

  st2 = icalreqstattype_from_string("2.1;Success but fallback taken  on one or more property  values.;booga");

  printf("%d --  %d --  %s -- %s\n",icalenum_reqstat_major(st2.code),
         icalenum_reqstat_minor(st2.code),
         icalenum_reqstat_desc(st2.code),
         st2.debug);

  st2 = icalreqstattype_from_string("2.1;Success but fallback taken  on one or more property  values.;booga");
  printf("%s\n",icalreqstattype_as_string(st2));

  st2 = icalreqstattype_from_string("2.1;Success but fallback taken  on one or more property  values.;");
  printf("%s\n",icalreqstattype_as_string(st2));

  st2 = icalreqstattype_from_string("2.1;Success but fallback taken  on one or more property  values.");
  printf("%s\n",icalreqstattype_as_string(st2));

  st2 = icalreqstattype_from_string("2.1;");
  printf("%s\n",icalreqstattype_as_string(st2));

  st2 = icalreqstattype_from_string("2.1");
  printf("%s\n",icalreqstattype_as_string(st2));

#ifndef ICAL_ERRORS_ARE_FATAL
  st2 = icalreqstattype_from_string("16.4");
  assert(st2.code == ICAL_UNKNOWN_STATUS);

  st2 = icalreqstattype_from_string("1.");
  assert(st2.code == ICAL_UNKNOWN_STATUS);
#endif
}

char ictt_str[1024];
char* ictt_as_string(struct icaltimetype t)
{

    sprintf(ictt_str,"%02d-%02d-%02d %02d:%02d:%02d%s",t.year,t.month,t.day,
	    t.hour,t.minute,t.second,t.is_utc?" Z":"");

    return ictt_str;
}


char* ical_timet_string(time_t t)
{
    struct tm stm = *(gmtime(&t));    

    sprintf(ictt_str,"%02d-%02d-%02d %02d:%02d:%02d Z",stm.tm_year+1900,
	    stm.tm_mon+1,stm.tm_mday,stm.tm_hour,stm.tm_min,stm.tm_sec);

    return ictt_str;
    
}

/* This test was written in GMT-8, and it might run differently in
   another timezone */



void test_time()
{
    struct icaltimetype ictt, icttutc, icttutczone, 
	icttla, icttny,icttphoenix;
    time_t tt,tt2, tt_p200;
    icalvalue *v;
    short day_of_week,start_day_of_week, day_of_year;


    tt = 973276230; /* Fri Nov  3 10:30:30 PST 2000 in PST 
                       Fri Nov  3 18:30:30 PST 2000 in UTC */

    printf(" Normalize \n");
    ictt = icaltime_from_string("20001103T183030Z");
    printf("Orig (ical) : %s\n", ictt_as_string(ictt));
    ictt.second -= 60 * 60 * 24 * 5;
    ictt = icaltime_normalize(ictt);
    printf("-5d in sec  : %s\n", ictt_as_string(ictt));
    ictt.day += 60;
    ictt = icaltime_normalize(ictt);
    printf("+60 d       : %s\n", ictt_as_string(ictt));


    printf("\n As time_t \n");

    ictt = icaltime_from_string("20001103T183030Z");
    tt2 = icaltime_as_timet(ictt);
    assert(tt2 == tt);
    printf("20001103T183030Z (timet): %s\n",ical_timet_string(tt2));
    printf("20001103T183030Z        : %s\n",ictt_as_string(ictt));

    ictt = icaltime_from_string("20001103T103030");
    tt2 = icaltime_as_timet(ictt);
    assert(tt2 == tt);
    printf("20001103T103030  (timet): %s\n",ical_timet_string(tt2));
    printf("20001103T103030         : %s\n",ictt_as_string(ictt));

    printf("\n From time_t \n");

    ictt = icaltime_from_timet(tt,0);
    printf("Orig        : %s\n",ical_timet_string(tt));
    printf("As utc      : %s\n", ictt_as_string(ictt));
    ictt = icaltime_from_timet(tt,0);
    ictt = icaltime_as_local(ictt);
    printf("As local    : %s\n", ictt_as_string(ictt));
    

    printf("\n Convert to and from lib c \n");

    printf("System time is: %s\n",ical_timet_string(tt));

    ictt = icaltime_from_timet(tt,0);

    v = icalvalue_new_datetime(ictt);

    printf("System time from libical: %s\n",icalvalue_as_ical_string(v));

    tt2 = icaltime_as_timet(ictt);
    printf("Converted back to libc: %s\n",ical_timet_string(tt2));
    
    printf("\n Incrementing time  \n");

    ictt.year++;
    tt2 = icaltime_as_timet(ictt);
    printf("Add a year: %s\n",ical_timet_string(tt2));

    ictt.month+=13;
    tt2 = icaltime_as_timet(ictt);
    printf("Add 13 months: %s\n",ical_timet_string(tt2));

    ictt.second+=90;
    tt2 = icaltime_as_timet(ictt);
    printf("Add 90 seconds: %s\n",ical_timet_string(tt2));

    ictt = icaltime_from_timet(tt,0);

    printf("\n Day Of week \n");

    day_of_week = icaltime_day_of_week(ictt);
    start_day_of_week = icaltime_start_doy_of_week(ictt);
    day_of_year = icaltime_day_of_year(ictt);

    assert(day_of_week == 6);
    assert(day_of_year == 308);
    assert(start_day_of_week == 303 );
    printf("Today is day of week %d, day of year %d\n",day_of_week,day_of_year);
    printf("Week started n doy of %d\n",start_day_of_week);

    printf("\n TimeZone Conversions \n");

    ictt = icaltime_from_timet(tt,0);
    
    icttla = icaltime_as_zone(ictt,"America/Los_Angeles");

    icttutc = icaltime_as_utc(icttla,"America/Los_Angeles");
    assert(icaltime_compare(icttla,
			 icaltime_from_string("20001103T103030"))==0);

    icttutczone = icaltime_as_zone(ictt,"Etc/GMT0");
    icttutczone.is_utc = 1;
    assert(icaltime_compare(icttutc, icttutczone) == 0); 
    assert(icaltime_compare(icttutc, ictt) == 0); 

    icttny  = icaltime_as_zone(ictt,"America/New_York");

    icttphoenix = icaltime_as_zone(ictt,"America/Phoenix");

    printf("Orig (ctime): %s\n", ical_timet_string(tt) );
    printf("Orig (ical) : %s\n", ictt_as_string(ictt));
    printf("UTC         : %s\n", ictt_as_string(icttutc));
    printf("Los Angeles : %s\n", ictt_as_string(icttla));
    printf("Phoenix     : %s\n", ictt_as_string(icttphoenix));
    printf("New York    : %s\n", ictt_as_string(icttny));


    /* Daylight savings test for New York */
    printf("\n Daylight Savings \n");

    printf("Orig (ctime): %s\n", ical_timet_string(tt) );
    printf("Orig (ical) : %s\n", ictt_as_string(ictt));
    printf("NY          : %s\n", ictt_as_string(icttny));
    printf("NY Day Offs : %d\n", icaltime_daylight_offset(icttny,
						     "America/New_York"));

    assert(strcmp(ictt_as_string(icttny),"2000-11-03 13:30:30")==0);
    assert(icaltime_daylight_offset(icttny,"America/New_York") == 0);


    tt_p200 = tt +  200 * 24 * 60 * 60 ; /* Add 200 days */

    ictt = icaltime_from_timet(tt_p200,0);    
    icttny = icaltime_as_zone(ictt,"America/New_York");
    
    printf("Orig +200d  : %s\n", ical_timet_string(tt_p200) );
    printf("NY+200D     : %s\n", ictt_as_string(icttny));
    printf("NY Offs     : %d\n", icaltime_daylight_offset(icttny,
						     "America/New_York"));

    assert(strcmp(ictt_as_string(icttny),"2001-05-22 14:30:30")==0);
    assert(icaltime_daylight_offset(icttny,"America/New_York") == 3600);

    /* Daylight savings test for Los Angeles */

    ictt = icaltime_from_timet(tt,0);    
    icttla  = icaltime_as_zone(ictt,"America/Los_Angeles");

    printf("\nOrig (ctime): %s\n", ical_timet_string(tt) );
    printf("Orig (ical) : %s\n", ictt_as_string(ictt));
    printf("LA          : %s\n", ictt_as_string(icttla));
    printf("LA Day Offs : %d\n", icaltime_daylight_offset(icttla,
						     "America/Los_Angeles"));

    assert(strcmp(ictt_as_string(icttla),"2000-11-03 10:30:30")==0);
    assert(icaltime_daylight_offset(icttla,"America/Los_Angeles") == 0);
						     
    
    ictt = icaltime_from_timet(tt_p200,0);    
    icttla = icaltime_as_zone(ictt,"America/Los_Angeles");
    
    printf("Orig +200d  : %s\n", ical_timet_string(tt_p200) );
    printf("LA+200D     : %s\n", ictt_as_string(icttla));
    printf("LA Offs     : %d\n", icaltime_daylight_offset(icttla,
						     "America/Los_Angeles"));

    assert(strcmp(ictt_as_string(icttla),"2001-05-22 11:30:30")==0);
    assert(icaltime_daylight_offset(icttla,"America/Los_Angeles") == 3600);

}

void test_iterators()
{
    icalcomponent *c,*inner,*next;
    icalcompiter i;

    c= icalcomponent_vanew(
	ICAL_VCALENDAR_COMPONENT,
	icalcomponent_vanew(ICAL_VEVENT_COMPONENT,
			    icalproperty_vanew_version("1"),0),
	icalcomponent_vanew(ICAL_VEVENT_COMPONENT,
			    icalproperty_vanew_version("2"),0),
	icalcomponent_vanew(ICAL_VEVENT_COMPONENT,
			    icalproperty_vanew_version("3"),0),
	icalcomponent_vanew(ICAL_VEVENT_COMPONENT,
			    icalproperty_vanew_version("4"),0),
	icalcomponent_vanew(ICAL_VTODO_COMPONENT,
			    icalproperty_vanew_version("5"),0),
	icalcomponent_vanew(ICAL_VJOURNAL_COMPONENT,
			    icalproperty_vanew_version("6"),0),
	icalcomponent_vanew(ICAL_VEVENT_COMPONENT,
			    icalproperty_vanew_version("7"),0),
	icalcomponent_vanew(ICAL_VJOURNAL_COMPONENT,
			    icalproperty_vanew_version("8"),0),
	icalcomponent_vanew(ICAL_VJOURNAL_COMPONENT,
			    icalproperty_vanew_version("9"),0),
	icalcomponent_vanew(ICAL_VJOURNAL_COMPONENT,
			    icalproperty_vanew_version("10"),0),
	0);
   
    printf("1: ");

    /* List all of the VEVENTS */
    for(i = icalcomponent_begin_component(c,ICAL_VEVENT_COMPONENT);
	icalcompiter_deref(&i)!= 0; icalcompiter_next(&i)){
	
	icalcomponent *this = icalcompiter_deref(&i);

	icalproperty *p = 
	    icalcomponent_get_first_property(this,
					     ICAL_VERSION_PROPERTY);
	const char* s = icalproperty_get_version(p);

	printf("%s ",s);
	    
    }

    printf("\n2: ");

#if 0
    for(inner = icalcomponent_get_first_component(c,ICAL_VEVENT_COMPONENT); 
	inner != 0; 
	inner = next){
	
	next = icalcomponent_get_next_component(c,ICAL_VEVENT_COMPONENT);

	icalcomponent_remove_component(c,inner);

	icalcomponent_free(inner);
    }
#endif

    /* Delete all of the VEVENTS */
    /* reset iterator */
    icalcomponent_get_first_component(c,ICAL_VEVENT_COMPONENT);

    while((inner=icalcomponent_get_current_component(c)) != 0 ){
	if(icalcomponent_isa(inner) == ICAL_VEVENT_COMPONENT){
	    icalcomponent_remove_component(c,inner);
	} else {
	    icalcomponent_get_next_component(c,ICAL_VEVENT_COMPONENT);
	}
    }
	


    /* List all remaining components */
    for(inner = icalcomponent_get_first_component(c,ICAL_ANY_COMPONENT); 
	inner != 0; 
	inner = icalcomponent_get_next_component(c,ICAL_ANY_COMPONENT)){
	

	icalproperty *p = 
	    icalcomponent_get_first_property(inner,ICAL_VERSION_PROPERTY);

	const char* s = icalproperty_get_version(p);	

	printf("%s ",s);
    }

    printf("\n3: ");

    
    /* Remove all remaining components */
    for(inner = icalcomponent_get_first_component(c,ICAL_ANY_COMPONENT); 
	inner != 0; 
	inner = next){

	icalcomponent *this;
	icalproperty *p;
	const char* s;
	next = icalcomponent_get_next_component(c,ICAL_ANY_COMPONENT);

	p=icalcomponent_get_first_property(inner,ICAL_VERSION_PROPERTY);
	s = icalproperty_get_version(p);	
	printf("rem:%s ",s);

	icalcomponent_remove_component(c,inner);

	this = icalcomponent_get_current_component(c);

	if(this != 0){
	    p=icalcomponent_get_first_property(this,ICAL_VERSION_PROPERTY);
	    s = icalproperty_get_version(p);	
	    printf("next:%s; ",s);
	}

	icalcomponent_free(inner);
    }

    printf("\n4: ");


    /* List all remaining components */
    for(inner = icalcomponent_get_first_component(c,ICAL_ANY_COMPONENT); 
	inner != 0; 
	inner = icalcomponent_get_next_component(c,ICAL_ANY_COMPONENT)){
	
	icalproperty *p = 
	    icalcomponent_get_first_property(inner,ICAL_VERSION_PROPERTY);

	const char* s = icalproperty_get_version(p);	

	printf("%s ",s);
    }

    printf("\n");
}


void test_icalset()
{
    icalcomponent *c; 

    icalset* f = icalset_new_file("2446.ics");
    icalset* d = icalset_new_dir("outdir");

    assert(f!=0);
    assert(d!=0);

    for(c = icalset_get_first_component(f);
	c != 0;
	c = icalset_get_next_component(f)){

	icalcomponent *clone; 

	clone = icalcomponent_new_clone(c);

	icalset_add_component(d,clone);

	printf(" class %d\n",icalclassify(c,0,"user"));

    }
}

void test_classify()
{
    icalcomponent *c,*match; 

    icalset* f = icalset_new_file("../../test-data/classify.ics");

    assert(f!=0);

    c = icalset_get_first_component(f);
    match = icalset_get_next_component(f);
    
    printf("Class %d\n",icalclassify(c,match,"A@example.com"));

   
}

void print_span(int c, struct icaltime_span span ){

    printf("#%02d start: %s\n",c,ical_timet_string(span.start));
    printf("    end  : %s\n",ical_timet_string(span.end));

}

void test_span()
{
    time_t tm1 = 973378800; /*Sat Nov  4 23:00:00 UTC 2000,
			      Sat Nov  4 15:00:00 PST 2000 */
    time_t tm2 = 973382400; /*Sat Nov  5 00:00:00 UTC 2000 
			      Sat Nov  4 16:00:00 PST 2000 */
    struct icaldurationtype dur;
    struct icaltime_span span;
    icalcomponent *c; 

    memset(&dur,0,sizeof(dur));
    dur.minutes = 30;

    span.start = tm1;
    span.end = tm2;
    print_span(0,span);

    /* Specify save timezone as in commend above */
    c = 
	icalcomponent_vanew(
	    ICAL_VEVENT_COMPONENT,
		icalproperty_vanew_dtstart(
		    icaltime_as_local(icaltime_from_timet(tm1,0)),
		    icalparameter_new_tzid("US/Pacific"),0),
		icalproperty_vanew_dtend(
		    icaltime_as_local(icaltime_from_timet(tm2,0)),
		    icalparameter_new_tzid("US/Pacific"),0),
	    0
	    );

    printf("%s\n",icalcomponent_as_ical_string(c));

    span = icalcomponent_get_span(c);

    print_span(1,span);

    icalcomponent_free(c);

    /* Use machine's local timezone. Same as above if run in US/Pacific */
    c = 
	icalcomponent_vanew(
	    ICAL_VEVENT_COMPONENT,
	    icalproperty_vanew_dtstart(icaltime_from_timet(tm1,0),0),
	    icalproperty_vanew_dtend(icaltime_from_timet(tm2,0),0),
	    0
	    );

    span = icalcomponent_get_span(c);

    print_span(2,span);

    icalcomponent_free(c);

    /* Specify different timezone */
    c = 
	icalcomponent_vanew(
	    ICAL_VEVENT_COMPONENT,
		icalproperty_vanew_dtstart(
		    icaltime_as_local(icaltime_from_timet(tm1,0)),
		    icalparameter_new_tzid("US/Eastern"),0),
		icalproperty_vanew_dtend(
		    icaltime_as_local(icaltime_from_timet(tm2,0)),
		    icalparameter_new_tzid("US/Eastern"),0),
	    0
	    );
    span = icalcomponent_get_span(c);
    print_span(3,span);

    icalcomponent_free(c);


    /* Specify different timezone for start and end*/
    c = 
	icalcomponent_vanew(
	    ICAL_VEVENT_COMPONENT,
		icalproperty_vanew_dtstart(
		    icaltime_as_local(icaltime_from_timet(tm1,0)),
		    icalparameter_new_tzid("US/Eastern"),0),
		icalproperty_vanew_dtend(
		    icaltime_as_local(icaltime_from_timet(tm2,0)),
		    icalparameter_new_tzid("US/Pacific"),0),
	    0
	    );
    span = icalcomponent_get_span(c);
    print_span(4,span);

    icalcomponent_free(c);

    /* Use Duration */
    c = 
	icalcomponent_vanew(
	    ICAL_VEVENT_COMPONENT,
		icalproperty_vanew_dtstart(
		    icaltime_as_local(icaltime_from_timet(tm1,0)),
		    icalparameter_new_tzid("US/Pacific"),0),
	    icalproperty_new_duration(dur),

	    0
	    );
    span = icalcomponent_get_span(c);
    print_span(5,span);

    icalcomponent_free(c);


#ifndef ICAL_ERRORS_ARE_FATAL
    /* Both UTC and Timezone -- an error */
    icalerror_clear_errno();
    c = 
	icalcomponent_vanew(
	    ICAL_VEVENT_COMPONENT,
		icalproperty_vanew_dtstart(icaltime_from_timet(tm1,0),
		    icalparameter_new_tzid("US/Eastern"),0),
		icalproperty_vanew_dtend(icaltime_from_timet(tm2,0),
		    icalparameter_new_tzid("US/Eastern"),0),
	    0
	    );

    span = icalcomponent_get_span(c);
    assert(icalerrno != ICAL_NO_ERROR);

    icalcomponent_free(c);

#endif /*ICAL_ERRORS_ARE_FATAL*/

}

icalcomponent* icalclassify_find_overlaps(icalset* set, icalcomponent* comp);

void test_overlaps()
{

#if 0 /* Hack, not working right now */
    icalcomponent *cset,*c;
    icalset *set;
    time_t tm1 = 973378800; /*Sat Nov  4 23:00:00 UTC 2000,
			      Sat Nov  4 15:00:00 PST 2000 */
    time_t tm2 = 973382400; /*Sat Nov  5 00:00:00 UTC 2000 
			      Sat Nov  4 16:00:00 PST 2000 */

    time_t hh = 1800; /* one half hour */

    set = icalset_new_file("../../test-data/overlaps.ics");

    printf("-- 1 -- \n");
    c = icalcomponent_vanew(
	ICAL_VEVENT_COMPONENT,
	icalproperty_vanew_dtstart(icaltime_from_timet(tm1-hh,0),0),
	icalproperty_vanew_dtend(icaltime_from_timet(tm2-hh,0),0),
	0
	);

    cset =  icalclassify_find_overlaps(set,c);

    printf("%s\n",icalcomponent_as_ical_string(cset));

    printf("-- 2 -- \n");
    c = icalcomponent_vanew(
	ICAL_VEVENT_COMPONENT,
	icalproperty_vanew_dtstart(icaltime_from_timet(tm1-hh,0),0),
	icalproperty_vanew_dtend(icaltime_from_timet(tm2,0),0),
	0
	);

    cset =  icalclassify_find_overlaps(set,c);

    printf("%s\n",icalcomponent_as_ical_string(cset));

    printf("-- 3 -- \n");
    c = icalcomponent_vanew(
	ICAL_VEVENT_COMPONENT,
	icalproperty_vanew_dtstart(icaltime_from_timet(tm1+5*hh,0),0),
	icalproperty_vanew_dtend(icaltime_from_timet(tm2+5*hh,0),0),
	0
	);

    cset =  icalclassify_find_overlaps(set,c);

    printf("%s\n",icalcomponent_as_ical_string(cset));

#endif 

}

void test_fblist()
{
    icalspanlist *sl;
    icalset* set = icalset_new_file("../../test-data/spanlist.ics");
    struct icalperiodtype period;

    sl = icalspanlist_new(set,
		     icaltime_from_string("19970324T1200Z"),
		     icaltime_from_string("19990424T020000Z"));
		
    printf("Restricted spanlist\n");
    icalspanlist_dump(sl);

    period= icalspanlist_next_free_time(sl,
				      icaltime_from_string("19970801T1200Z"));


    printf("Next Free time: %s\n",icaltime_as_ctime(period.start));
    printf("                %s\n",icaltime_as_ctime(period.end));


    icalspanlist_free(sl);

    printf("Unrestricted spanlist\n");

    sl = icalspanlist_new(set,
		     icaltime_from_string("19970324T1200Z"),
			  icaltime_null_time());
		
    printf("Restricted spanlist\n");

    icalspanlist_dump(sl);

    period= icalspanlist_next_free_time(sl,
				      icaltime_from_string("19970801T1200Z"));


    printf("Next Free time: %s\n",icaltime_as_ctime(period.start));
    printf("                %s\n",icaltime_as_ctime(period.end));


    icalspanlist_free(sl);


}

void test_convenience(){
    
    icalcomponent *c;
    int duration;

    c = icalcomponent_vanew(
	ICAL_VCALENDAR_COMPONENT,
	icalcomponent_vanew(
	    ICAL_VEVENT_COMPONENT,
	    icalproperty_new_dtstart(icaltime_from_string("19970801T1200")),
	    icalproperty_new_dtend(icaltime_from_string("19970801T1300")),
	    0
	    ),
	0);

    printf("** 1 DTSTART and DTEND **\n%s\n\n",
	   icalcomponent_as_ical_string(c));

    duration = icaldurationtype_as_int(icalcomponent_get_duration(c))/60;

    printf("Start: %s\n",ictt_as_string(icalcomponent_get_dtstart(c)));
    printf("End:   %s\n",ictt_as_string(icalcomponent_get_dtend(c)));
    printf("Dur:   %d m\n",duration);

    icalcomponent_free(c);

    c = icalcomponent_vanew(
	ICAL_VCALENDAR_COMPONENT,
	icalcomponent_vanew(
	    ICAL_VEVENT_COMPONENT,
	    icalproperty_new_dtstart(icaltime_from_string("19970801T1200Z")),
	    icalproperty_new_duration(icaldurationtype_from_string("PT1H30M")),
	    0
	    ),
	0);

    printf("\n** 2 DTSTART and DURATION **\n%s\n\n",
	   icalcomponent_as_ical_string(c));

    duration = icaldurationtype_as_int(icalcomponent_get_duration(c))/60;

    printf("Start: %s\n",ictt_as_string(icalcomponent_get_dtstart(c)));
    printf("End:   %s\n",ictt_as_string(icalcomponent_get_dtend(c)));
    printf("Dur:   %d m\n",duration);

    icalcomponent_free(c);

    c = icalcomponent_vanew(
	ICAL_VCALENDAR_COMPONENT,
	icalcomponent_vanew(
	    ICAL_VEVENT_COMPONENT,
	    icalproperty_new_dtstart(icaltime_from_string("19970801T1200")),
	    icalproperty_new_dtend(icaltime_from_string("19970801T1300")),
	    0
	    ),
	0);

    icalcomponent_set_duration(c,icaldurationtype_from_string("PT1H30M"));

    printf("** 3 DTSTART and DTEND, Set DURATION **\n%s\n\n",
	   icalcomponent_as_ical_string(c));

    duration = icaldurationtype_as_int(icalcomponent_get_duration(c))/60;

    printf("Start: %s\n",ictt_as_string(icalcomponent_get_dtstart(c)));
    printf("End:   %s\n",ictt_as_string(icalcomponent_get_dtend(c)));
    printf("Dur:   %d m\n",duration);

    icalcomponent_free(c);

    c = icalcomponent_vanew(
	ICAL_VCALENDAR_COMPONENT,
	icalcomponent_vanew(
	    ICAL_VEVENT_COMPONENT,
	    icalproperty_new_dtstart(icaltime_from_string("19970801T1200Z")),
	    icalproperty_new_duration(icaldurationtype_from_string("PT1H30M")),
	    0
	    ),
	0);

    icalcomponent_set_dtend(c,icaltime_from_string("19970801T1330Z"));

    printf("\n** 4 DTSTART and DURATION, set DTEND **\n%s\n\n",
	   icalcomponent_as_ical_string(c));

    duration = icaldurationtype_as_int(icalcomponent_get_duration(c))/60;

    printf("Start: %s\n",ictt_as_string(icalcomponent_get_dtstart(c)));
    printf("End:   %s\n",ictt_as_string(icalcomponent_get_dtend(c)));
    printf("Dur:   %d m\n",duration);

    icalcomponent_free(c);

    c = icalcomponent_vanew(
	ICAL_VCALENDAR_COMPONENT,
	icalcomponent_vanew(
	    ICAL_VEVENT_COMPONENT,
	    0
	    ),
	0);

    icalcomponent_set_dtstart(c,icaltime_from_string("19970801T1200Z"));
    icalcomponent_set_dtend(c,icaltime_from_string("19970801T1330Z"));

    printf("\n** 5 Set DTSTART and DTEND **\n%s\n\n",
	   icalcomponent_as_ical_string(c));


    duration = icaldurationtype_as_int(icalcomponent_get_duration(c))/60;

    printf("Start: %s\n",ictt_as_string(icalcomponent_get_dtstart(c)));
    printf("End:   %s\n",ictt_as_string(icalcomponent_get_dtend(c)));
    printf("Dur:   %d m\n",duration);

    icalcomponent_free(c);

    c = icalcomponent_vanew(
	ICAL_VCALENDAR_COMPONENT,
	icalcomponent_vanew(
	    ICAL_VEVENT_COMPONENT,
	    0
	    ),
	0);


    icalcomponent_set_dtstart(c,icaltime_from_string("19970801T1200Z"));
    icalcomponent_set_duration(c,icaldurationtype_from_string("PT1H30M"));

    printf("\n** 6 Set DTSTART and DURATION **\n%s\n\n",
	   icalcomponent_as_ical_string(c));


    duration = icaldurationtype_as_int(icalcomponent_get_duration(c))/60;

    printf("Start: %s\n",ictt_as_string(icalcomponent_get_dtstart(c)));
    printf("End:   %s\n",ictt_as_string(icalcomponent_get_dtend(c)));
    printf("Dur:   %d m\n",duration);

    icalcomponent_free(c);

}

void test_time_parser()
{
    struct icaltimetype tt;

    tt = icaltime_from_string("19970101T1000");
    assert(icaltime_is_null_time(tt));

    tt = icaltime_from_string("19970101X100000");
    assert(icaltime_is_null_time(tt));

    tt = icaltime_from_string("19970101T100000");
    assert(!icaltime_is_null_time(tt));
    printf("%s\n",icaltime_as_ctime(tt));

    tt = icaltime_from_string("19970101T100000Z");
    assert(!icaltime_is_null_time(tt));
    printf("%s\n",icaltime_as_ctime(tt));

    tt = icaltime_from_string("19970101");
    assert(!icaltime_is_null_time(tt));
    printf("%s\n",icaltime_as_ctime(tt));

}

void test_recur_parser()
{
    struct icalrecurrencetype rt; 

    rt = icalrecurrencetype_from_string("FREQ=YEARLY;UNTIL=20000131T090000Z;BYMONTH=1,2,3,4,8;BYYEARDAY=34,65,76,78;BYDAY=-1TU,3WE,-4FR,SU,SA");

    printf("%s\n",icalrecurrencetype_as_string(&rt));

    rt = icalrecurrencetype_from_string("FREQ=DAILY;COUNT=3;BYMONTH=1,2,3,4,8;BYYEARDAY=34,65,76,78;BYDAY=-1TU,3WE,-4FR,SU,SA");

    printf("%s\n",icalrecurrencetype_as_string(&rt));

}

void test_doy()
{
    struct icaltimetype tt1, tt2;
    short doy;

    tt1 = icaltime_from_string("19950301");
    doy = icaltime_day_of_year(tt1);
    tt2 = icaltime_from_day_of_year(doy,1995);
    printf("%d %s %s\n",doy, icaltime_as_ctime(tt1),icaltime_as_ctime(tt2));

    tt1 = icaltime_from_string("19960301");
    doy = icaltime_day_of_year(tt1);
    tt2 = icaltime_from_day_of_year(doy,1996);
    printf("%d %s %s\n",doy, icaltime_as_ctime(tt1),icaltime_as_ctime(tt2));

    tt1 = icaltime_from_string("19970301");
    doy = icaltime_day_of_year(tt1);
    tt2 = icaltime_from_day_of_year(doy,1997);
    printf("%d %s %s\n",doy, icaltime_as_ctime(tt1),icaltime_as_ctime(tt2));


}


void test_x(){

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
 
#include <stdio.h>
#include <stdlib.h>
#include <ical.h>
 
    static const char test_icalcomp_str[] =
"BEGIN:VEVENT\r\n"
"RRULE\r\n"
" ;X-EVOLUTION-ENDDATE=20030209T081500\r\n"
" :FREQ=DAILY;COUNT=10;INTERVAL=6\r\n"
"X-COMMENT;X-FOO=BAR: Booga\r\n"
"END:VEVENT\r\n";
  
    icalcomponent *icalcomp;
    icalproperty *prop;
    struct icalrecurrencetype recur;
    int n_errors;
    
    icalcomp = icalparser_parse_string ((char *) test_icalcomp_str);
    if (!icalcomp) {
	fprintf (stderr, "main(): could not parse the component\n");
	exit (EXIT_FAILURE);
    }
    
    printf("%s\n\n",icalcomponent_as_ical_string(icalcomp));

    n_errors = icalcomponent_count_errors (icalcomp);
    if (n_errors) {
	icalproperty *p;
	
	for (p = icalcomponent_get_first_property (icalcomp,
						   ICAL_XLICERROR_PROPERTY);
	     p;
	     p = icalcomponent_get_next_property (icalcomp,
						  ICAL_XLICERROR_PROPERTY)) {
	    char *str;
	    
	    str = icalproperty_as_ical_string (p);
	    fprintf (stderr, "error: %s\n", str);
	}
    }
    
    prop = icalcomponent_get_first_property (icalcomp, ICAL_RRULE_PROPERTY);
    if (!prop) {
	fprintf (stderr, "main(): could not get the RRULE property");
	exit (EXIT_FAILURE);
    }
    
    recur = icalproperty_get_rrule (prop);
    
    printf("%s\n",icalrecurrencetype_as_string(&recur));

}

void test_gauge_sql() {


    icalgauge *g;
    
    printf("\nSELECT DTSTART,DTEND,COMMENT FROM VEVENT,VTODO WHERE VEVENT.SUMMARY = 'Bongoa' AND SEQUENCE < 5\n");

    g = icalgauge_new_from_sql("SELECT DTSTART,DTEND,COMMENT FROM VEVENT,VTODO WHERE VEVENT.SUMMARY = 'Bongoa' AND SEQUENCE < 5");
    
    icalgauge_dump(g);

    icalgauge_free(g);

    printf("\nSELECT * FROM VEVENT,VTODO WHERE VEVENT.SUMMARY = 'Bongoa' AND SEQUENCE < 5 OR METHOD != 'CREATE'\n");

    g = icalgauge_new_from_sql("SELECT * FROM VEVENT,VTODO WHERE VEVENT.SUMMARY = 'Bongoa' AND SEQUENCE < 5 OR METHOD != 'CREATE'");
    
    icalgauge_dump(g);

    icalgauge_free(g);

}

void test_gauge_compare() {

    icalgauge *g;
    icalcomponent *c;

    /* Equality */

    c =  icalcomponent_vanew(ICAL_VCALENDAR_COMPONENT,
	      icalcomponent_vanew(ICAL_VEVENT_COMPONENT,
		  icalproperty_new_dtstart(
		      icaltime_from_string("20000101T000002")),0),0);

    g = icalgauge_new_from_sql(
	"SELECT * FROM VEVENT WHERE DTSTART = '20000101T000002'");

    printf("SELECT * FROM VEVENT WHERE DTSTART = '20000101T000002'\n");
    assert(c!=0);
    assert(g!=0);

    assert(icalgauge_compare(g,c) == 1);

    icalgauge_free(g);


    g = icalgauge_new_from_sql(
	"SELECT * FROM VEVENT WHERE DTSTART = '20000101T000001'");

    printf("SELECT * FROM VEVENT WHERE DTSTART = '20000101T000001'\n");

    assert(g!=0);
    assert(icalgauge_compare(g,c) == 0);

    icalgauge_free(g);

    g = icalgauge_new_from_sql(
	"SELECT * FROM VEVENT WHERE DTSTART != '20000101T000003'");

    printf("SELECT * FROM VEVENT WHERE DTSTART != '20000101T000003'\n");


    assert(g!=0);
    assert(icalgauge_compare(g,c) == 1);

    icalgauge_free(g);


    /* Less than */

    g = icalgauge_new_from_sql(
	"SELECT * FROM VEVENT WHERE DTSTART < '20000101T000003'");

    printf("SELECT * FROM VEVENT WHERE DTSTART < '20000101T000003'\n");

    assert(icalgauge_compare(g,c) == 1);

    assert(g!=0);
    icalgauge_free(g);

    g = icalgauge_new_from_sql(
	"SELECT * FROM VEVENT WHERE DTSTART < '20000101T000002'");

    printf("SELECT * FROM VEVENT WHERE DTSTART < '20000101T000002'\n");


    assert(g!=0);
    assert(icalgauge_compare(g,c) == 0);

    icalgauge_free(g);

    /* Greater than */

    g = icalgauge_new_from_sql(
	"SELECT * FROM VEVENT WHERE DTSTART > '20000101T000001'");

    printf("SELECT * FROM VEVENT WHERE DTSTART > '20000101T000001'\n");


    assert(g!=0);
    assert(icalgauge_compare(g,c) == 1);

    icalgauge_free(g);

    g = icalgauge_new_from_sql(
	"SELECT * FROM VEVENT WHERE DTSTART > '20000101T000002'");

    printf("SELECT * FROM VEVENT WHERE DTSTART > '20000101T000002'\n");


    assert(g!=0);
    assert(icalgauge_compare(g,c) == 0);

    icalgauge_free(g);


    /* Greater than or Equal to */

    g = icalgauge_new_from_sql(
	"SELECT * FROM VEVENT WHERE DTSTART >= '20000101T000002'");

    printf("SELECT * FROM VEVENT WHERE DTSTART >= '20000101T000002'\n");


    assert(g!=0);
    assert(icalgauge_compare(g,c) == 1);

    icalgauge_free(g);

    g = icalgauge_new_from_sql(
	"SELECT * FROM VEVENT WHERE DTSTART >= '20000101T000003'");

    printf("SELECT * FROM VEVENT WHERE DTSTART >= '20000101T000003'\n");


    assert(g!=0);
    assert(icalgauge_compare(g,c) == 0);

    icalgauge_free(g);

    /* Less than or Equal to */

    g = icalgauge_new_from_sql(
	"SELECT * FROM VEVENT WHERE DTSTART <= '20000101T000002'");

    printf("SELECT * FROM VEVENT WHERE DTSTART <= '20000101T000002'\n");


    assert(g!=0);
    assert(icalgauge_compare(g,c) == 1);

    icalgauge_free(g);

    g = icalgauge_new_from_sql(
	"SELECT * FROM VEVENT WHERE DTSTART <= '20000101T000001'");

    printf("SELECT * FROM VEVENT WHERE DTSTART <= '20000101T000001'\n");


    assert(g!=0);
    assert(icalgauge_compare(g,c) == 0);

    icalgauge_free(g);

    icalcomponent_free(c);


    /* Complex comparisions */

    c =  icalcomponent_vanew(
	ICAL_VCALENDAR_COMPONENT,
	icalproperty_new_method(ICAL_METHOD_REQUEST),
	icalcomponent_vanew(
	    ICAL_VEVENT_COMPONENT,
	    icalproperty_new_dtstart(
		icaltime_from_string("20000101T000002")),
	    icalproperty_new_comment("foo"),
	    icalcomponent_vanew(
		ICAL_VALARM_COMPONENT,
		icalproperty_new_dtstart(
		    icaltime_from_string("20000101T120000")),
		
		0),
	    0),
	0);
    

    g = icalgauge_new_from_sql(
	"SELECT * FROM VEVENT WHERE VALARM.DTSTART = '20000101T120000'");

    printf("SELECT * FROM VEVENT WHERE VALARM.DTSTART = '20000101T120000'\n");

    assert(icalgauge_compare(g,c) == 1);

    icalgauge_free(g);

    g = icalgauge_new_from_sql(
	"SELECT * FROM VEVENT WHERE COMMENT = 'foo'");

    printf("SELECT * FROM VEVENT WHERE COMMENT = 'foo'\n");

    assert(icalgauge_compare(g,c) == 1);

    icalgauge_free(g);


    g = icalgauge_new_from_sql(
	"SELECT * FROM VEVENT WHERE COMMENT = 'foo' AND  VALARM.DTSTART = '20000101T120000'");

    printf("SELECT * FROM VEVENT WHERE COMMENT = 'foo' AND  VALARM.DTSTART = '20000101T120000'\n");

    assert(icalgauge_compare(g,c) == 1);

    icalgauge_free(g);

    icalcomponent_free(c);
    
}

icalcomponent* make_component(int i){

    icalcomponent *c;

    struct icaltimetype t = icaltime_from_string("20000101T120000Z");

    t.day += i;

    icaltime_normalize(t);

    c =  icalcomponent_vanew(
	ICAL_VCALENDAR_COMPONENT,
	icalproperty_new_method(ICAL_METHOD_REQUEST),
	icalcomponent_vanew(
	    ICAL_VEVENT_COMPONENT,
	    icalproperty_new_dtstart(t),
	    0),
	0);

    assert(c != 0);

    return c;

}
void test_fileset()
{
    icalfileset *fs;
    icalcomponent *c;
    int i;
    char *path = "test_fileset.ics";
    icalgauge  *g = icalgauge_new_from_sql(
	"SELECT * FROM VEVENT WHERE DTSTART > '20000103T120000Z' AND DTSTART <= '20000106T120000Z'");


    fs = icalfileset_new(path);
    
    assert(fs != 0);

    for (i = 0; i!= 10; i++){
	c = make_component(i);
	icalfileset_add_component(fs,c);
    }

    icalfileset_commit(fs);

    printf("== No Selections \n");

    for (c = icalfileset_get_first_component(fs);
	 c != 0;
	 c = icalfileset_get_next_component(fs)){
	struct icaltimetype t = icalcomponent_get_dtstart(c);

	printf("%s\n",icaltime_as_ctime(t));
    }

    icalfileset_select(fs,g);

    printf("\n== DTSTART > '20000103T120000Z' AND DTSTART <= '20000106T120000Z' \n");

    for (c = icalfileset_get_first_component(fs);
	 c != 0;
	 c = icalfileset_get_next_component(fs)){
	struct icaltimetype t = icalcomponent_get_dtstart(c);

	printf("%s\n",icaltime_as_ctime(t));
    }

    icalfileset_free(fs);
    
    unlink(path);

}

int main(int argc, char *argv[])
{
    int c;
    extern char *optarg;
    extern int optind, optopt;
    int errflg=0;
    char* program_name = strrchr(argv[0],'/');
    int ttime=0, trecur=0,tspan=0, tmisc=0, tgauge = 0, tfile = 0;

    if(argc==1) {
	ttime = trecur = tspan = tmisc = tgauge = tfile = 1;
    }

    while ((c = getopt(argc, argv, "t:s:r:m:g:f:")) != -1) {
	switch (c) {

	    case 't': {
		ttime = atoi(optarg);
		break;
	    }

	    case 's': {
		tspan = atoi(optarg);
		break;
	    }

	    case 'r': {
		trecur = atoi(optarg);
		break;
	    }


	    case 'm': {
		tmisc = atoi(optarg);
		break;
	    }
	    

	    case 'g': {
		tgauge = atoi(optarg);
		break;
	    }

	    case 'f': {
		tfile = atoi(optarg);
		break;
	    }
	    
	    case ':': {/* Option given without an operand */
		fprintf(stderr,
			"%s: Option -%c requires an operand\n", 
			program_name,optopt);
		errflg++;
		break;
	    }
	    case '?': {
		errflg++;
	    }
	    
	}
	
    } 


    if(ttime==1 || ttime==2){
	printf("\n------------Test time parser ----------\n");
	test_time_parser();
	    
    }

    if(ttime==1 || ttime==3){
	printf("\n------------Test time----------------\n");
	test_time();
    }
	    
    if(ttime==1 || ttime==4){
	printf("\n------------Test day of year---------\n");
	test_doy();
    }
	    
	    
    if(trecur==1 || trecur==2){
	printf("\n------------Test recur parser ----------\n");
	test_recur_parser();
    }
     
    if(trecur==1 || trecur==3){
	printf("\n------------Test recur---------------\n");
	test_recur();
    }

    if(trecur==1 || trecur==4){
	printf("\n------------Test parameter bug---------\n");
	test_recur_parameter_bug();
    }


    if(tspan==1 || tspan==2){
	printf("\n------------Test FBlist------------\n");
	test_fblist();
    }

    if(tspan==1 || tspan==3){
	printf("\n------------Test Overlaps------------\n");
	test_overlaps();
    }

    if(tspan==1 || tspan==4){
	printf("\n------------Test Span----------------\n");
	test_span();
    }

    if(tgauge == 1 || tgauge == 2){
	printf("\n------------Test Gauge SQL----------------\n");
	test_gauge_sql();
    }	

    if(tgauge == 1 || tgauge == 3){
	printf("\n------------Test Gauge Compare--------------\n");
	test_gauge_compare();
    }	

    if(tfile ==1 || tfile == 2){
	printf("\n------------Test File Set--------------\n");
	test_fileset();
    }


    if(tmisc == 1 || tmisc  == 2){
	printf("\n------------Test X Props and Params--------\n");
	test_x();
    }


    if(tmisc == 1){

	printf("\n------------Test duration---------------\n");
	test_duration();
	
	
	printf("\n------------Test Convenience ------------\n");
	test_convenience();
	
	
	printf("\n------------Test classify ---------------\n");
	test_classify();
	
	
	printf("\n------------Test Iterators-----------\n");
	test_iterators();
	
	printf("\n------------Test Restriction---------------\n");
	test_restriction();
	
	printf("\n------------Test request status-------\n");
	test_requeststat();
	
	printf("\n------------Test strings---------------\n");
	test_strings();
	
	printf("\n------------Test Compare---------------\n");
	test_compare();
	
	printf("\n------------Test Values---------------\n");
	test_values();
	
	printf("\n------------Test Parameters-----------\n");
	test_parameters();

	printf("\n------------Test Properties-----------\n");
	test_properties();
	
	printf("\n------------Test Components ----------\n");
	test_components();
	
	printf("\n------------Create Components --------\n");
	create_new_component();
	
	printf("\n----- Create Components with vaargs ---\n");
	create_new_component_with_va_args();

	printf("\n------------Test Memory---------------\n");
	test_memory();
    }

    return 0;
}



