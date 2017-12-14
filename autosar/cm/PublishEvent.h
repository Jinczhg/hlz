/*
 * Copyright (c) 2017, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2017-12-14
 * Author: ryan
 */
 
#ifndef ARA_COM_PUBLISHEVENT_H_
#define ARA_COM_PUBLISHEVENT_H_

#include "ServiceSkeleton.h"

namespace ara
{
	namespace com
	{
		class PublishEvent
		{
		public:
			PublishEvent(ServiceSkeleton* skeleton, uint16_t eventId);
			virtual ~PublishEvent();
			
			void Send(uint8_t *data, uint32_t len);
		
		private:
			ServiceSkeleton* m_owner;
			uint16_t m_eventId;
		};
	} // namespace com
} // namespace ara

#endif // ARA_COM_PUBLISHEVENT_H_
