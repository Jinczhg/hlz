/*
 * Copyright (c) 2017, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2017-12-14
 * Author: ryan
 */
 
#include "SomeIpBinding.h"

#include <vsomeip/SomeIPManager.hpp>

namespace ara
{
namespace com
{

SomeIpBinding::SomeIpBinding()
{
}

SomeIpBinding::~SomeIpBinding()
{
}

bool SomeIpBinding::send(std::shared_ptr<Message> msg)
{
	std::shared_ptr<vsomeip::SomeIPMessage> ipMsg(new vsomeip::SomeIPMessage());
	
	if (msg->getType() == MessageType::MT_NOTIFICATION) //event
	{
	}
	else if (msg->getType() == MessageType::MT_REQUEST || msg->getType() == MessageType::MT_REQUEST_NO_RETURN) //method request
	{
	}
	else //method response or others
	{
	}
	
	
	
	return true;
}

void SomeIpBinding::setReceiveHandler(MessageReceiveHandler handler)
{
}

void SomeIpBinding::onMessage(std::shared_ptr<Message> msg)
{
}

void SomeIpBinding::subscribe(uint16_t eventgroupId)
{
}

void SomeIpBinding::unsubscribe(uint16_t eventgroupId)
{
}

void SomeIpBinding::addSubscriber(uint16_t eventgroupId, Endpoint endpoint)
{
}

void SomeIpBinding::delSubscriber(uint16_t eventgroupId, Endpoint endpoint)
{
}

} // namespace com
} // namespace ara
