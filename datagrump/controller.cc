#include <iostream>
#include <math.h>

#include "controller.hh"
#include "timestamp.hh"

using namespace std;

/* Default constructor */
Controller::Controller( const bool debug )
  : debug_( debug )
  , ewma(60)
  , curwindow(80)
  , lowest_owt(99999)
  , got_greg(false)
  , since_window_drop(0)
  , window_drop_at(0)
  , freeze_window(false)
  , first_time(-1)
  , consecutive_high_delay(0)
  , consecutive_low_delay(4)
  , got_warning(false)
{}

/* Get current window size, in datagrams */
unsigned int Controller::window_size( void )
{
    return curwindow/4;
}

void Controller::adjust_window(int64_t estimated_one_way_time)
{
    if (estimated_one_way_time < 30)
    {
        if (consecutive_high_delay == 1 || consecutive_high_delay == 2)
            curwindow++;

        curwindow++;
        if (consecutive_low_delay > 15)
        {
            curwindow++;
            consecutive_low_delay = 5;
        }
        consecutive_low_delay++;
    }
    else 
        consecutive_low_delay = 0;


    if (estimated_one_way_time > 33 )
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


    since_window_drop++;

    if (curwindow < 10)
        curwindow = 10;
    else if (curwindow > 400)
        curwindow = 400;
}

void Controller::greg_recieved()
{
    adjust_window(50); // a number above limit
    adjust_window(50); // twice
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

    double alpha = .05;
    ewma = alpha * est_owt + ((1-alpha) * ewma);
    adjust_window(est_owt);
    if ((timestamp_ack_received -first_time) > 36000 && (timestamp_ack_received -first_time)< 42000) {
        cerr << "out chea ";
        curwindow = 60;
    }

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
