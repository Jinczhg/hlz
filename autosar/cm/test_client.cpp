#include "Future.h"
//#include <future>

#include <iostream>

#include <unistd.h>
#include "ServiceSkeleton.h"
#include "ServiceProxy.h"
#include "PublishEvent.h"
#include "SubscribeEvent.h"
#include "Method.h"
#include "ManagementFactory.h"
#include "DataTypes.h"


int main(int argc, char** argv)
{
	ara::com::Configuration conf;
	
	ara::com::InstanceIdentifier instance("1");
	
	std::shared_ptr<ara::com::Endpoint> endpoint(new ara::com::Endpoint({{127,0,0,1}}, 8080, ara::com::TransportProtocol::tcp));
	
	ara::com::ServiceProxy::HandleType handle(1, 1, endpoint);
	
	ara::com::ServiceProxy proxy(handle);
	
	ara::com::SubscribeEvent sEvent(&proxy, 1);
	
	ara::com::Method method(&proxy, 2);
	
	proxy.Init(&conf);
	
	sEvent.Subscribe(ara::com::EventCacheUpdatePolicy::kLastN, 1);
	
	sEvent.SetReceiveHandler([](){
		std::cout << "event receive" << std::endl;
	});
	
	
	while (1)
	{
		sleep(60);
	}

	return 0;
}
