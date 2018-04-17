#include <iostream>
#include <map>
#include <utility>

#include "controller.hh"
#include "timestamp.hh"

using namespace std;

double current_window_size = 128;
map<uint64_t, pair<uint64_t, uint64_t>> packets_outstanding;

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
  packets_outstanding[sequence_number] = make_pair(send_timestamp, 0);

  if (after_timeout) current_window_size *= 0.5;

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
  static uint64_t last_sequence_number = 0; 
  bool first = true;

  static int count = current_window_size;
  static int64_t up_sum = 0;
  static int64_t down_sum = 0;

  packets_outstanding[sequence_number_acked].second = timestamp_ack_received;
  if (first) {
    last_sequence_number = sequence_number_acked;
    first = false;
  } else {
    pair<uint64_t, uint64_t> current = packets_outstanding[sequence_number_acked];
    pair<uint64_t, uint64_t> prev = packets_outstanding[last_sequence_number];

    int64_t diff_send = current.first - prev.first;
    int64_t diff_recv = current.second - prev.second;
    int64_t delay = diff_recv - diff_send;

    count--;
    if (delay <= 0) down_sum += delay;
    else up_sum += delay;

    if (count == 0) {
      if (up_sum > -down_sum) { // congested
        current_window_size *= 0.5;
      } else {
        current_window_size += 1 + (-down_sum / (this->timeout_ms() * current_window_size));
      }

      count = current_window_size;
      up_sum = 0;
      down_sum = 0;
    }

    packets_outstanding.erase(packets_outstanding.find(last_sequence_number));
    last_sequence_number = sequence_number_acked;
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
