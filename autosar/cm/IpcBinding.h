/*
 * Copyright (c) 2017, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2017-12-14
 * Author: ryan
 */
 
#ifndef ARA_COM_IPCBINDING_H_
#define ARA_COM_IPCBINDING_H_

#include "DataTypes.h"
#include "BaseNetworkBinding.h"

#include <zmq.hpp>

#include <map>

namespace ara
{
	namespace com
	{
		class IpcEndpoint
		{
		public:
			IpcEndpoint(){}
			~IpcEndpoint(){}
			std::shared_ptr<Endpoint> m_server;
			std::vector<std::shared_ptr<Endpoint>> m_client;
			std::shared_ptr<Endpoint> m_multicast;
			bool m_isServer;
		};
		
		class IpcBinding : public BaseNetworkBinding
		{
		public:
			IpcBinding(uint16_t serviceId, uint16_t instanceId, std::shared_ptr<IpcEndpoint> endpoint);
			~IpcBinding();
			
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
			MessageReceiveHandler m_handler;
			std::shared_ptr<IpcEndpoint> m_endpoint;
			std::shared_ptr<zmq::context_t> m_context;
			std::shared_ptr<zmq::socket_t> m_PUB_SUB;
			std::map<uint16_t,std::shared_ptr<zmq::socket_t>> m_REQs;
			std::map<uint16_t,std::shared_ptr<zmq::socket_t>> m_REPs;
			std::map<uint16_t,std::vector<std::shared_ptr<Endpoint>>> m_eventgroupSubscribers;
			
			std::shared_ptr<zmq::message_t> buildMessage(std::shared_ptr<Message> msg);
			std::shared_ptr<Message> parseMessage(std::shared_ptr<zmq::message_t> msg);
		};
	} // namespace com
} // namespace ara

#endif // ARA_COM_IPCBINDING_H_
