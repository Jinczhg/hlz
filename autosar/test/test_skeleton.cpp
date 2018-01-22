#include <com/myCompany/radar_skeleton.h>

#include <unistd.h>
#include <iostream>
#include <sstream>

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
		std::cout << "skeleton Calibrate" << std::endl;
		
		m_pos.x = configuration.x;
		m_pos.y = configuration.y;
		m_pos.z = configuration.z;
		
		ara::com::Promise<com::myCompany::skeleton::RadarSkeketion::CalibrateOutput> p;
		
		com::myCompany::skeleton::RadarSkeketion::CalibrateOutput output;
		
		output.result = true;
		
		p.set_value(output);
		
		return p.get_future();
	}
	
	virtual ara::com::Future<com::myCompany::skeleton::RadarSkeketion::AdjustOutput> Adjust(const Position& target_position)
	{
		std::cout << "skeleton Adjust" << std::endl;
		
		ara::com::Promise<com::myCompany::skeleton::RadarSkeketion::AdjustOutput> p;
		
		com::myCompany::skeleton::RadarSkeketion::AdjustOutput output;
		
		output.success = true;
		output.effective_position.x = m_pos.x + target_position.x;
		output.effective_position.y = m_pos.y + target_position.y;
		output.effective_position.z = m_pos.z + target_position.z;
		
		p.set_value(output);
		
		return p.get_future();
	}
	
	private:
		Position m_pos;
};

int main(int argc, char** argv)
{
	std::cout << "InstanceIdentifier" << std::endl;
	
	ara::com::InstanceIdentifier instance("1");
	
	std::cout << "RadarSkeketionImp instance" << std::endl;
	
	RadarSkeketionImp skeleton(instance, ara::com::MethodCallProcessingMode::kEvent);
	
	std::cout << "RadarSkeketionImp instance OK" << std::endl;
	
	int num = 0;
	
	while (1)
	{
		com::myCompany::skeleton::events::BrakeEvent::SampleType sample;
		sample.active = true;
		std::stringstream objects;
		objects << "RadarObjects" << num;
		for (uint32_t i = 0; i < objects.str().size(); i++)
		{
			sample.objects.push_back(objects.str().c_str()[i]);
		}
		sample.objects.push_back('\0');
		
		skeleton.BrakeEvent.Send(sample);
		
		num++;
		
		sleep(1);
	}

	return 0;
}
