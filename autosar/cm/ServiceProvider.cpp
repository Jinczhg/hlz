/*
 * Copyright (c) 2017, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2017-12-14
 * Author: ryan
 */
 
#include "ServiceProvider.h"

namespace ara
{
namespace com
{

ServiceProvider::ServiceProvider(uint16_t serviceId, uint16_t instanceId, Configuration *conf)
{
}
			
ServiceProvider::~ServiceProvider()
{
}

void ServiceProvider::addSubscriber(uint16_t eventgroupId, Endpoint endpoint)
{
}

void ServiceProvider::delSubscriber(uint16_t eventgroupId, Endpoint endpoint)
{
}
			
void ServiceProvider::notify(uint16_t eventId, std::shared_ptr<Payload>)
{
}
			
void ServiceProvider::response(uint16_t methodId, uint32_t request, std::shared_ptr<Payload>)
{
}
			
void ServiceProvider::setRequestReceiveHandler(uint16_t methodId, RequestReceiveHandler handler)
{
}

void ServiceProvider::unsetRequestReceiveHandler(uint16_t methodId)
{
}
			
void ServiceProvider::onMessage(NetWorkBindingType type, std::shared_ptr<Message> msg)
{
}
			
} // namespace com
} // namespace ara
