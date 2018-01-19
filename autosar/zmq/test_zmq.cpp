#include <zmq.h>

#include <iostream>

int main(int argc, char** argv)
{
	void *ptr = NULL;
	
	std::cout << "zmq_ctx_new" << std::endl;
    ptr = zmq_ctx_new();
    std::cout << "zmq_ctx_new OK" << std::endl;
    
    zmq_ctx_destroy(ptr);
    
	return 0;
}
