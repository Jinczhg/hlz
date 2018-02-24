#include <com/myCompany/radar_proxy.h>

#include <unistd.h>
#include <iostream>
#include <signal.h>

com::myCompany::proxy::RadarProxy *g_proxy;
void terminate_handler(int sig)
{	
	std::cout << "terminate_handler" << std::endl;
	if (g_proxy)
	{
		g_proxy->BrakeEvent.Unsubscribe();
		usleep(10000);
		std::cout << "g_proxy->BrakeEvent.Unsubscribe()" << std::endl;
	}
	
	if (sig == SIGINT)
	{
		exit(0);
	}
}

int main(int argc, char** argv)
{
	signal(SIGABRT, terminate_handler);
	signal(SIGINT, terminate_handler);
	signal(SIGSEGV, terminate_handler);
	
	ara::com::InstanceIdentifier instance("1");

	ara::com::ServiceHandleContainer<ara::com::ServiceProxy::HandleType> handles = com::myCompany::proxy::RadarProxy::FindService(instance);
	
	std::cout << "RadarProxy instance" << std::endl;
	
	com::myCompany::proxy::RadarProxy proxy(handles[0]);

	g_proxy = &proxy;
	
	std::cout << "RadarProxy instance OK" << std::endl;
	
	proxy.BrakeEvent.SetReceiveHandler([&proxy](){
		proxy.BrakeEvent.Update();
		ara::com::SampleContainer<ara::com::SamplePtr<const com::myCompany::proxy::events::BrakeEvent::SampleType>> samples = proxy.BrakeEvent.GetCachedSamples();

		std::cout << "BrakeEvent receive" << std::endl;
		std::cout << "RadarObjects active = " << samples[0]->active << std::endl;
		std::cout << "RadarObjects objects = ";
		for (auto obj : samples[0]->objects)
		{
			std::cout << (char)obj;
		}
		
		std::cout << std::endl;
	});
	
	proxy.BrakeEvent.Subscribe(ara::com::EventCacheUpdatePolicy::kLastN, 1);
	
	std::thread calibrateThread([&proxy]{
		Position configuration;
		configuration.x = 0;
		configuration.y = 0;
		configuration.z = 0;
		while (1)
		{
			configuration.x += 100;
			configuration.y += 100;
			configuration.z += 100;

			std::cout << std::endl << "call Calibrate" << std::endl;
			try
			{
				ara::com::Future<com::myCompany::proxy::methods::Calibrate::Output> calibrateResult = proxy.Calibrate(configuration);
		
				com::myCompany::proxy::methods::Calibrate::Output calibrateOutput = calibrateResult.get();
		
				std::cout << "calibrateOutput.result:" << calibrateOutput.result << std::endl << std::endl << std::endl;
			}
			catch (std::exception &e)
			{
				std::cout << e.what() << std::endl;
			}

			usleep(10000);
		}
	});
	calibrateThread.detach();
	
	std::thread adjustThread([&proxy]{
		while (1)
		{
			Position configuration;
		
			configuration.x = 100;
			configuration.y = 100;
			configuration.z = 100;

			std::cout << std::endl <<  "call Adjust" << std::endl;
			try
			{
				ara::com::Future<com::myCompany::proxy::methods::Adjust::Output> adjustResult = proxy.Adjust(configuration);
		
				com::myCompany::proxy::methods::Adjust::Output adjustOutput = adjustResult.get();
		
				std::cout << "adjustOutput.success:" << adjustOutput.success << std::endl;
				std::cout << "adjustOutput.effective_position.x:" << adjustOutput.effective_position.x << std::endl;
				std::cout << "adjustOutput.effective_position.y:" << adjustOutput.effective_position.y << std::endl;
				std::cout << "adjustOutput.effective_position.z:" << adjustOutput.effective_position.z << std::endl << std::endl;
			}
			catch (std::exception &e)
			{
				std::cout << e.what() << std::endl;
			}

			usleep(10000);
		}
	});
	adjustThread.detach();
	
	while (1)
	{
		sleep(1);
	}
	
	return 0;
}
