#ifndef RADAR_SKELETON_H_
#define RADAR_SKELETON_H_

#include "radar_common.h"

namespace com
{
namespace myCompany
{
namespace skeleton
{
	
namespace events
{
	class BrakeEvent : public ara::com::PublishEvent
	{
	public:
		using SampleType = RadarObjects;
		
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
		virtual ~RadarSkeketion(){}
		
		events::BrakeEvent BrakeEvent;
		
		struct CalibrateOutput
		{
			boolean result;
		};
		
		virtual ara::com::Future<CalibrateOutput> Calibrate(const Position& configuration) = 0;
		
		struct AdjustOutput
		{
			boolean success;
			Position effective_position;
		};
		
		virtual ara::com::Future<AdjustOutput> Adjust(const Position& target_position) = 0;
	};
} // namespace proxy
} // namespace second
} // namespace first

#endif //RADAR_SKELETON_H_
