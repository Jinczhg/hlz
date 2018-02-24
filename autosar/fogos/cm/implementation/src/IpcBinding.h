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
#include <mutex>

#include <sys/time.h>

namespace ara
{
	namespace com
	{		
		class IpcEndpoint
		{
		public:
			IpcEndpoint(){}
			~IpcEndpoint(){}
			std::vector<std::shared_ptr<Endpoint>> m_server;
			std::vector<std::shared_ptr<Endpoint>> m_client;
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
			
			bool publish(std::shared_ptr<zmq::socket_t> sock, std::shared_ptr<Message> msg);
			bool publish(std::shared_ptr<zmq::socket_t> sock, std::shared_ptr<zmq::message_t> message);
			bool request(std::shared_ptr<zmq::socket_t> sock, std::shared_ptr<Message> msg);
			bool request(std::shared_ptr<zmq::socket_t> sock, std::shared_ptr<zmq::message_t> message);
			bool isTerminated();
			void checkClientActive();
			void addClient(uint64_t ipc_id);
			
		private:
			uint16_t m_serviceId;
			uint16_t m_instanceId;
			uint16_t m_messageId;
			bool m_terminated;
			MessageReceiveHandler m_handler;
			std::shared_ptr<IpcEndpoint> m_endpoint;
			zmq::context_t m_context;
			std::shared_ptr<zmq::socket_t> m_REP[2]; //ipc and tcp
			std::shared_ptr<zmq::socket_t> m_REQ;
			std::map<uint64_t,std::shared_ptr<zmq::socket_t>> m_PUBs;
			std::shared_ptr<zmq::socket_t> m_SUB;
			std::map<uint16_t,std::shared_ptr<Message>> m_eventMessages;
			std::map<uint16_t,bool> m_subscriberStatus;
			std::map<uint16_t,std::map<uint16_t,bool>> m_subscriberEventStatus;
			uint32_t m_ip;
			uint16_t m_port;
			std::map<uint64_t,uint16_t> m_clientIds;
			std::map<uint16_t,timeval> m_clientActive;
			int m_clientCount;
			std::string m_protocol;
			
			std::mutex m_mutex;
			
			std::shared_ptr<zmq::message_t> buildMessage(std::shared_ptr<Message> msg);
			std::shared_ptr<Message> parseMessage(std::shared_ptr<zmq::message_t> msg);
			
			void serverStartup();
			
			void clientStartup();
		};
	} // namespace com
} // namespace ara

#endif // ARA_COM_IPCBINDING_H_
