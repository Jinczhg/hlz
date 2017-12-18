/*
 * Copyright (c) 2017, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2017-12-14
 * Author: ryan
 */
 
 #include "PublishEvent.h"
 #include "ManagementFactory.h"
 
namespace ara
{
namespace com
{

PublishEvent::PublishEvent(ServiceSkeleton* skeleton, uint16_t eventId)
: m_owner(skeleton), m_eventId(eventId)
{
}

PublishEvent::~PublishEvent()
{
}

void PublishEvent::Send(uint8_t *data, uint32_t len)
{
	ServiceProvider *sp = ManagementFactory::get()->getServiceProvider(m_owner->getServiceId(), m_owner->getInstanceId());
	
	std::shared_ptr<Payload> payload(new Payload(len, data)); 
	
	sp->notify(m_eventId, payload);
}
			
} // namespace com
} //namespace ara
