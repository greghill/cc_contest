#include <iostream>
#include <math.h>

#include "controller.hh"
#include "timestamp.hh"

using namespace std;

/* Default constructor */
Controller::Controller( const bool debug )
  : debug_( debug )
  , ewma(60)
  , lowest_owt(99999)
  , got_greg(false)
  , freeze_window(false)
{}

/* Get current window size, in datagrams */
unsigned int Controller::window_size( void )
{
    /*
    if (freeze_window) {
        //        cerr << "window_frozen" << endl;
        return 0;
    }
    */
    int window = 300/ (ewma-20);
    if (window < 2)
        return 2;
    else  if (window > 50)
        return 50;
    else
        return window;
}

void Controller::greg_recieved()
{
    if (got_greg) {
 //       cerr << "unfrozen" << endl;
        freeze_window = false;
        ewma = 50; // reset ewma on freeze event
    } else {
        freeze_window = true;
        got_greg = true;
    }
}

/* A datagram was sent */
void Controller::datagram_was_sent( const uint64_t sequence_number,
				    /* of the sent datagram */
				    const uint64_t send_timestamp )
                                    /* in milliseconds */
{
  /* Default: take no action */
  //last_timestamp_sent = send_timestamp;

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
    double alpha = .03;
    int64_t est_owt = (20-lowest_owt) + owt;
    //cerr << "est owt " <<  est_owt << endl;
    ewma = alpha * est_owt + ((1-alpha) * ewma);
    /*
    for (int i = 0; i < ewma-20; i++)
        cerr << "|";
    cerr << endl;
    */
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
