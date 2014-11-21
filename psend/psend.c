/*
*
* This code is your code, this code is my code,
* from California to the Forsyth Islands, from the
* Czech Republic, to Tierra del Fuego, this code
* was made for you and me.
*
* Even if prohibited by applicable law or agreed to
* in writing, software distributed under this 'License'
* is distributed on an "OH WHATEVER" BASIS, WITH
* WARRANTIES AND CONDITIONS OF ALL SORTS  both expressed
* and implied: to wit, i.e., e.g., and in lieu:
* this software is guaranteed to make you more
* intelligent, wittier, more attractive to members of
* the opposite sex, er, that is, to... well, to whomever
* you like, actually.  It will make you Too Rich,
* and Too Thin.  It will reverse hair-loss, gout, rickets,
* and flatulence.  ( Think about that one for a moment. )
* Pass it on!  Give it to your friends and enemies!  I knew
* a guy who tried to sell this code after deleting this
* 'License' and all his teeth fell out!
*
*/


#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>

#include <proton/connection.h>
#include <proton/delivery.h>
#include <proton/driver.h>
#include <proton/event.h>
#include <proton/terminus.h>
#include <proton/link.h>
#include <proton/message.h>
#include <proton/session.h>


#define MY_BUF_SIZE 1000



void
print_timestamp ( FILE * fp, char const * label )
{
  struct timeval tv;
  struct tm      * timeinfo;

  gettimeofday ( & tv, 0 );
  timeinfo = localtime ( & tv.tv_sec );

  int seconds_today = 3600 * timeinfo->tm_hour +
                        60 * timeinfo->tm_min  +
                             timeinfo->tm_sec;

  fprintf ( fp, "time : %d.%.6ld : %s\n", seconds_today, tv.tv_usec, label );
}





void
print_data ( FILE * fp, char const * str, int len )
{
  fputc ( '|', fp );
  int i;
  for ( i = 0; i < len; ++ i )
  {
    unsigned char c = * str ++;
    if ( isprint(c) )
      fputc ( c, fp );
    else
      fprintf ( fp, "\\0x%.2X", c );
  }
  fputs ( "|\n", fp );
}





bool
get_sasl_over_with ( pn_connector_t * driver_connector )
{
  pn_sasl_t * sasl = pn_connector_sasl ( driver_connector );

  while ( pn_sasl_state(sasl) != PN_SASL_PASS ) 
  {
    pn_sasl_state_t state = pn_sasl_state ( sasl );
    switch ( state ) 
    {
      case PN_SASL_IDLE:
        return false;
        break;

      case PN_SASL_CONF:
        pn_sasl_mechanisms ( sasl, "ANONYMOUS" );
        pn_sasl_client ( sasl );
        break;

      case PN_SASL_STEP:
        if ( pn_sasl_pending(sasl) ) 
        {
          fprintf ( stderr, "challenge failed\n" );
          exit ( 1 );
        }
        break;

      case PN_SASL_FAIL:
        fprintf ( stderr, "authentication failed\n" );
        exit ( 1 );
        break;

      case PN_SASL_PASS:
        break;
    }
  }

  pn_sasl_done ( sasl, PN_SASL_OK );
  return true;
}


static 
double
get_time ( )
{
  struct timeval tv;
  struct tm      * timeinfo;

  gettimeofday ( & tv, 0 );
  timeinfo = localtime ( & tv.tv_sec );

  double time_now = 3600 * timeinfo->tm_hour +
                      60 * timeinfo->tm_min  +
                           timeinfo->tm_sec;

  time_now += ((double)(tv.tv_usec) / 1000000.0);
  return time_now;
}





int 
main ( )
{
  char const * addr = "queue";

  char const * host = "10.16.44.238";
  char const * port = "5671";

  uint64_t to_send_count = 1000 * 1000 * 1000;  
  to_send_count *= 10;

  uint64_t  delivery_count = 0;
  int  size           = 32;
  bool done           = false;
  bool sasl_done      = false;
  int  msg_size       = 50;
  int  sent_count     = 0;
  char str [ 100 ];
  int const n_links = 5;


  pn_driver_t     * driver;
  pn_connector_t  * connector;
  pn_connector_t  * driver_connector;
  pn_connection_t * connection;
  pn_collector_t  * collector;
  pn_link_t       * links [ n_links ];
  pn_session_t    * session;
  pn_event_t      * event;
  pn_delivery_t   * delivery;


  int const message_length = 600;
  char * message = (char *) malloc(message_length);
  memset ( message, 13, message_length );

  /*----------------------------------------------------
    Get everything set up.
    We will have a single connector, a single 
    connection, a single session, and a single link.
  ----------------------------------------------------*/
  driver = pn_driver ( );
  connector = pn_connector ( driver, host, port, 0 );
  bool result = get_sasl_over_with ( connector );

  connection = pn_connection();
  collector  = pn_collector  ( );
  pn_connection_collect ( connection, collector );
  pn_connector_set_connection ( connector, connection );

  session = pn_session ( connection );
  pn_connection_open ( connection );
  pn_session_open ( session );

  for ( int i = 0; i < n_links; ++ i )
  {
    char name[100];
    sprintf ( name, "tvc_15_%d", i );
    links[i] = pn_sender ( session, name );
    pn_terminus_set_address ( pn_link_target(links[i]), addr );
    pn_link_open ( links[i] );
  }


  /*-----------------------------------------------------------
    For my speed tests, I do not want to count setup time.
    Start timing here.  The receiver will print out a similar
    timestamp when he receives the final message.
  -----------------------------------------------------------*/
  fprintf ( stderr, "psend start: sending %llu messages.\n", to_send_count );


  while ( 1 )
  {
    pn_driver_wait ( driver, -1 );

    int event_count = 1;
    while ( event_count > 0 )
    {
      event_count = 0;
      pn_connector_process ( connector );

      event = pn_collector_peek(collector);
      while ( event )
      {
        ++ event_count;
        pn_event_type_t event_type = pn_event_type ( event );
        //fprintf ( stderr, "event: %s\n", pn_event_type_name ( event_type ) );

        switch ( event_type )
        {
          case PN_LINK_FLOW:
          {
            pn_link_t * link = pn_event_link ( event );
            int credit = pn_link_credit ( link );

            while ( credit > 0 )
            {
              sprintf ( str, "%x", delivery_count ++ );
              delivery = pn_delivery ( link, pn_dtag(str, strlen(str)) );
              pn_delivery_settle ( delivery );
              pn_link_send ( link, message, message_length );
              pn_link_advance ( link );
              credit = pn_link_credit ( link );

              if ( delivery_count >= to_send_count )
              {
                fprintf ( stdout, "stop_time: %.3lf\n", get_time() );
                goto all_done;
              }
            }
          }
          break;

          default:
          break;
        }

        pn_collector_pop ( collector );
        event = pn_collector_peek(collector);
      }
    }
  }

  all_done:

  return 0;
}





