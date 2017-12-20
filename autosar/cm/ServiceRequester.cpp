/*
 * Copyright (c) 2017, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2017-12-14
 * Author: ryan
 */
 
#include "ServiceRequester.h"
#include "SomeIpBinding.h"

namespace ara
{
namespace com
{

ServiceRequester::ServiceRequester(uint16_t serviceId, uint16_t instanceId, Configuration *conf)
: m_serviceId(serviceId), m_instanceId(instanceId), m_clientId(0), m_session(0)
{
	//someip binding
	std::shared_ptr<SomeIpEndpoint> endpoints(new SomeIpEndpoint());
	m_networkBinding = new SomeIpBinding(m_serviceId, m_instanceId, endpoints);
	m_networkBinding->setReceiveHandler([this](std::shared_ptr<Message> msg){
		this->onMessage(NetWorkBindingType::SOMEIP, msg);
	});
}
			
ServiceRequester::~ServiceRequester()
{
}

bool ServiceRequester::subscribe(uint16_t eventId)
{
	return false;
}
			
bool ServiceRequester::unsubscribe(uint16_t eventId)
{
	return false;
}
			
void ServiceRequester::setEventReceiveHandler(uint16_t eventId, EventHandler handler)
{
	m_eventHandlers[eventId] = handler;
}

void ServiceRequester::unsetEventReceiveHandler(uint16_t eventId)
{
	auto it = m_eventHandlers.find(eventId);
	if (it != m_eventHandlers.end())
	{
		m_eventHandlers.erase(it);
	}
}

bool ServiceRequester::request(uint16_t methodId, std::shared_ptr<Payload> payload, ResponseHandler handler)
{
	std::shared_ptr<Message> msg(new Message());
	uint16_t session = ++m_session;
	
	msg->setServiceId(m_serviceId);
	msg->setInstanceId(m_instanceId);
	msg->setClientId(m_clientId);
	msg->setMethodId(methodId);
	msg->setSession(session);
	msg->setId(0);
	msg->setType(MessageType::MT_REQUEST);
	msg->setCode(ReturnCode::E_OK);
	msg->setPayload(payload);
	
	m_responseHandlers[session] = handler;
	
	if (!m_networkBinding->send(msg))
	{
		auto it = m_responseHandlers.find(session);
		if (it != m_responseHandlers.end())
		{
			m_responseHandlers.erase(it);
		}
		
		return false;
	}
	
	return true;
}
			
void ServiceRequester::onMessage(NetWorkBindingType type, std::shared_ptr<Message> msg)
{
	if (msg->getType() == MessageType::MT_NOTIFICATION)
	{
		auto it = m_eventHandlers.find(msg->getMethodId());
		if (it != m_eventHandlers.end())
		{
			it->second(msg->getPayload());
		}
	}
	else if (msg->getType() == MessageType::MT_RESPONSE)
	{
		auto it = m_responseHandlers.find(msg->getSession());
		if (it != m_responseHandlers.end())
		{
			it->second(msg);
			m_responseHandlers.erase(it);
		}
	}
}
			
} // namespace com
} // namespace ara
