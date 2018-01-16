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
	
	std::shared_ptr<ara::com::Endpoint> server(new ara::com::Endpoint({{127,0,0,1}}, 9000, ara::com::TransportProtocol::tcp));
	std::shared_ptr<ara::com::Endpoint> client(new ara::com::Endpoint({{127,0,0,1}}, 9001, ara::com::TransportProtocol::tcp));
	std::shared_ptr<ara::com::Endpoint> mutilcast(new ara::com::Endpoint({{127,0,0,1}}, 9002, ara::com::TransportProtocol::tcp));
	
	std::vector<std::shared_ptr<ara::com::Endpoint>> servers;
	std::vector<std::shared_ptr<ara::com::Endpoint>> clients;
	
	servers.push_back(server);
	clients.push_back(client);
	
	conf.setServerEndpoint(servers);
	conf.setClientEndpoint(clients);
	conf.setMulticastEndpoint(mutilcast);
	conf.setNetWorkBindingType(ara::com::NetWorkBindingType::IPC);
	
	ara::com::ServiceProxy::HandleType handle(1, 1, server);
	
	ara::com::ServiceProxy proxy(handle);
	
	ara::com::SubscribeEvent sEvent(&proxy, 1);
	
	ara::com::Method method(&proxy, 2);
	
	proxy.Init(&conf);

#if 1
	sEvent.Subscribe(ara::com::EventCacheUpdatePolicy::kLastN, 1);
	
	sEvent.SetReceiveHandler([](){
		std::cout << "event receive" << std::endl;
	});
	
	ara::com::ServiceSkeleton skeleton(1, instance, ara::com::MethodCallProcessingMode::kEvent);
	
	ara::com::PublishEvent pEvent(&skeleton, 1);
	
	skeleton.Init(&conf);
	
	std::shared_ptr<ara::com::Payload> payload(new ara::com::Payload(strlen("hello world")+1, (uint8_t*)"hello world"));
	
	ara::com::ManagementFactory::get()->getServiceProvider(1,1)->setRequestReceiveHandler(2, [](std::shared_ptr<ara::com::Message> msg){
		ara::com::ManagementFactory::get()->getServiceProvider(1,1)->response(2, msg->getId() | (msg->getSession() << 16), msg->getPayload());
	});
	
	pEvent.Send(payload);
	
	method(payload, [](std::shared_ptr<ara::com::Payload> payload){
		std::cout << "method result:" << payload->getData() << std::endl;
	});
#endif	
	sleep(10);

	return 0;
}
