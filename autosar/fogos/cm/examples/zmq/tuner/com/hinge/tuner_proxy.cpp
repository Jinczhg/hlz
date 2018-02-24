#include "tuner_proxy.h"

#include <iostream>

using namespace com::hinge::proxy;

//events

events::SwitchStateEvent::SwitchStateEvent(ara::com::ServiceProxy* proxy, uint16_t eventId)
: ara::com::SubscribeEvent(proxy, eventId)
{
}

bool events::SwitchStateEvent::Update(ara::com::FilterFunction<events::BrakeEvent::SampleType> filter)
{
	m_samples.clear();
	std::lock_guard<std::mutex> guard(m_mutex);
	
	if (!m_data.empty())
	{
		for (auto payload : m_data)
		{
			uint8_t *data = payload->getData();
			uint32_t len = payload->getSize();
			
			ara::com::SamplePtr<events::BrakeEvent::SampleType> sample(new events::BrakeEvent::SampleType);
			
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

const ara::com::SampleContainer<ara::com::SamplePtr<const events::BrakeEvent::SampleType>>& events::SwitchStateEvent::GetCachedSamples() const
{
	return m_samples;
}

//methods

methods::Calibrate::Calibrate(ara::com::ServiceProxy* proxy, uint16_t methodId)
: ara::com::Method(proxy, methodId)
{
}

ara::com::Future<methods::Calibrate::Output> methods::Calibrate::operator()(const Position& configuration)
{
	RadarSerializer serializer;
	serializer.serialize(configuration);
	
	const uint8_t *data = serializer.getData();
	uint32_t len = serializer.getSize();
	uint16_t session = 0;
	
	std::shared_ptr<ara::com::Payload> payload(new ara::com::Payload(len, data));
	ara::com::Promise<methods::Calibrate::Output> promise;
	
	ara::com::Future<methods::Calibrate::Output> future = promise.get_future();
	
	session = ara::com::Method::operator()(payload, [&promise, this](std::shared_ptr<ara::com::Payload> payload){
		methods::Calibrate::Output output;
		const uint8_t *resData = payload->getData();
		uint32_t resLen = payload->getSize();
	
		RadarDeserializer deserializer(resData, resLen);
		deserializer.deserialize(output.result);
		try
		{
			promise.set_value(output);
		}
		catch(std::exception &e)
		{
			std::cout << e.what() << std::endl;
		}
	});
	
	//future.wait();

	ara::com::FutureStatus status = future.wait_for(std::chrono::seconds(1));
	if (status != ara::com::FutureStatus::ready)
	{
		this->cancel(session);
		usleep(1000);
		throw ara::com::Exception("methods::Calibrate timeout");
	}
    
	return future;
}

methods::Adjust::Adjust(ara::com::ServiceProxy* proxy, uint16_t methodId)
: ara::com::Method(proxy, methodId)
{
}

ara::com::Future<methods::Adjust::Output> methods::Adjust::operator()(const Position& target_position)
{	
	RadarSerializer serializer;
	serializer.serialize(target_position);
	
	const uint8_t *data = serializer.getData();
	uint32_t len = serializer.getSize();
	uint16_t session = 0;
	
	std::shared_ptr<ara::com::Payload> payload(new ara::com::Payload(len, data));
	ara::com::Promise<methods::Adjust::Output> promise;
	
	ara::com::Future<methods::Adjust::Output> future = promise.get_future();
	
	session = ara::com::Method::operator()(payload, [&promise, this](std::shared_ptr<ara::com::Payload> payload){
		methods::Adjust::Output output;
		uint8_t *resData = payload->getData();
		uint32_t resLen = payload->getSize();
		
		RadarDeserializer deserializer(resData, resLen);
		deserializer.deserialize(output.success);
		deserializer.deserialize(output.effective_position);

		try
		{
			promise.set_value(output);
		}
		catch(std::exception &e)
		{
			std::cout << e.what() << std::endl;
		}
	});
	
	//future.wait();
	ara::com::FutureStatus status = future.wait_for(std::chrono::seconds(1));
	if (status != ara::com::FutureStatus::ready)
	{
		this->cancel(session);
		usleep(1000);
		throw ara::com::Exception("methods::Adjust timeout");
	}
	
	return future;
}

//proxy
RadarProxy::RadarProxy(ara::com::ServiceProxy::HandleType handle)
: ara::com::ServiceProxy(handle), BrakeEvent(this, 1), Calibrate(this, 2), Adjust(this, 3)
{
	Init(handle.getConf());
}
			
ara::com::ServiceHandleContainer<ara::com::ServiceProxy::HandleType> RadarProxy::FindService(ara::com::InstanceIdentifier instance)
{
	//return ara::com::ServiceProxy::FindService(31, instance);
	std::shared_ptr<ara::com::Configuration> conf(new ara::com::Configuration);
	
	std::shared_ptr<ara::com::Endpoint> server(new ara::com::Endpoint({{127,0,0,1}}, 9000, ara::com::TransportProtocol::ipc));
	std::shared_ptr<ara::com::Endpoint> client(new ara::com::Endpoint({{127,0,0,1}}, 9001, ara::com::TransportProtocol::ipc));
	
	std::vector<std::shared_ptr<ara::com::Endpoint>> servers;
	std::vector<std::shared_ptr<ara::com::Endpoint>> clients;
	
	servers.push_back(server);
	clients.push_back(client);
	
	conf->setServerEndpoint(servers);
	conf->setClientEndpoint(clients);
	conf->setNetWorkBindingType(ara::com::NetWorkBindingType::IPC);
	
	ara::com::ServiceProxy::HandleType handle(31,1,conf);
	
	ara::com::ServiceHandleContainer<ara::com::ServiceProxy::HandleType> handles;
	handles.push_back(handle);
	
	return handles;
}

ara::com::FindServiceHandle RadarProxy::StartFindService(ara::com::FindServiceHandler<ara::com::ServiceProxy::HandleType> handler, ara::com::InstanceIdentifier instance)
{
	return ara::com::ServiceProxy::StartFindService(handler, 31, instance);
}
