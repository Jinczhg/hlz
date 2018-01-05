#ifndef RADAR_PROXY_H_
#define RADAR_PROXY_H_

#include "radar_common.h"

namespace first
{
namespace second
{
namespace proxy
{
	
namespace events
{
	class BrakeEvent : public ara::com::SubscribeEvent
	{
	public:
		using SampleType = RadarObjects;
		
		BrakeEvent(ara::com::ServiceProxy* proxy, uint16_t eventId);
		virtual ~BrakeEvent(){}
		
		bool Update(ara::com::FilterFunction<SampleType> filter = {});
		
		const ara::com::SampleContainer<ara::com::SamplePtr<const SampleType>>& GetCachedSamples() const;
		 
	private:
		ara::com::SampleContainer<ara::com::SamplePtr<const SampleType>> m_samples;
	};
} // namespace events

namespace methods
{
	class Calibrate : public ara::com::Method
	{
	public:
		struct Output
		{
			ara::com::boolean result;
		};
		
		Calibrate(ara::com::ServiceProxy* proxy, uint16_t methodId);
		virtual ~Calibrate(){}
		
		ara::com::Future<Output> operator()(const Position& configuration);
	};
	
	class Adjust : public ara::com::Method
	{
	public:
		struct Output
		{
			boolean success;
			ara::com::Position effective_position;
		};
		
		Adjust(ara::com::ServiceProxy* proxy, uint16_t methodId);
		virtual ~Adjust(){}
		
		ara::com::Future<Output> operator()(const Position& target_position);
	};
} //namespace methods

	class RadarProxy : public ara::com::ServiceProxy
	{
	public:
		explicit RadarProxy(ara::com::HandleType handle);
		virtual ~ServiceProxy(){}
			
		static ServiceHandleContainer<ara::com::ServiceProxy::HandleType> FindService(ara::com::InstanceIdentifier instance);
		static ara::com::FindServiceHandle StartFindService(FindServiceHandler<ara::com::ServiceProxy::HandleType> handler, ara::com::InstanceIdentifier instance);
		
		events::BrakeEvent BrakeEvent;
		methods::Calibrate Calibrate;
		methods::Adjust Adjust;
	};
} // namespace proxy
} // namespace second
} // namespace first

#endif //RADAR_PROXY_H_
