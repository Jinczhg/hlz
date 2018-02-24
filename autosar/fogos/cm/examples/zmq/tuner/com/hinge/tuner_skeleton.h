#ifndef RADAR_SKELETON_H_
#define RADAR_SKELETON_H_

#include "tuner_common.h"

namespace com
{
namespace hinge
{
namespace skeleton
{
	
namespace events
{
	class SwitchStateEvent : public ara::com::PublishEvent
	{
	public:
		using SampleType = std::vector<uint8>;
		
		SwitchStateEvent(ara::com::ServiceSkeleton* proxy, uint16_t eventId);
		virtual ~SwitchStateEvent(){}
		
		void Send(const SampleType &data);
		
		ara::com::SampleAllocateePtr<SampleType> Allocate();
		
		void Send(ara::com::SampleAllocateePtr<SampleType> data);
	};
	
	class ChangModeEvent : public ara::com::PublishEvent
	{
	public:
		using SampleType = std::vector<uint8>;
		
		ChangModeEvent(ara::com::ServiceSkeleton* proxy, uint16_t eventId);
		virtual ~ChangModeEvent(){}
		
		void Send(const SampleType &data);
		
		ara::com::SampleAllocateePtr<SampleType> Allocate();
		
		void Send(ara::com::SampleAllocateePtr<SampleType> data);
	};
	
	class GetFreqEvent : public ara::com::PublishEvent
	{
	public:
		using SampleType = sint32;
		
		GetFreqEvent(ara::com::ServiceSkeleton* proxy, uint16_t eventId);
		virtual ~GetFreqEvent(){}
		
		void Send(const SampleType &data);
		
		ara::com::SampleAllocateePtr<SampleType> Allocate();
		
		void Send(ara::com::SampleAllocateePtr<SampleType> data);
	};
	
	class ActionStateEvent : public ara::com::PublishEvent
	{
	public:
		using SampleType = SActionState;
		
		ActionStateEvent(ara::com::ServiceSkeleton* proxy, uint16_t eventId);
		virtual ~ActionStateEvent(){}
		
		void Send(const SampleType &data);
		
		ara::com::SampleAllocateePtr<SampleType> Allocate();
		
		void Send(ara::com::SampleAllocateePtr<SampleType> data);
	};
	
	class PlayListEvent : public ara::com::PublishEvent
	{
	public:
		using SampleType = SPlayList;
		
		PlayListEvent(ara::com::ServiceSkeleton* proxy, uint16_t eventId);
		virtual ~PlayListEvent(){}
		
		void Send(const SampleType &data);
		
		ara::com::SampleAllocateePtr<SampleType> Allocate();
		
		void Send(ara::com::SampleAllocateePtr<SampleType> data);
	};
	
	class GetRtDataEvent : public ara::com::PublishEvent
	{
	public:
		using SampleType = SObtainDataInfo;
		
		GetRtDataEvent(ara::com::ServiceSkeleton* proxy, uint16_t eventId);
		virtual ~GetRtDataEvent(){}
		
		void Send(const SampleType &data);
		
		ara::com::SampleAllocateePtr<SampleType> Allocate();
		
		void Send(ara::com::SampleAllocateePtr<SampleType> data);
	};
	
	class FavoritesListEvent : public ara::com::PublishEvent
	{
	public:
		using SampleType = SPlayList;
		
		FavoritesListEvent(ara::com::ServiceSkeleton* proxy, uint16_t eventId);
		virtual ~FavoritesListEvent(){}
		
		void Send(const SampleType &data);
		
		ara::com::SampleAllocateePtr<SampleType> Allocate();
		
		void Send(ara::com::SampleAllocateePtr<SampleType> data);
	};
} // namespace events

	class TunerSkeketion : public ara::com::ServiceSkeleton
	{
	public:
		TunerSkeketion(ara::com::InstanceIdentifier instance, ara::com::MethodCallProcessingMode mode = ara::com::MethodCallProcessingMode::kEvent);
		virtual ~TunerSkeketion(){}
		
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
