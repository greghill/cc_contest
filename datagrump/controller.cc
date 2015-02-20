#include <iostream>

#include "controller.hh"
#include "timestamp.hh"

using namespace std;

/* Default constructor */
Controller::Controller( const bool debug )
  : debug_( debug )
  , consecutive_high_delay(0)
  , consecutive_low_delay(20)
  , got_greg(false)
  , freeze_window(false)
{}

/* Get current window size, in datagrams */
unsigned int Controller::window_size( void )
{
    if (freeze_window) {
//        cerr << "window_frozen" << endl;
        return 0;
    }

    if (consecutive_low_delay > 10) {
        //cerr << "OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO" << endl;
        return 35;
    } else if (consecutive_high_delay > 1) {
        //cerr << "OOOOOOOO" << endl;
        return 8;
    } else {
        //cerr << "OOOOOOOOOOOOOOO" << endl;
        return 15;
    }
}

void Controller::greg_recieved()
{
    if (got_greg) {
 //       cerr << "unfrozen" << endl;
        freeze_window = false;
        consecutive_high_delay = 2;
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
    uint64_t rtt = timestamp_ack_received-send_timestamp_acked;
    if (rtt < 65)
        consecutive_low_delay++;
    else
        consecutive_low_delay = 0;

    if (rtt > 75)
        consecutive_high_delay++;
    else
        consecutive_high_delay = 0;
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
