#include <iostream>
#include <algorithm>
#include <climits>

#include "controller.hh"
#include "timestamp.hh"

using namespace std;

unsigned int the_window_size = 50;
unsigned int acks_recv = 0;
unsigned int rtt_min = UINT_MAX;
uint64_t prev_ack_recv = 0;
uint64_t prev_seq_num = 0;
double prev_bw_sample = 0;
double curr_bwe = 0;
double tau = 10;
unsigned int rtt_thresh = 200;

/* Default constructor */
Controller::Controller( const bool debug )
  : debug_( debug )
{}

/* Get current window size, in datagrams */
unsigned int Controller::window_size()
{
  if ( true ) {
    cerr << "At time " << timestamp_ms()
	 << " window size is " << the_window_size << endl;
  }

  return the_window_size;
}

/* A datagram was sent */
void Controller::datagram_was_sent( const uint64_t sequence_number,
				    /* of the sent datagram */
				    const uint64_t send_timestamp,
                                    /* in milliseconds */
				    const bool after_timeout
				    /* datagram was sent because of a timeout */ )
{
  /* Default: take no action */

  if ( true ) {
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
  /* Default: take no action */
  uint64_t rtt = timestamp_ack_received - send_timestamp_acked;
  if (rtt < rtt_min) {
    rtt_min = rtt;
  }
  acks_recv++;
  printf("rtt_min: %u\n", rtt_min);
  
  if (acks_recv == the_window_size / 2) {
    the_window_size++;
    acks_recv = 0;
  }
  

  uint64_t time_delta = timestamp_ack_received - prev_ack_recv;
  if (time_delta != 0) {
    printf("time delta %lu\n", time_delta);
    //printf("seq number acked %lu\n", sequence_number_acked);
    //printf("prev seq num %lu\n", prev_seq_num);
    double bw_sample = ((double)sequence_number_acked - prev_seq_num) / time_delta;
    //printf("bw sample %f\n", bw_sample);
    double exp_filter = 0.9;//(2 * tau - time_delta) / (2 * tau + time_delta);
    curr_bwe = exp_filter * curr_bwe + (1 - exp_filter) * (bw_sample + prev_bw_sample) / 2;
    //printf("curr bandwidth estimate: %f\n", curr_bwe);
    

    prev_seq_num = sequence_number_acked;
    prev_ack_recv = timestamp_ack_received;
    prev_bw_sample = bw_sample;
  }
  
  if (rtt > rtt_thresh) {
    the_window_size = (unsigned int)max((const double)the_window_size / 2, (const double)curr_bwe * rtt_min);
    acks_recv = 0;
  }

  if ( true ) {
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
  return 1000; /* timeout of one second */
}
