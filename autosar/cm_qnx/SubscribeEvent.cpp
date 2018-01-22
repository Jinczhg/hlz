/*
 * Copyright (c) 2017, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2017-12-14
 * Author: ryan
 */
 
 #include "SubscribeEvent.h"
 #include "ServiceRequester.h"
 #include "ManagementFactory.h"
 
namespace ara
{
namespace com
{

SubscribeEvent::SubscribeEvent(ServiceProxy* proxy, uint16_t eventId)
: m_owner(proxy), m_eventId(eventId)
{
}

SubscribeEvent::~SubscribeEvent()
{
}

void SubscribeEvent::Subscribe(EventCacheUpdatePolicy policy, size_t cacheSize)
{
	m_policy = policy;
	m_cacheSize = cacheSize;
	
	ServiceRequester *sr = ManagementFactory::get()->getServiceRequester(m_owner->getServiceId(), m_owner->getInstanceId());
	
	sr->setEventReceiveHandler(m_eventId, [this](std::shared_ptr<Payload> payload){
		{
			std::lock_guard<std::mutex> guard(this->m_mutex);
		
			this->m_data.push_back(payload);
		
			if (this->m_data.size() > this->m_cacheSize)
			{
				this->m_data.erase(this->m_data.begin());
			}
		}
		
		if (this->m_handler)
		{
			this->m_handler();
		}
	});
	
	sr->subscribe(m_eventId);
}

void SubscribeEvent::Unsubscribe()
{
	ServiceRequester *sr = ManagementFactory::get()->getServiceRequester(m_owner->getServiceId(), m_owner->getInstanceId());
	sr->unsubscribe(m_eventId);
}

void SubscribeEvent::Cleanup()
{
	if (m_policy == EventCacheUpdatePolicy::kNewestN)
	{
		std::lock_guard<std::mutex> guard(m_mutex);
		m_data.clear();
	}
}

void SubscribeEvent::SetReceiveHandler(EventReceiveHandler handler)
{
	m_handler = handler;
}

void SubscribeEvent::UnsetReceiveHandler()
{
	m_handler = {};
}
	
} // namespace com
} //namespace ara
