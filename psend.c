#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <proton/driver.h>
#include <proton/message.h>

#include "common.h"

bool get_sasl_over_with(pn_connector_t *driver_connector) {
  pn_sasl_t *sasl = pn_connector_sasl(driver_connector);

  while (pn_sasl_state(sasl) != PN_SASL_PASS) {
    pn_sasl_state_t state = pn_sasl_state(sasl);
    switch (state) {
    case PN_SASL_IDLE:
      return false;

    case PN_SASL_CONF:
      pn_sasl_mechanisms(sasl, "ANONYMOUS");
      pn_sasl_client(sasl);
      break;

    case PN_SASL_STEP:
      if (pn_sasl_pending(sasl)) {
        fprintf(stderr, "challenge failed\n");
        exit(1);
      }
      break;

    case PN_SASL_FAIL:
      fprintf(stderr, "authentication failed\n");
      exit(1);

    case PN_SASL_PASS:
      break;
    }
  }

  return true;
}

int main(int argc, char **argv) {
  char const *addr = "queue";
  char const *host = "0.0.0.0";
  char const *port = "5672";

  int send_count = (argc > 1) ? atoi(argv[1]) : 100000;
  int delivery_count = 0;
  int msg_size = 50;
  char str[100];

  /*------------------------------------------------
    The message content is what I want to send.
    ------------------------------------------------*/
  char *message_content = (char *) malloc(MY_BUF_SIZE);
  memcpy(message_content, "Hello, Receiver!", 16);

  /*------------------------------------------------
    The message data is what I actually send, after
    Proton gets done messing with it.
    ------------------------------------------------*/
  char *message_data = (char *) malloc(MY_BUF_SIZE);
  size_t data_size =
    pn_message_data(message_data, MY_BUF_SIZE, message_content, msg_size);

  /*----------------------------------------------------
    Get everything set up.
    We will have a single connector, a single 
    connection, a single session, and a single link.
    ----------------------------------------------------*/
  pn_driver_t *driver = pn_driver();
  pn_connector_t *connector = pn_connector(driver, host, port, 0);
  get_sasl_over_with(connector);
  pn_connection_t *connection = pn_connection();
  pn_connector_set_connection(connector, connection);
  pn_session_t *session = pn_session(connection);
  pn_connection_open(connection);
  pn_session_open(session);
  pn_link_t *send_link = pn_sender(session, "sender");
  pn_terminus_set_address(pn_link_target(send_link), addr);
  pn_link_open(send_link);

  /*-----------------------------------------------------------
    For my speed tests, I do not want to count setup time.
    Start timing here.  The receiver will print out a similar
    timestamp when he receives the final message.
    -----------------------------------------------------------*/
  sprintf(str, "client start: sending %d messages", send_count);
  print_timestamp(stderr, str);

  bool done = false;
  while (!done) {
    sprintf(str, "%x", delivery_count);
    pn_delivery(send_link, pn_dtag(str, strlen(str)));
    ++delivery_count;

    pn_driver_wait(driver, -1);

    pn_connector_t *driver_connector;
    while ((driver_connector = pn_driver_connector(driver))) {
      pn_connector_process(driver_connector);
      if (pn_connector_closed(driver_connector)) {
        done = true;
      }

      connection = pn_connector_connection(driver_connector);

      for (pn_delivery_t *delivery = pn_work_head(connection);
           delivery; delivery = pn_work_next(delivery)) {
        pn_link_t *link = pn_delivery_link(delivery);

        if (pn_delivery_writable(delivery)) {
          // Send pre-settled message.
          pn_delivery_settle(delivery);
          pn_link_send(link, message_data, data_size);
          pn_link_advance(link);

          if (--send_count <= 0) {
            pn_link_close(link);
            pn_connection_close(connection);
            break;
          }
        }
      }

      if (pn_connector_closed(driver_connector)) {
        pn_connection_free(pn_connector_connection(driver_connector));
        pn_connector_free(driver_connector);
        done = true;
      } else {
        pn_connector_process(driver_connector);
      }
    }
  }

  pn_driver_free(driver);

  return 0;
}
