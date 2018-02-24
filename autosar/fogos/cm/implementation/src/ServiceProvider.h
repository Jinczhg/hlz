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
#include "BaseNetworkBinding.h"

#include <map>
#include <mutex>
#include <condition_variable>

namespace ara
{
	namespace com
	{
		using RequestReceiveHandler = std::function<void (std::shared_ptr<Message>)>;
		
		class ServiceProvider
		{
			friend class ManagementFactory;
			
			ServiceProvider(uint16_t serviceId, uint16_t instanceId, MethodCallProcessingMode mode, std::shared_ptr<Configuration> conf);
			
		public:
			virtual ~ServiceProvider();
			
			void addSubscriber(uint16_t eventgroupId, std::shared_ptr<Endpoint> endpoint);
			void delSubscriber(uint16_t eventgroupId, std::shared_ptr<Endpoint> endpoint);
			
			void notify(uint16_t eventId, std::shared_ptr<Payload>);
			
			void response(uint16_t methodId, uint32_t request, std::shared_ptr<Payload>);
			
			void setRequestReceiveHandler(uint16_t methodId, RequestReceiveHandler handler);
			void unsetRequestReceiveHandler(uint16_t methodId);
			
			bool hasRequest();
			
			void processRequest();
			
			void onMessage(NetWorkBindingType type, std::shared_ptr<Message> msg);
			
			BaseNetworkBinding* getNetworkBinding() const;
			
		private:
			uint16_t m_serviceId;
			uint16_t m_instanceId;
			MethodCallProcessingMode m_mode;
			uint16_t m_clientId;
			uint16_t m_session;
			std::map<uint16_t,RequestReceiveHandler> m_handlers;
			std::vector<std::shared_ptr<Message>> m_requestMessages;
			std::mutex m_mutex;
			std::condition_variable m_condition;
			bool m_terminated;
			BaseNetworkBinding *m_networkBinding;
		};
	} // namespace com
} // namespace ara

#endif // ARA_COM_SERVICEPROVIDER_H_
