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
#include "BaseNetworkBinding.h"

#include <map>
#include <mutex>

namespace ara
{
	namespace com
	{
		using EventHandler = std::function<void (std::shared_ptr<Payload>)>;
		using ResponseHandler = std::function<void (std::shared_ptr<Message>)>;
		
		class ServiceRequester
		{
			friend class ManagementFactory;
			
			ServiceRequester(uint16_t serviceId, uint16_t instanceId, std::shared_ptr<Configuration> conf);
			
		public:
			virtual ~ServiceRequester();
			
			bool subscribe(uint16_t eventId);
			
			bool unsubscribe(uint16_t eventId);
			
			void setEventReceiveHandler(uint16_t eventId, EventHandler handler);
			
			void unsetEventReceiveHandler(uint16_t eventId);
			
			uint16_t request(uint16_t methodId, std::shared_ptr<Payload> payload, ResponseHandler handler);
			
			void cancelRequest(uint16_t session);
			
			void onMessage(NetWorkBindingType type, std::shared_ptr<Message> msg);
			
			BaseNetworkBinding* getNetworkBinding() const;
			
		private:
			std::map<uint16_t,EventHandler> m_eventHandlers;
			uint16_t m_serviceId;
			uint16_t m_instanceId;
			uint16_t m_clientId;
			uint16_t m_session;
			std::map<uint16_t,ResponseHandler> m_responseHandlers;
			BaseNetworkBinding *m_networkBinding;
			
			std::mutex m_mutex;
		};
	} // namespace com
} // namespace ara

#endif // ARA_COM_SERVICEREQUESTER_H_
