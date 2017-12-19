/*
 * Copyright (c) 2017, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2017-12-14
 * Author: ryan
 */
 
#ifndef ARA_COM_SERVICEPROVIDER_H_
#define ARA_COM_SERVICEPROVIDER_H_

#include "DataTypes.h"
#include "Configuration.h"

namespace ara
{
	namespace com
	{
		class ServiceProvider
		{
			friend class ManagementFactory;
			
			ServiceProvider(uint16_t serviceId, uint16_t instanceId, Configuration *conf);
			
		public:
			virtual ~ServiceProvider();
			
			void addSubscriber(uint16_t eventgroupId, Endpoint endpoint);
			void delSubscriber(uint16_t eventgroupId, Endpoint endpoint);
			
			void notify(uint16_t eventId, std::shared_ptr<Payload>);
			
			void response(uint16_t methodId, uint32_t request, std::shared_ptr<Payload>);
			
			void setRequestReceiveHandler(uint16_t methodId, RequestReceiveHandler handler);
			void unsetRequestReceiveHandler(uint16_t methodId);
			
			void onMessage(NetWorkBindingType type, std::shared_ptr<Message> msg);
			
		};
	} // namespace com
} // namespace ara

#endif // ARA_COM_SERVICEPROVIDER_H_