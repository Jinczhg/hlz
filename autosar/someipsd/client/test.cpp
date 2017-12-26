#include "SdClient.h"

#include <unistd.h>

int main(int argc, char **argv)
{
	ara::com::SdClient *sd = ara::com::SdClient::get();
	
	sleep(10);
	
	return 0;
}
