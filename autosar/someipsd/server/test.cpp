#include "SdServer.h"

#include <unistd.h>

int main(int argc, char **argv)
{
	ara::com::SdServer *sd = ara::com::SdServer::get();
	
	sleep(10);
	
	return 0;
}
