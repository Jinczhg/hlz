#include <com/myCompany/radar_skeleton.h>

#include <unistd.h>
#include <iostream>

class RadarSkeketionImp : public com::myCompany::skeleton::RadarSkeketion
{
public:

	RadarSkeketionImp(ara::com::InstanceIdentifier instance, ara::com::MethodCallProcessingMode mode = ara::com::MethodCallProcessingMode::kEvent)
	: com::myCompany::skeleton::RadarSkeketion(instance, mode)
	{
	}
	
	virtual ~RadarSkeketionImp(){}
		
	virtual ara::com::Future<com::myCompany::skeleton::RadarSkeketion::CalibrateOutput> Calibrate(const Position& configuration)
	{
		std::cout << "Calibrate" << std::endl;
		
		ara::com::Promise<com::myCompany::skeleton::RadarSkeketion::CalibrateOutput> p;
		
		com::myCompany::skeleton::RadarSkeketion::CalibrateOutput output;
		
		output.result = true;
		
		p.set_value(output);
		
		return p.get_future();
	}
	
	virtual ara::com::Future<com::myCompany::skeleton::RadarSkeketion::AdjustOutput> Adjust(const Position& target_position)
	{
		std::cout << "Adjust" << std::endl;
		
		ara::com::Promise<com::myCompany::skeleton::RadarSkeketion::AdjustOutput> p;
		
		com::myCompany::skeleton::RadarSkeketion::AdjustOutput output;
		
		output.success = true;
		output.effective_position.x = target_position.x;
		output.effective_position.y = target_position.y;
		output.effective_position.z = target_position.z;
		
		p.set_value(output);
		
		return p.get_future();
	}
};

int main(int argc, char** argv)
{
	std::cout << "InstanceIdentifier" << std::endl;
	
	ara::com::InstanceIdentifier instance("1");
	
	std::cout << "RadarSkeketionImp instance" << std::endl;
	
	RadarSkeketionImp skeleton(instance, ara::com::MethodCallProcessingMode::kEvent);
	
	std::cout << "RadarSkeketionImp instance OK" << std::endl;
	
	while (1)
	{
		com::myCompany::skeleton::events::BrakeEvent::SampleType sample;
		skeleton.BrakeEvent.Send(sample);
		sleep(1);
	}

	return 0;
}
