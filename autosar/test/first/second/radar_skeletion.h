#ifndef RADAR_SKELETON_H_
#define RADAR_SKELETON_H_

#include "radar_common.h"

namespace first
{
namespace second
{
namespace proxy
{
	
namespace events
{
	class BrakeEvent : public PublishEvent
	{
	public:
		using SampleType = ara::com::RadarObjects;
		
		BrakeEvent(ara::com::ServiceSkeleton* proxy, uint16_t eventId);
		virtual ~BrakeEvent(){}
		
		void Send(const SampleType &data);
		
		ara::com::SampleAllocateePtr<SampleType> Allocate();
		
		void Send(ara::com::SampleAllocateePtr<SampleType> data);
	};
} // namespace events

	class RadarSkeketion : public ara::com::ServiceSkeleton
	{
	public:
		RadarSkeketion(ara::com::InstanceIdentifier instance, ara::com::MethodCallProcessingMode mode = ara::com::MethodCallProcessingMode::kEvent);
		virtual ~RadarSkeketion();
		
		events::BrakeEvent BrakeEvent;
		
		struct CalibrateOutput
		{
			ara::com::boolean result;
		};
		
		virtual ara::com::Future<CalibrateOutput> Calibrate(const Position& configuration) = 0;
		
		struct AdjustOutput
		{
			ara::com::boolean success;
			Position effective_position;
		};
		
		virtual ara::com::Future<AdjustOutput> Adjust(const Position& target_position) = 0;
		
		bool Init(Configuration* conf);
	};
} // namespace proxy
} // namespace second
} // namespace first

#endif //RADAR_SKELETON_H_
