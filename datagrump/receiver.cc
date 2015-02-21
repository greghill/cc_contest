/* simple UDP receiver that acknowledges every datagram */

#include <cstdlib>
#include <iostream>

#include "socket.hh"
#include "timestamp.hh"
#include "contest_message.hh"

using namespace std;

int main( int argc, char *argv[] )
{
   /* check the command-line arguments */
  if ( argc < 1 ) { /* for sticklers */
    abort();
  }

  if ( argc != 2 ) {
    cerr << "Usage: " << argv[ 0 ] << " PORT" << endl;
    return EXIT_FAILURE;
  }

  /* create UDP socket for incoming datagrams */
  UDPSocket socket;

  /* turn on timestamps on receipt */
  socket.set_timestamps();

  //socket.set_nonblocking();

  /* "bind" the socket to the user-specified local port number */
  socket.bind( Address( "::0", argv[ 1 ] ) );

  cerr << "Listening on " << socket.local_address().to_string() << endl;

  uint64_t sequence_number = 0;
  uint64_t last_message_time = 0;
  bool sent_warning = false;
  
  ContestMessage hi("hi this is not a real message and I am just making stuff up so it is long enough");
  Address source_addr;
  bool got_first = false;


  /* Loop and acknowledge every incoming datagram back to its source */
  while ( true ) {
      std::unique_ptr<UDPSocket::received_datagram> recd;
      bool gotMsg = socket.recv(recd, false);
      if (gotMsg) {
          got_first = true;
          last_message_time = timestamp_ms();
          ContestMessage message = recd->payload;

          /* assemble the acknowledgment */
          message.transform_into_ack( sequence_number++, recd->timestamp );

          hi = message;
          source_addr = recd->source_address;
          if (sent_warning) 
          {
              sent_warning = false;
              hi.header.ack_sequence_number = uint64_t (-2);
              socket.sendto( source_addr, hi.to_string());
          }

          /* timestamp the ack just before sending */
          message.set_send_timestamp();

          /* send the ack */
          socket.sendto( recd->source_address, message.to_string() ); // deal with nonblocking
      } else {
          if (!sent_warning && got_first) {
              uint64_t wait_time = timestamp_ms() - last_message_time;
              if (wait_time > 110) {
                  sent_warning = true;
                  cerr << "110+ MS WAITING " << wait_time << endl;
                  hi.header.ack_sequence_number = uint64_t (-2);
                  socket.sendto( source_addr, hi.to_string());
              }
          }
      }
  }

  return EXIT_SUCCESS;
}
