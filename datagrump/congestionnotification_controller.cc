#include <iostream>
#include <map>
#include <utility>

#include "controller.hh"
#include "timestamp.hh"

using namespace std;

double current_window_size = 128;
double target = 2 * current_window_size;

int counter = 0;
int cycle_counter = 0;
bool recovery = false;
bool active = true;

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
  if (after_timeout) {
    target = current_window_size;
    current_window_size *= 0.25;
    recovery = true;
    active = false;
  }

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
  counter++;

  if (recovery) {
    if (counter == 25) {
      current_window_size = 0.5 * (current_window_size + target);
      counter = 0;       
      cycle_counter++;
    }
    
    if (cycle_counter == 3) {
      counter = 0;
      cycle_counter = 0;
      recovery = false;
      active = true;
    }
  } else if (active) {
    if ((int64_t) timestamp_ack_received - (int64_t) send_timestamp_acked > 250) {
      target = current_window_size;
      current_window_size *= 0.5;
      counter = 0; cycle_counter = 0;
      recovery = true;
      active = false;
    } else if (counter == 250) {
      target += 32;
      current_window_size = 0.5 * (current_window_size + target);
      counter = 0;
    } 
  }

  if ( debug_ ) {
    cerr << "At time " << timestamp_ack_received
	 << " received ack for datagram " << sequence_number_acked
	 << " (send @ time " << send_timestamp_acked
	 << ", received @ time " << recv_timestamp_acked << " by receiver's clock)"
     << "  in mode recovery = " << recovery
	 << endl;
  }
}

/* How long to wait (in milliseconds) if there are no acks
   before sending one more datagram */
unsigned int Controller::timeout_ms()
{
  return 1000;
}
