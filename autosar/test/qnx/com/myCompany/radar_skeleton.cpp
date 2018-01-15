#incluce "radar_skeleton.h"

using namespace com::myCompany;

//events
events::BrakeEvent::BrakeEvent(ara::com::ServiceSkeleton* proxy, uint16_t eventId)
{
}
		
void events::BrakeEvent::Send(const SampleType &data)
{
	RadarSerializer serializer();
	serializer.serialize(data);
	
	uint8_t *sendData = serializer.getData();
	uint32_t len = serializer.getSize();
	
	std::shared_ptr<Payload> payload(new ara::com::Payload(sendData, len));
	
	Send(payload);
}

ara::com::SampleAllocateePtr<SampleType> events::BrakeEvent::Allocate()
{
	std::unique_ptr<SampleType> ptr(new SampleType);
	return ptr;
}

void events::BrakeEvent::Send(ara::com::SampleAllocateePtr<SampleType> data)
{
	RadarSerializer serializer();
	serializer.serialize(*(data.get()));
	
	uint8_t *sendData = serializer.getData();
	uint32_t len = serializer.getSize();
	
	std::shared_ptr<Payload> payload(new ara::com::Payload(sendData, len));
	
	Send(payload);
}

RadarSkeketion::RadarSkeketion(ara::com::InstanceIdentifier instance, ara::com::MethodCallProcessingMode mode)
: rar::com::ServiceSkeleton(31, instance, mode), BrakeEvent(this, 1)
{
}

bool RadarSkeketion::Init(Configuration* conf)
{
	rar::com::ServiceSkeleton::Init(conf);
	
	ara::com::ServiceProvider *provider = ara::com::ManagementFactory::get()->getServiceProvider(this->getServiceId(), this->getInstanceId());
	
	provider->setRequestReceiveHandler(2, [this, provider](std::shared_ptr<ara::com::Message> msg){
		std::shared_ptr<Payload> request = msg->getPayload();
		uint8_t *data = request->getData();
		uint32_t len = request->getSize();
		RadarDeserializer deserializer(data, len);
		Position configuration;
		deserializer.deserialize(configuration);
		
		ara::com::Future<CalibrateOutput> f = this->Calibrate(configuration);
		CalibrateOutput output = f.get();
		
		RadarSerializer serializer();
		serializer.serialize(output.result);
		
		data = serializer.getData();
		len = serializer.getSize();
		
		std::shared_ptr<Payload> response(new ara::com::Payload(data, len));
		
		provider->response(2, msg->getId() | (msg->getSession() << 16), response);
	});
	
	provider->setRequestReceiveHandler(3, [this, provider](std::shared_ptr<ara::com::Message> msg){
		std::shared_ptr<Payload> request = msg->getPayload();
		uint8_t *data = request->getData();
		uint32_t len = request->getSize();
		RadarDeserializer deserializer(data, len);
		Position configuration;
		deserializer.deserialize(configuration);
		
		ara::com::Future<AdjustOutput> f = this->Adjust(configuration);
		AdjustOutput output = f.get();
		
		RadarSerializer serializer();
		serializer.serialize(output.success);
		serializer.serialize(output.effective_position);
		
		data = serializer.getData();
		len = serializer.getSize();
		
		std::shared_ptr<Payload> response(new ara::com::Payload(data, len));
		
		provider->response(3, msg->getId() | (msg->getSession() << 16), response);
	});
}
