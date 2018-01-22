#include <com/myCompany/radar_proxy.h>

#include <unistd.h>
#include <iostream>

int main(int argc, char** argv)
{
	ara::com::InstanceIdentifier instance("1");

	ara::com::ServiceHandleContainer<ara::com::ServiceProxy::HandleType> handles = com::myCompany::proxy::RadarProxy::FindService(instance);
	
	com::myCompany::proxy::RadarProxy proxy(handles[0]);
	
	proxy.BrakeEvent.SetReceiveHandler([&proxy](){
		std::cout << "BrakeEvent receive" << std::endl;
		proxy.BrakeEvent.Update();

		ara::com::SampleContainer<ara::com::SamplePtr<const com::myCompany::proxy::events::BrakeEvent::SampleType>> samples = proxy.BrakeEvent.GetCachedSamples();

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
			ara::com::Future<com::myCompany::proxy::methods::Calibrate::Output> calibrateResult = proxy.Calibrate(configuration);
		
			com::myCompany::proxy::methods::Calibrate::Output calibrateOutput = calibrateResult.get();
		
			std::cout << "calibrateOutput.result:" << calibrateOutput.result << std::endl << std::endl << std::endl;
		
			sleep(1);
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
			ara::com::Future<com::myCompany::proxy::methods::Adjust::Output> adjustResult = proxy.Adjust(configuration);
		
			com::myCompany::proxy::methods::Adjust::Output adjustOutput = adjustResult.get();
		
			std::cout << "adjustOutput.success:" << adjustOutput.success << std::endl;
			std::cout << "adjustOutput.effective_position.x:" << adjustOutput.effective_position.x << std::endl;
			std::cout << "adjustOutput.effective_position.y:" << adjustOutput.effective_position.y << std::endl;
			std::cout << "adjustOutput.effective_position.z:" << adjustOutput.effective_position.z << std::endl << std::endl;
		
			sleep(1);
		}
	});
	adjustThread.detach();
		
	while (1)
	{
	#if 0
		Position configuration;
		
		configuration.x = 100;
		configuration.y = 200;
		configuration.z = 300;

		std::cout << "call Calibrate" << std::endl;
		ara::com::Future<com::myCompany::proxy::methods::Calibrate::Output> calibrateResult = proxy.Calibrate(configuration);
		
		com::myCompany::proxy::methods::Calibrate::Output calibrateOutput = calibrateResult.get();
		
		std::cout << "calibrateOutput.result:" << calibrateOutput.result << std::endl << std::endl;

		std::cout << "call Adjust" << std::endl;
		ara::com::Future<com::myCompany::proxy::methods::Adjust::Output> adjustResult = proxy.Adjust(configuration);
		
		com::myCompany::proxy::methods::Adjust::Output adjustOutput = adjustResult.get();
		
		std::cout << "adjustOutput.success:" << adjustOutput.success << std::endl;
		std::cout << "adjustOutput.effective_position.x:" << adjustOutput.effective_position.x << std::endl;
		std::cout << "adjustOutput.effective_position.y:" << adjustOutput.effective_position.y << std::endl;
		std::cout << "adjustOutput.effective_position.z:" << adjustOutput.effective_position.z << std::endl << std::endl;
	#endif	
		sleep(1);
	}
	
	return 0;
}
