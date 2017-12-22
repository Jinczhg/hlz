/*
 * Copyright (c) 2017, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2017-12-14
 * Author: ryan
 */
 
#include "ServiceProvider.h"
#include "SomeIpBinding.h"

#include <thread>
#include <iostream>

namespace ara
{
namespace com
{

ServiceProvider::ServiceProvider(uint16_t serviceId, uint16_t instanceId, MethodCallProcessingMode mode, Configuration *conf)
: m_serviceId(serviceId), m_instanceId(instanceId), m_mode(mode), m_clientId(0), m_session(0), m_mutex(), m_condition()
{
	//someip binding
	std::shared_ptr<SomeIpEndpoint> endpoints(new SomeIpEndpoint());
	m_networkBinding = new SomeIpBinding(m_serviceId, m_instanceId, endpoints);
	m_networkBinding->setReceiveHandler([this](std::shared_ptr<Message> msg){
		this->onMessage(NetWorkBindingType::SOMEIP, msg);
	});
}
			
ServiceProvider::~ServiceProvider()
{
}

void ServiceProvider::addSubscriber(uint16_t eventgroupId, std::shared_ptr<Endpoint> endpoint)
{
	m_networkBinding->addSubscriber(eventgroupId, endpoint);
}

void ServiceProvider::delSubscriber(uint16_t eventgroupId, std::shared_ptr<Endpoint> endpoint)
{
	m_networkBinding->delSubscriber(eventgroupId, endpoint);
}
			
void ServiceProvider::notify(uint16_t eventId, std::shared_ptr<Payload> payload)
{
	std::shared_ptr<Message> msg(new Message());
	
	msg->setServiceId(m_serviceId);
	msg->setInstanceId(m_instanceId);
	msg->setClientId(m_clientId);
	msg->setMethodId(eventId);
	msg->setSession(m_session++);
	msg->setId(0);
	msg->setType(MessageType::MT_NOTIFICATION);
	msg->setCode(ReturnCode::E_OK);
	msg->setPayload(payload);
	
	m_networkBinding->send(msg);
}
			
void ServiceProvider::response(uint16_t methodId, uint32_t request, std::shared_ptr<Payload> payload)
{
	std::shared_ptr<Message> msg(new Message());
	
	msg->setServiceId(m_serviceId);
	msg->setInstanceId(m_instanceId);
	msg->setClientId(m_clientId);
	msg->setMethodId(methodId);
	msg->setSession((request>>16) & 0xFFFF);
	msg->setId(request & 0xFFFF);
	msg->setType(MessageType::MT_RESPONSE);
	msg->setCode(ReturnCode::E_OK);
	msg->setPayload(payload);
	
	m_networkBinding->send(msg);
}
			
void ServiceProvider::setRequestReceiveHandler(uint16_t methodId, RequestReceiveHandler handler)
{
	m_handlers[methodId] = handler;
}

void ServiceProvider::unsetRequestReceiveHandler(uint16_t methodId)
{
	auto it = m_handlers.find(methodId);
	if (it != m_handlers.end())
	{
		m_handlers.erase(it);
	}
}

bool ServiceProvider::hasRequest()
{
	return !m_requestMessages.empty();
}

void ServiceProvider::processRequest()
{
	std::unique_lock<std::mutex> lck(m_mutex);
	if (m_requestMessages.empty())
	{
		m_condition.wait(lck);
	}
	
	std::shared_ptr<Message> msg = m_requestMessages[0];
	m_requestMessages.erase(m_requestMessages.begin());
	
	uint16_t methodId = msg->getMethodId();
	auto it = m_handlers.find(methodId);
	if (it != m_handlers.end())
	{
		it->second(msg);
	}
	
}
			
void ServiceProvider::onMessage(NetWorkBindingType type, std::shared_ptr<Message> msg)
{
	uint16_t methodId = msg->getMethodId();
	auto it = m_handlers.find(methodId);
	if (it != m_handlers.end())
	{
		if (m_mode == MethodCallProcessingMode::kPoll)
		{
			std::unique_lock<std::mutex> lck(m_mutex);
			m_requestMessages.push_back(msg);
			m_condition.notify_one();
		}
		else if (m_mode == MethodCallProcessingMode::kEvent)
		{
			std::thread t([this,msg,methodId](){
				auto it = this->m_handlers.find(methodId);
				if (it != this->m_handlers.end())
				it->second(msg);
			});
			t.detach();
		}
		else if (m_mode == MethodCallProcessingMode::kEventSingleThread)
		{
			it->second(msg);
		}
	}
	else
	{
		msg->setType(MessageType::MT_RESPONSE);
		msg->setCode(ReturnCode::E_UNKNOWN_METHOD);
		m_networkBinding->send(msg);
	}
}

BaseNetworkBinding* ServiceProvider::getNetworkBinding() const
{
	return m_networkBinding;
}
		
} // namespace com
} // namespace ara
