#include <iostream>

#include "controller.hh"
#include "timestamp.hh"

using namespace std;

/* Default constructor */
Controller::Controller( const bool debug )
  : debug_( debug )
  , high_delay(false)
  , was_high_delay(false)
  , consecutive_low_delay(20)
{}

/* Get current window size, in datagrams */
unsigned int Controller::window_size( void )
{
    /*
    if (consecutive_low_delay > 10)
        return 30;
    else if (high_delay && was_high_delay)
        return 8;
    else
        return 15;
        */
        return 1;
}

/* A datagram was sent */
void Controller::datagram_was_sent( const uint64_t sequence_number,
				    /* of the sent datagram */
				    const uint64_t send_timestamp )
                                    /* in milliseconds */
{
  /* Default: take no action */

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
    uint64_t rtt = timestamp_ack_received-send_timestamp_acked;
    if (rtt < 60)
        consecutive_low_delay++;
    else
        consecutive_low_delay = 0;

    was_high_delay = high_delay;
    high_delay = rtt > 75;
    //cerr << "Ack for datagram " << sequence_number_acked //<< " with 1 way time " << send_timestamp_acked-recv_timestamp_acked
	 //<< ", rtt time " << timestamp_ack_received-send_timestamp_acked << " and smallwindow=" << (high_delay && was_high_delay) << endl;
//   cerr << (consecutive_low_delay > 10) << " and rtt time is " << timestamp_ack_received-send_timestamp_acked << endl;
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
unsigned int Controller::timeout_ms( void )
{
  return 1000; /* timeout of one second */
}
