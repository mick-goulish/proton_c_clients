
/*
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
 */


#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <proton/connection.h>
#include <proton/delivery.h>
#include <proton/driver.h>
#include <proton/event.h>
#include <proton/terminus.h>
#include <proton/link.h>
#include <proton/message.h>
#include <proton/session.h>





#define MY_BUF_SIZE  1000




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





void
print_timestamp_like_a_normal_person ( FILE * fp )
{
  char const * month_abbrevs[] = { "jan", 
                                   "feb", 
                                   "mar", 
                                   "apr", 
                                   "may", 
                                   "jun", 
                                   "jul", 
                                   "aug", 
                                   "sep", 
                                   "oct", 
                                   "nov", 
                                   "dec" 
                                 };
  time_t rawtime;
  struct tm * timeinfo;

  time ( & rawtime );
  timeinfo = localtime ( &rawtime );

  char time_string[100];
  sprintf ( time_string,
            "%d-%s-%02d %02d:%02d:%02d",
            1900 + timeinfo->tm_year,
            month_abbrevs[timeinfo->tm_mon],
            timeinfo->tm_mday,
            timeinfo->tm_hour,
            timeinfo->tm_min,
            timeinfo->tm_sec
          );

  fprintf ( fp, "|%s|\n", time_string );
}





int 
main (  )
{
  char info[1000];

  uint64_t received = 0;
  int      size     = 32;
  int      msg_size = 50;

  char const * host = "10.16.44.238";
  char const * port = "5671";

  int const initial_flow   = 400;
  int const flow_increment = 200;



  pn_driver_t     * driver;
  pn_listener_t   * listener;
  pn_connector_t  * connector;
  pn_connection_t * connection;
  pn_collector_t  * collector;
  pn_transport_t  * transport;
  pn_sasl_t       * sasl;
  pn_session_t    * session;
  pn_event_t      * event;
  pn_link_t       * link;
  pn_delivery_t   * delivery;

  int const report_frequency = 5000000;

  uint64_t delivery_count      = 0,
           expected_deliveries = 1000 * 1000 * 1000;

  expected_deliveries *= 10;


  double last_time,
         this_time,
         time_diff;

  char * message_data          = (char *) malloc ( MY_BUF_SIZE );
  int    message_data_capacity = MY_BUF_SIZE;

  int credit = 0;

  FILE * output_fp;


  output_fp = fopen ( "./precv.output", "w" );
  //output_fp = stderr;


  driver = pn_driver ( );

  if ( ! pn_listener(driver, host, port, 0) ) 
  {
    fprintf ( output_fp, "precv listener creation failed.\n" );
    exit ( 1 );
  }

  while ( 1 )
  {
    pn_driver_wait ( driver, -1 );
    if ( listener = pn_driver_listener(driver) )
    {
      if ( connector = pn_listener_accept(listener) )
        break;
    }
  }

  connection = pn_connection ( );


  collector  = pn_collector  ( );
  pn_connection_collect ( connection, collector );
  pn_connector_set_connection ( connector, connection );

  transport = pn_connector_transport ( connector );
  sasl = pn_sasl ( transport );
  pn_sasl_mechanisms ( sasl, "ANONYMOUS" );
  pn_sasl_server ( sasl );
  pn_sasl_allow_skip ( sasl, true );
  pn_sasl_done ( sasl, PN_SASL_OK );


  last_time = get_time();

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
        //fprintf ( stderr, "MDEBUG precv event: %s\n", pn_event_type_name(event_type));

        switch ( event_type )
        {
          case PN_CONNECTION_REMOTE_OPEN:
          break;


          case PN_SESSION_REMOTE_OPEN:
            session = pn_event_session(event);
            if ( pn_session_state(session) & PN_LOCAL_UNINIT ) 
            {
              // big number because it's in bytes.
              pn_session_set_incoming_capacity ( session, 1000000 );
              pn_session_open ( session );
            }
          break;


          case PN_LINK_REMOTE_OPEN:
            link = pn_event_link(event);
            if (pn_link_state(link) & PN_LOCAL_UNINIT )
            {
              pn_link_open ( link );
              pn_link_flow ( link, initial_flow );
            }
          break;


          case PN_CONNECTION_BOUND:
            if ( pn_connection_state(connection) & PN_LOCAL_UNINIT )
            {
              pn_connection_open ( connection );
            }
          break;


          case PN_DELIVERY:
            link = pn_event_link ( event );
            ++ delivery_count; 

            delivery = pn_event_delivery ( event );

            if ( pn_delivery_readable ( delivery ) )
            {
              if ( ! (delivery_count % report_frequency) )
              {
                char incoming [ 1000 ];
                pn_link_t * delivery_link = pn_delivery_link ( delivery );
                int received_bytes = pn_delivery_pending ( delivery );
                pn_link_recv ( delivery_link, incoming, 1000 );
                fprintf ( stderr, "MDEBUG received bytes: %d\n", received_bytes );
              }

              // don't bother updating.  they're pre-settled.
              // pn_delivery_update ( delivery, PN_ACCEPTED );
              pn_delivery_settle ( delivery );

              credit = pn_link_credit ( link );

              if ( delivery_count >= expected_deliveries )
              {
                fprintf ( output_fp, "Got %d deliveries.\n", delivery_count );
                goto all_done;
              }

              if ( ! (delivery_count % report_frequency) )
              {
                this_time = get_time();
                time_diff = this_time - last_time;

                print_timestamp_like_a_normal_person ( output_fp );
                fprintf ( output_fp, 
                          "deliveries: %" PRIu64 "  time: %.3lf\n", 
                          delivery_count,
                          time_diff
                        );
                fflush ( output_fp );
                last_time = this_time;
              }

              if ( credit <= flow_increment )
              {
                pn_link_flow ( link, flow_increment );
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
  
  fclose ( output_fp );
  return 0;
}





