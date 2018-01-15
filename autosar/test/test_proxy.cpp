#include <com/myCompany/radar_proxy.h>

#include <unistd.h>
#include <iostream>

int main(int argc, char** argv)
{
	ara::com::Configuration conf;
	
	ara::com::InstanceIdentifier instance("1");
	
	std::shared_ptr<ara::com::Endpoint> server1(new ara::com::Endpoint({{127,0,0,1}}, 9000, ara::com::TransportProtocol::tcp));
	std::shared_ptr<ara::com::Endpoint> client1(new ara::com::Endpoint({{127,0,0,1}}, 9001, ara::com::TransportProtocol::tcp));
	std::shared_ptr<ara::com::Endpoint> server2(new ara::com::Endpoint({{127,0,0,1}}, 9002, ara::com::TransportProtocol::tcp));
	std::shared_ptr<ara::com::Endpoint> client2(new ara::com::Endpoint({{127,0,0,1}}, 9003, ara::com::TransportProtocol::tcp));
	
	std::vector<std::shared_ptr<ara::com::Endpoint>> servers;
	std::vector<std::shared_ptr<ara::com::Endpoint>> clients;
	
	servers.push_back(server1);
	clients.push_back(client1);
	
	servers.push_back(server2);
	clients.push_back(client2);
	
	conf.setServerEndpoint(servers);
	conf.setClientEndpoint(clients);
	conf.setNetWorkBindingType(ara::com::NetWorkBindingType::IPC);
	
	ara::com::ServiceProxy::HandleType handle(1,1,server1);
	
	com::myCompany::proxy::RadarProxy proxy(handle);
	proxy.Init(&conf);
	
	proxy.BrakeEvent.SetReceiveHandler([](){
		std::cout << "event receive" << std::endl;
	});
	
	proxy.BrakeEvent.Subscribe(ara::com::EventCacheUpdatePolicy::kLastN, 1);
	
	Position configuration;
	ara::com::Future<com::myCompany::proxy::methods::Calibrate::Output> result = proxy.Calibrate(configuration);
	result.get();
	//result.wait();
	//com::myCompany::proxy::methods::Calibrate::Output output = result.get();
	//std::cout << output.result << std::endl;
		
	while (1)
	{
		//Position configuration;
		//ara::com::Future<com::myCompany::proxy::methods::Calibrate::Output> result = proxy.Calibrate(configuration);
		
		//result.wait();
		
		//com::myCompany::proxy::methods::Calibrate::Output output = result.get();
		
		//std::cout << output.result << std::endl;
		
		sleep(1);
	}
	
	return 0;
}
