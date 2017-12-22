/*
 * Copyright (c) 2017, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2017-12-14
 * Author: ryan
 */
 
#include "SomeIpBinding.h"
#include "ManagementFactory.h"

#include <vsomeip/SomeIPManager.hpp>

#include <iostream>

namespace ara
{
namespace com
{

SomeIpBinding::SomeIpBinding(uint16_t serviceId, uint16_t instanceId, std::shared_ptr<SomeIpEndpoint> someIpEndpoint)
: m_serviceId(serviceId), m_instanceId(instanceId), m_someIpEndpoint(someIpEndpoint)
{
	//m_someIpManager = vsomeip::SomeIPManager::get();
	//std::shared_ptr<vsomeip::SomeIPPort> ipPort(new vsomeip::SomeIPPort());
	//TODO
	
	//m_someIpManager->createSomeIP(ipPort);
}

SomeIpBinding::~SomeIpBinding()
{
}

bool SomeIpBinding::send(std::shared_ptr<Message> msg)
{
	std::shared_ptr<vsomeip::SomeIPMessage> ipMsg(new vsomeip::SomeIPMessage());
	
	if (msg->getType() == MessageType::MT_NOTIFICATION) //event
	{
		std::cout << "notify:" << (char*)msg->getPayload()->getData() << std::endl;
		ServiceRequester *sr = ManagementFactory::get()->getServiceRequester(msg->getServiceId(), msg->getInstanceId());
		sr->getNetworkBinding()->onMessage(msg);
	}
	else if (msg->getType() == MessageType::MT_REQUEST || msg->getType() == MessageType::MT_REQUEST_NO_RETURN) //method request
	{
		std::cout << "method request:" << (char*)msg->getPayload()->getData() << std::endl;
		ServiceProvider *sp = ManagementFactory::get()->getServiceProvider(msg->getServiceId(), msg->getInstanceId());
		sp->getNetworkBinding()->onMessage(msg);
	}
	else //method response or others
	{
		std::cout << "method response[" << (int)msg->getCode() << "]" << ":" << (char*)msg->getPayload()->getData() << std::endl;
		ServiceRequester *sr = ManagementFactory::get()->getServiceRequester(msg->getServiceId(), msg->getInstanceId());
		sr->getNetworkBinding()->onMessage(msg);
	}
	
	//m_someIpManager->sendSomeIP(ipMsg);
	
	return true;
}

void SomeIpBinding::setReceiveHandler(MessageReceiveHandler handler)
{
	m_handler = handler;
}

void SomeIpBinding::onMessage(std::shared_ptr<Message> msg)
{
	if (m_handler)
	{
		m_handler(msg);
	}
}

void SomeIpBinding::subscribe(uint16_t eventgroupId)
{
}

void SomeIpBinding::unsubscribe(uint16_t eventgroupId)
{
}

void SomeIpBinding::addSubscriber(uint16_t eventgroupId, std::shared_ptr<Endpoint> endpoint)
{
	auto  subscribers = m_eventgroupSubscribers.find(eventgroupId);
	if (subscribers != m_eventgroupSubscribers.end())
	{
		for (std::vector<std::shared_ptr<Endpoint>>::iterator it = subscribers->second.begin(); it != subscribers->second.end(); it++)
		{
			if ((*it).get() == endpoint.get())
			{
				return;
			}
		}
	}
	
	m_eventgroupSubscribers[eventgroupId].push_back(endpoint);
}

void SomeIpBinding::delSubscriber(uint16_t eventgroupId, std::shared_ptr<Endpoint> endpoint)
{
	auto  subscribers = m_eventgroupSubscribers.find(eventgroupId);
	if (subscribers != m_eventgroupSubscribers.end())
	{
		for (std::vector<std::shared_ptr<Endpoint>>::iterator it = subscribers->second.begin(); it != subscribers->second.end(); it++)
		{
			if ((*it).get() == endpoint.get())
			{
				subscribers->second.erase(it);
				break;
			}
		}
	}
}

} // namespace com
} // namespace ara
