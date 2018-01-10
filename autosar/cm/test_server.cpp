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
	
	ara::com::ServiceSkeleton skeleton(1, instance, ara::com::MethodCallProcessingMode::kEvent);
	
	ara::com::PublishEvent pEvent(&skeleton, 1);
	
	skeleton.Init(&conf);
	
	std::shared_ptr<ara::com::Payload> payload(new ara::com::Payload(strlen("hello world")+1, (uint8_t*)"hello world"));
	
	ara::com::ManagementFactory::get()->getServiceProvider(1,1)->setRequestReceiveHandler(2, [](std::shared_ptr<ara::com::Message> msg){
		ara::com::ManagementFactory::get()->getServiceProvider(1,1)->response(2, msg->getId() | (msg->getSession() << 16), msg->getPayload());
	});
	
	sleep(1);
	
	pEvent.Send(payload);
	
	while (1)
	{
		sleep(1);
		//pEvent.Send(payload);
	}

	return 0;
}
