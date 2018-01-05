#incluce "radar_proxy.h"

using namespace first::second;

//events

events::BrakeEvent::BrakeEvent(ara::com::ServiceProxy* proxy, uint16_t eventId)
: ara::com::SubscribeEvent(proxy, eventId)
{
}

bool events::BrakeEvent::Update(ara::com::FilterFunction<events::BrakeEvent::SampleType> filter)
{
	m_samples.clear();
	
	std::lock_guard<std::mutex> guard(m_mutex);
	
	if (!m_data.empty())
	{
		for (auto payload : m_data)
		{
			uint8_t *data = payload->getData();
			uint32_t len = payload->getSize();
			events::BrakeEvent::SampleType sample;
			ara::com::SamplePtr<const events::BrakeEvent::SampleType> sample(new events::BrakeEvent::SampleType);
			
			RadarDeserializer deserializer(data, len);
			deserializer.deserialize(*(sample.get()));
			
			if (filter)
			{
				if (filter(*(sample.get())))
				{
					m_samples.push_back(sample);
				}
			}
			else
			{
				m_samples.push_back(sample);
			}
		}
	}
	
	return !m_samples.empty();
}

const ara::com::SampleContainer<ara::com::SamplePtr<const events::BrakeEvent::SampleType>>& events::BrakeEvent::GetCachedSamples() const
{
	return m_samples;
}

//methods

methods::Calibrate::Calibrate(ara::com::ServiceProxy* proxy, uint16_t methodId)
: ara::com::Method(proxy, methodId)
{
}

ara::com::Future<methods::Calibrate::Output> Calibrate::operator()(const Position& configuration)
{
	RadarSerializer serializer();
	serializer.serialize(configuration);
	
	uint8_t *data = serializer.getData();
	uint32_t len = serializer.getSize();
	
	std::shared_ptr<Payload> payload(new ara::com::Payload(data, len));
	ara::com::Promise<methods::Calibrate::Output> promise;
	
	ara::com::Method::operator(payload, [&promise](std::shared_ptr<ara::com::Payload> payload){
		methods::Calibrate::Output output;
		uint8_t *resData = payload.getData();
		uint32_t resLen = payload->getSize();
		
		RadarDeserializer deserializer(data, len);
		deserializer.deserialize(output.result);
		
		promise.set_value(output);
	});
	
	return promise.get_future();
}

methods::Adjust::Adjust(ara::com::ServiceProxy* proxy, uint16_t methodId)
: ara::com::Method(proxy, methodId)
{
}

ara::com::Future<methods::Adjust::Output> Adjust::operator()(const Position& target_position)
{
	RadarSerializer serializer();
	serializer.serialize(target_position);
	
	uint8_t *data = serializer.getData();
	uint32_t len = serializer.getSize();
	
	std::shared_ptr<Payload> payload(new ara::com::Payload(data, len));
	ara::com::Promise<methods::Calibrate::Output> promise;
	
	ara::com::Method::operator(payload, [&promise](std::shared_ptr<ara::com::Payload> payload){
		methods::Adjust::Output output;
		uint8_t *resData = payload.getData();
		uint32_t resLen = payload->getSize();
		
		RadarDeserializer deserializer(data, len);
		deserializer.deserialize(output.success);
		deserializer.deserialize(output.effective_position);
		
		promise.set_value(output);
	});
	
	return promise.get_future();
}

//proxy
RadarProxy::RadarProxy(ara::com::HandleType handle)
: ara::com::ServiceProxy(handle)
{
}
			
ServiceHandleContainer<ara::com::ServiceProxy::HandleType> RadarProxy::FindService(ara::com::InstanceIdentifier instance)
{
	return FindService(31, instance);
}

ara::com::FindServiceHandle RadarProxy::StartFindService(FindServiceHandler<ara::com::ServiceProxy::HandleType> handler, ara::com::InstanceIdentifier instance)
{
	return StartFindService(handler, 31, instance);
}
