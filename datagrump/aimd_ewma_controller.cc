#include <iostream>

#include "controller.hh"
#include "timestamp.hh"

using namespace std;

double current_window_size = 100;

/* Default constructor */
Controller::Controller( const bool debug )
  : debug_( debug )
{
  debug_ = true;
}

/* Get current window size, in datagrams */
unsigned int Controller::window_size() {
  if ( debug_ ) {
    cerr << "At time " << timestamp_ms()
	 << " window size is " << current_window_size << endl;
  }

  return (unsigned int) current_window_size;
}

/* A datagram was sent */
void Controller::datagram_was_sent( const uint64_t sequence_number,
				    /* of the sent datagram */
				    const uint64_t send_timestamp,
                                    /* in milliseconds */
				    const bool after_timeout
				    /* datagram was sent because of a timeout */ )
{
  if (after_timeout) current_window_size *= 0.75;

  if ( debug_ ) {
    cerr << "At time " << send_timestamp
	 << " sent datagram " << sequence_number << " (timeout = " << after_timeout << ")\n";
  }
}

/* An ack was received */
void Controller::ack_received( const uint64_t sequence_number_acked,
			       /* what sequence number was acknowledged */
			       const uint64_t send_timestamp_acked,
			       /* when the acknowledged datagram was sent (sender's clock) */
			       const uint64_t recv_timestamp_acked,
			       /* when the acknowledged datagram was received (receiver's clock)*/
			       const uint64_t timestamp_ack_received )
                               /* when the ack was received (by sender) */
{
  static int64_t last_rtt = 0;
  static uint64_t last_send_timestamp_acked = 0;
  static uint64_t last_timestamp_ack_received = 0;
  static bool first = true;

  static double change_ewma = 1.0;

  unsigned int rtt = timestamp_ack_received - send_timestamp_acked;
  if (first) {
    last_rtt = rtt;
    last_send_timestamp_acked = send_timestamp_acked;
    last_timestamp_ack_received = timestamp_ack_received;
    first = false;
  } else if((send_timestamp_acked != last_send_timestamp_acked || 
            timestamp_ack_received != last_timestamp_ack_received) &&
            current_window_size >= 10) {

    int64_t change = last_rtt - rtt;
    change_ewma = 0.75 * change_ewma + 0.25 * change;

    if (change_ewma >= 0) {
      current_window_size += (1 + (change_ewma / this->timeout_ms()))/current_window_size;
    } else {
      current_window_size += 1/current_window_size;
    }

    last_rtt = rtt;
    last_send_timestamp_acked = send_timestamp_acked;
    last_timestamp_ack_received = timestamp_ack_received;
  }

  if ( debug_ ) {
    cerr << "At time " << timestamp_ack_received
	 << " received ack for datagram " << sequence_number_acked
	 << " (send @ time " << send_timestamp_acked
	 << ", received @ time " << recv_timestamp_acked << " by receiver's clock)"
	 << endl;
  }
}

/* How long to wait (in milliseconds) if there are no acks
   before sending one more datagram */
unsigned int Controller::timeout_ms()
{
  return 600; /* timeout of one second */
}
