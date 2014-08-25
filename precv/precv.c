#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>

#include <proton/driver.h>
#include <proton/message.h>





#define MY_BUF_SIZE  1000





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
get_sasl_over_with ( pn_connector_t * connector )
{
  pn_sasl_t *sasl = pn_connector_sasl ( connector );

  while ( pn_sasl_state(sasl) != PN_SASL_PASS ) 
  {
    switch ( pn_sasl_state(sasl)) 
    {
      case PN_SASL_IDLE:
        return false;
      break;

      case PN_SASL_CONF:
        pn_sasl_mechanisms(sasl, "PLAIN ANONYMOUS");
        pn_sasl_server(sasl);
      break;

      case PN_SASL_STEP:
      {
        char response [ MY_BUF_SIZE ];
        pn_sasl_recv(sasl, response, pn_sasl_pending(sasl) );
        pn_sasl_done(sasl, PN_SASL_OK);
        pn_connector_set_connection(connector, pn_connection());
      }
      break;

      case PN_SASL_PASS:
      break;

      case PN_SASL_FAIL:
        return false;
      break;
    }
  }
  
  return true;
}





int 
main ( int argc, char ** argv )
{
  char info[1000];
  int  expected = (argc > 1) ? atoi(argv[1]) : 100000;
  int  received = 0;
  int  size     = 32;
  int  msg_size = 50;
  bool done     = false;
  int  initial_credit   = 500,
       new_credit       = 250,
       low_credit_limit = 250;

  char const * host = "0.0.0.0";
  char const * port = "5672";

  bool closed    = false;
  bool sasl_done = false;

  pn_driver_t     * driver;
  pn_listener_t   * listener;
  pn_connector_t  * connector;
  pn_connection_t * connection;
  pn_session_t    * session;
  pn_link_t       * link;
  pn_delivery_t   * delivery;

  /*------------------------------------------------
    The message data is what I actually send, after
    Proton gets done messing with it.
  ------------------------------------------------*/
  char * message_data          = (char *) malloc ( MY_BUF_SIZE );
  int    message_data_capacity = MY_BUF_SIZE;


  fprintf ( stderr, "drecv expecting %d messages.\n", expected );
  driver = pn_driver ( );

  if ( ! pn_listener(driver, host, port, 0) ) 
  {
    fprintf ( stderr, "listener creation failed.\n" );
    exit ( 1 );
  }

  while ( (! done) && (! closed) ) 
  {
    pn_driver_wait ( driver, -1 );

    while ( (listener = pn_driver_listener(driver)) ) 
      pn_listener_accept( listener );

    while ( (connector = pn_driver_connector(driver)) ) 
    {
      pn_connector_process ( connector );

      if ( ! sasl_done )
        if( ! (sasl_done = get_sasl_over_with(connector) ))
          continue;

      connection = pn_connector_connection ( connector );


      /*=========================================================
        Open everything that is ready on the 
        other side but not here.
      =========================================================*/
      pn_state_t hes_ready_im_not = PN_LOCAL_UNINIT | PN_REMOTE_ACTIVE;

      if (pn_connection_state(connection) == hes_ready_im_not)
        pn_connection_open( connection);


      for ( session = pn_session_head(connection, hes_ready_im_not);
            session;
            session = pn_session_next(session, hes_ready_im_not)
          )
        pn_session_open(session);


      for ( link = pn_link_head(connection, hes_ready_im_not);
            link;
            link = pn_link_next(link, hes_ready_im_not)
          )
       {
         pn_terminus_copy(pn_link_source(link), pn_link_remote_source(link));
         pn_terminus_copy(pn_link_target(link), pn_link_remote_target(link));
         pn_link_open ( link );
         if ( pn_link_is_receiver(link) ) 
           pn_link_flow ( link, initial_credit );
       }


      /*==========================================================
        Get all available deliveries.
      ==========================================================*/
      for ( delivery = pn_work_head ( connection );
            delivery;
            delivery = pn_work_next ( delivery )
          )
      {
        if ( pn_delivery_readable(delivery) ) 
        {
          link = pn_delivery_link ( delivery );
          while ( PN_EOS != pn_link_recv(link, message_data, MY_BUF_SIZE) )
            ;
          pn_link_advance ( link );
          pn_delivery_update ( delivery, PN_ACCEPTED );
          pn_delivery_settle ( delivery );


          if ( ++ received >= expected )
          {
            sprintf ( info, "received %d messages", received );
            print_timestamp ( stderr, info );
            done = true;
          }

          if ( ! (received % 5000000) )
            fprintf ( stderr, "received: %d\n", received );


          if ( pn_link_credit(link) <= low_credit_limit ) 
            pn_link_flow ( link, new_credit );
        } 
        else
        {
          // TODO
          // Why am I getting writables?
          // And what to do with them?
        }
      } 


      /*===============================================================
        Shut down everything that the other side has closed.
      ===============================================================*/
      pn_state_t active_here_closed_there = PN_LOCAL_ACTIVE | PN_REMOTE_CLOSED;

      if ( pn_connection_state(connection) == active_here_closed_there )
        pn_connection_close ( connection );

      for ( session = pn_session_head(connection, active_here_closed_there);
            session;
            session = pn_session_next(session, active_here_closed_there)
          )
        pn_session_close ( session );

      for ( link = pn_link_head(connection, active_here_closed_there);
            link;
            link = pn_link_next(link, active_here_closed_there)
          )
        pn_link_close ( link );

      if ( pn_connector_closed(connector) ) 
      {
        pn_connection_free ( pn_connector_connection(connector) );
        pn_connector_free ( connector );
        closed = true;
      } 
      else 
        pn_connector_process(connector);
    }
  }

  pn_driver_free(driver);

  return 0;
}





