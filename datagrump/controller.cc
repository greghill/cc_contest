#include <iostream>

#include "controller.hh"

using namespace std;

/* Default constructor */
Controller::Controller( const bool debug )
  : debug_( debug )
  , curwindow(10)
  , lowest_owt(99999)
  , lowest_rtt(99999)
  , first_time(-1)
  , consecutive_high_delay(0)
  , consecutive_low_delay(4)
{}

/* Get current window size, in datagrams */
unsigned int Controller::window_size( void )
{
    return curwindow;
}

/* A datagram was sent */
void Controller::datagram_was_sent( const uint64_t sequence_number,
				    /* of the sent datagram */
				    const uint64_t send_timestamp )
                                    /* in milliseconds */
{
    if (first_time == uint64_t(-1))
        first_time = send_timestamp;

  if ( debug_ ) {
    cerr << "At time " << send_timestamp
	 << " sent datagram " << sequence_number << endl;
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
    int64_t owt =  (int64_t) recv_timestamp_acked - (int64_t) send_timestamp_acked;
    if (owt < lowest_owt)
        lowest_owt = owt;

    int64_t rtt =  (int64_t) timestamp_ack_received - (int64_t) send_timestamp_acked;
    if (rtt < lowest_rtt)
        lowest_rtt = rtt;

    int64_t est_owt = ((lowest_rtt/2)-lowest_owt) + owt;

    if (est_owt > 33 )
        curwindow -= .25;
    else if (est_owt < 30)
        curwindow += .25;

    if (curwindow < 2)
        curwindow = 2;
    else if (curwindow > 100)
        curwindow = 100;

  if ( debug_ ) {
    cerr << "At time " << timestamp_ack_received
	 << " received ack for datagram " << sequence_number_acked
	 << " (send @ time " << send_timestamp_acked
	 << ", received @ time " << recv_timestamp_acked << " by receiver's clock)"
    << " est_owt is! " << est_owt
	 << endl;
  }
}

/* How long to wait (in milliseconds) if there are no acks
   before sending one more datagram */
unsigned int Controller::timeout_ms( void )
{
  return 80; /* timeout of one second */
}
