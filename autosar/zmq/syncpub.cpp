//
//  Synchronized publisher in C++
//
// Olivier Chamoux <olivier.chamoux@fr.thalesgroup.com>


#include "zhelpers.hpp"

//  We wait for 10 subscribers
#define SUBSCRIBERS_EXPECTED  10

int main () {
	zmq::context_t context(1);

    //  Socket to talk to clients
    zmq::socket_t publisher (context, ZMQ_PUB);

    int sndhwm = 0;
    publisher.setsockopt (ZMQ_SNDHWM, &sndhwm, sizeof (sndhwm));

    publisher.bind("tcp://*:5561");

#if 0
    //  Socket to receive signals
    zmq::socket_t syncservice (context, ZMQ_REP);
    syncservice.bind("tcp://*:5562");

    //  Get synchronization from subscribers
    int subscribers = 0;
    while (subscribers < SUBSCRIBERS_EXPECTED) {
        
		//  - wait for synchronization request
		s_recv (syncservice);
       
		//  - send synchronization reply
		s_send (syncservice, "");


        subscribers++;
    }
 #endif
    
    //  Now broadcast exactly 1M updates followed by END
    while (1) {	
		s_send (publisher, "Rhubarb");
		sleep (1);
	}

    sleep (1);              //  Give 0MQ time to flush output
    return 0;
}
