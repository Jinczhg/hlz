/*
 * Copyright (c) 2017, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2017-12-14
 * Author: ryan
 */
 
#include "ServiceRequester.h"

namespace ara
{
namespace com
{

ServiceRequester::ServiceRequester(uint16_t serviceId, uint16_t instanceId, Configuration *conf)
{
}
			
ServiceRequester::~ServiceRequester()
{
}

void ServiceRequester::subscribe(uint16_t eventId)
{
}
			
void ServiceRequester::unsubscribe(uint16_t eventId)
{
}
			
void ServiceRequester::setEventSubscribeHandler(uint16_t eventId, std::function<void(std::shared_ptr<Payload>)> handler)
{
}
			
void ServiceRequester::onMessage(NetWorkBindingType type, std::shared_ptr<Message> msg)
{
}
			
} // namespace com
} // namespace ara
