#include "tuner_skeleton.h"

#include <iostream>

using namespace com::hinge::skeleton;

//events
events::BrakeEvent::BrakeEvent(ara::com::ServiceSkeleton* skeleton, uint16_t eventId)
: ara::com::PublishEvent(skeleton, eventId)
{
}
		
void events::BrakeEvent::Send(const SampleType &data)
{
	RadarSerializer serializer;
	serializer.serialize(data);
	
	const uint8_t *sendData = serializer.getData();
	uint32_t len = serializer.getSize();
	
	std::shared_ptr<ara::com::Payload> payload(new ara::com::Payload(len, sendData));
	
	ara::com::PublishEvent::Send(payload);
}

ara::com::SampleAllocateePtr<events::BrakeEvent::SampleType> events::BrakeEvent::Allocate()
{
	std::unique_ptr<events::BrakeEvent::SampleType> ptr(new events::BrakeEvent::SampleType);
	return ptr;
}

void events::BrakeEvent::Send(ara::com::SampleAllocateePtr<SampleType> data)
{
	RadarSerializer serializer;
	serializer.serialize(*(data.get()));
	
	const uint8_t *sendData = serializer.getData();
	uint32_t len = serializer.getSize();
	
	std::shared_ptr<ara::com::Payload> payload(new ara::com::Payload(len, sendData));
	
	ara::com::PublishEvent::Send(payload);
}

RadarSkeketion::RadarSkeketion(ara::com::InstanceIdentifier instance, ara::com::MethodCallProcessingMode mode)
: ara::com::ServiceSkeleton(31, instance, mode), BrakeEvent(this, 1)
{
	std::shared_ptr<ara::com::Configuration> conf(new ara::com::Configuration);
	
	std::shared_ptr<ara::com::Endpoint> server(new ara::com::Endpoint({{127,0,0,1}}, 9000, ara::com::TransportProtocol::ipc));
	//std::shared_ptr<ara::com::Endpoint> client1(new ara::com::Endpoint({{127,0,0,1}}, 9001, ara::com::TransportProtocol::ipc));
	//std::shared_ptr<ara::com::Endpoint> client2(new ara::com::Endpoint({{127,0,0,1}}, 9002, ara::com::TransportProtocol::ipc));
	
	std::vector<std::shared_ptr<ara::com::Endpoint>> servers;
	std::vector<std::shared_ptr<ara::com::Endpoint>> clients;
	
	servers.push_back(server);
	//clients.push_back(client1);
	//clients.push_back(client2);
	
	conf->setServerEndpoint(servers);
	conf->setClientEndpoint(clients);
	conf->setNetWorkBindingType(ara::com::NetWorkBindingType::IPC);
	
	Init(conf);
	
	ara::com::ServiceProvider *provider = ara::com::ManagementFactory::get()->getServiceProvider(this->getServiceId(), this->getInstanceId());
	
	provider->setRequestReceiveHandler(2, [this, provider](std::shared_ptr<ara::com::Message> msg){
		std::shared_ptr<ara::com::Payload> request = msg->getPayload();
		const uint8_t *data = request->getData();
		uint32_t len = request->getSize();
		RadarDeserializer deserializer(data, len);
		Position configuration;
		deserializer.deserialize(configuration);
		
		ara::com::Future<CalibrateOutput> f = this->Calibrate(configuration);
		CalibrateOutput output = f.get();
		
		std::shared_ptr<RadarSerializer> serializer(new RadarSerializer);
		serializer->serialize(output.result);
		
		data = serializer->getData();
		len = serializer->getSize();
		
		std::shared_ptr<ara::com::Payload> response(new ara::com::Payload(len, data));
		
		provider->response(2, msg->getId() | (msg->getSession() << 16), response);
	});
	
	provider->setRequestReceiveHandler(3, [this, provider](std::shared_ptr<ara::com::Message> msg){
		std::shared_ptr<ara::com::Payload> request = msg->getPayload();
		const uint8_t *data = request->getData();
		uint32_t len = request->getSize();
		RadarDeserializer deserializer(data, len);
		Position configuration;
		deserializer.deserialize(configuration);
		
		ara::com::Future<AdjustOutput> f = this->Adjust(configuration);
		AdjustOutput output = f.get();
		
		RadarSerializer serializer;
		serializer.serialize(output.success);
		serializer.serialize(output.effective_position);
		
		data = serializer.getData();
		len = serializer.getSize();
		
		std::shared_ptr<ara::com::Payload> response(new ara::com::Payload(len, data));
		
		provider->response(3, msg->getId() | (msg->getSession() << 16), response);
	});
}
