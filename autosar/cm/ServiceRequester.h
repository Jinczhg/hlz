/*
 * Copyright (c) 2017, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2017-12-14
 * Author: ryan
 */
 
#ifndef ARA_COM_SERVICEREQUESTER_H_
#define ARA_COM_SERVICEREQUESTER_H_

#include "DataTypes.h"
#include "Configuration.h"

namespace ara
{
	namespace com
	{
		class ServiceRequester
		{
			friend class ManagementFactory;
			
			ServiceRequester(uint16_t serviceId, uint16_t instanceId, Configuration *conf);
			
		public:
			virtual ~ServiceRequester();
			
			void subscribe(uint16_t eventId);
			
			void unsubscribe(uint16_t eventId);
			
			void setEventSubscribeHandler(uint16_t eventId, std::function<void(std::shared_ptr<PayLoad>)> handler);
			
			void onMessage(NetWorkBindingType type, Message msg);
			
		private:
			std::map<uint16_t, std::function<void(std::shared_ptr<PayLoad>)>> m_eventSubscribeHandlers;
		};
	} // namespace com
} // namespace ara

#endif // ARA_COM_SERVICEREQUESTER_H_
