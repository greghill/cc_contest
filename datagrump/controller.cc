#include <iostream>
#include <math.h>

#include "controller.hh"
#include "timestamp.hh"

using namespace std;

/* Default constructor */
Controller::Controller( const bool debug )
  : debug_( debug )
  , curwindow(80)
  , lowest_owt(99999)
  , first_time(-1)
  , consecutive_high_delay(0)
  , consecutive_low_delay(4)
{}

/* Get current window size, in datagrams */
unsigned int Controller::window_size( void )
{
    return curwindow/4;
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
    int64_t est_owt = (20-lowest_owt) + owt;
    if (est_owt > 33 )
    {
        if (consecutive_high_delay > 35)
        {
            //cerr << "too many, not decrementing" << endl;
            //curwindow++;
            consecutive_high_delay = 15;
        }
        else
            curwindow--;
        consecutive_high_delay++;
    }
    else
        consecutive_high_delay=0;

    if (est_owt < 30)
    {
        curwindow++;
        if (consecutive_low_delay > 20)
        {
            curwindow++;
        }
        consecutive_low_delay++;
    }
    else 
        consecutive_low_delay = 0;

    if (curwindow < 8)
        curwindow = 8;
    else if (curwindow > 400)
        curwindow = 400;

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
