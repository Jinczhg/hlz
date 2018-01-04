#incluce "radar_proxy.h"

using namespace first::second;

//events

events::BrakeEvent::BrakeEvent(ara::com::ServiceProxy* proxy, uint16_t eventId)
: SubscribeEvent(proxy, eventId)
{
}

bool events::BrakeEvent::Update(ara::com::FilterFunction<SampleType> filter = {})
{
}

const ara::com::SampleContainer<ara::com::SamplePtr<const SampleType>>& events::BrakeEvent::GetCachedSamples() const
{
}

//methods

methods::Calibrate::Calibrate(ara::com::ServiceProxy* proxy, uint16_t methodId)
{
}

ara::com::Future<methods::Calibrate::Output> Calibrate::operator()(const Position& configuration)
{
}

methods::Adjust::Adjust(ara::com::ServiceProxy* proxy, uint16_t methodId)
{
}

ara::com::Future<methods::Adjust::Output> Adjust::operator()(const Position& target_position)
{
}

//proxy
RadarProxy::RadarProxy(ara::com::HandleType handle)
{
}
			
ServiceHandleContainer<ara::com::ServiceProxy::HandleType> RadarProxy::FindService(ara::com::InstanceIdentifier instance)
{
}

ara::com::FindServiceHandle RadarProxy::StartFindService(FindServiceHandler<ara::com::ServiceProxy::HandleType> handler, ara::com::InstanceIdentifier instance)
{
}
