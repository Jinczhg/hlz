/*
 * Copyright (c) 2017, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2017-12-14
 * Author: ryan
 */
 
#ifndef ARA_COM_SUBSCRIBEEVENT_H_
#define ARA_COM_SUBSCRIBEEVENT_H_

#include "ServiceProxy.h"

#include <mutex>

namespace ara
{
	namespace com
	{
		class SubscribeEvent
		{
		public:
			SubscribeEvent(ServiceProxy* proxy, uint16_t eventId);
			virtual ~SubscribeEvent();
			
			void Subscribe(EventCacheUpdatePolicy policy, size_t cacheSize);
			void Unsubscribe();
			void Cleanup();
			void SetReceiveHandler(EventReceiveHandler handler);
			void UnsetReceiveHandler();
			
		protected:
			ServiceProxy* m_owner;
			uint16_t m_eventId;
			bool m_subscribed;
			EventCacheUpdatePolicy m_policy;
			size_t m_cacheSize;
			std::vector<std::shared_ptr<Payload>> m_data;
			EventReceiveHandler m_handler;
			std::mutex m_mutex;
		};
	} // namespace com
} // namespace ara

#endif // ARA_COM_SUBSCRIBEEVENT_H_
