/*
 * Copyright (c) 2017, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2017-12-14
 * Author: ryan
 */
 
#ifndef ARA_COM_SOMEIPBINDING_H_
#define ARA_COM_SOMEIPBINDING_H_

#include "DataTypes.h"
#include "BaseNetworkBinding.h"

#include <vsomeip/SomeIPManager.hpp>

#include <map>

namespace ara
{
	namespace com
	{
		class SomeIpEndpoint
		{
			std::shared_ptr<Endpoint> m_server;
			std::shared_ptr<Endpoint> m_client;
			std::shared_ptr<Endpoint> m_multicast;
			bool m_isServer;
		};
		
		class SomeIpBinding : public BaseNetworkBinding
		{
		public:
			SomeIpBinding(uint16_t serviceId, uint16_t instanceId, std::shared_ptr<SomeIpEndpoint> someIpEndpoint);
			~SomeIpBinding();
			
			virtual bool send(std::shared_ptr<Message> msg);
			virtual void setReceiveHandler(MessageReceiveHandler handler);
			virtual void onMessage(std::shared_ptr<Message> msg);
			virtual void subscribe(uint16_t eventgroupId);
			virtual void unsubscribe(uint16_t eventgroupId);
			virtual void addSubscriber(uint16_t eventgroupId, std::shared_ptr<Endpoint> endpoint);
			virtual void delSubscriber(uint16_t eventgroupId, std::shared_ptr<Endpoint> endpoint);
			
		private:
			uint16_t m_serviceId;
			uint16_t m_instanceId;
			uint16_t m_messageId;
			std::shared_ptr<SomeIpEndpoint> m_someIpEndpoint;
			MessageReceiveHandler m_handler;
			std::map<uint16_t,std::vector<std::shared_ptr<Endpoint>>> m_eventgroupSubscribers;
			std::shared_ptr<vsomeip::SomeIPManager> m_someIpManager;
		};
	} // namespace com
} // namespace ara

#endif // ARA_COM_SOMEIPBINDING_H_
