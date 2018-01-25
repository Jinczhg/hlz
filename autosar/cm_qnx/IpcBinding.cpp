/*
 * Copyright (c) 2017, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2017-12-14
 * Author: ryan
 */
 
#include "IpcBinding.h"
#include "ManagementFactory.h"
#include "Serializer.h"
#include "Deserializer.h"

#include <iostream>
#include <sstream>
#include <thread>

#include <unistd.h>

namespace ara
{
namespace com
{

#define IPC_HEADER_LEN 22
#define IPC_HEADER_FIRST_LEN 8
#define IPC_HEADER_SECOND_LEN 14 //(IPC_HEADER_LEN - IPC_HEADER_FIRST_LEN)

IpcBinding::IpcBinding(uint16_t serviceId, uint16_t instanceId, std::shared_ptr<IpcEndpoint> endpoint)
: m_serviceId(serviceId), m_instanceId(instanceId), m_terminated(false), m_endpoint(endpoint), m_context(1), m_clientCount(0)
{	
	if (m_endpoint->m_isServer) //server
	{
		//reply
		for (auto ep : m_endpoint->m_client)
		{
			std::shared_ptr<zmq::socket_t> publisher = std::make_shared<zmq::socket_t>(m_context, ZMQ_PUB);
	
			std::stringstream ss_client;
			ss_client << "tcp://*:" << ep->getPort();
			
			ipv4_address_t ip = ep->getIp();
			
			uint32_t ipc_ip = ip[0];
			ipc_ip = (ipc_ip << 8) + ip[1];
			ipc_ip = (ipc_ip << 8) + ip[2];
			ipc_ip = (ipc_ip << 8) + ip[3];
			uint64_t ipc_id = ipc_ip;
			ipc_id = (ipc_id << 16) + ep->getPort();

			int sndhwm = 0;
			publisher->setsockopt(ZMQ_SNDHWM, &sndhwm, sizeof(sndhwm));
	
			publisher->bind(ss_client.str().c_str());
			
			m_REPs[m_clientCount] = publisher;
			m_subscriberStatus[m_clientCount] = false;
			m_clientIds[ipc_id] = m_clientCount;
			
			m_clientCount++;
		}
		
		//request
		{
			std::shared_ptr<Endpoint> ep =  m_endpoint->m_server[0];
			std::shared_ptr<zmq::socket_t> request = std::make_shared<zmq::socket_t>(m_context, ZMQ_REP);
			ipv4_address_t ip = ep->getIp();
			std::stringstream ss_server;
			ss_server << "tcp://*:" << ep->getPort();
	
			m_ip = ip[0];
			m_ip = (m_ip << 8) + ip[1];
			m_ip = (m_ip << 8) + ip[2];
			m_ip = (m_ip << 8) + ip[3];
			m_port =  ep->getPort();
			
			request->bind(ss_server.str().c_str());

			m_REQ = request;
			
			std::thread tr([this](){
				std::shared_ptr<zmq::message_t> message(new zmq::message_t);
				zmq::message_t reply(1);
				uint64_t ipc_id = 0;
				while (!this->isTerminated())
				{
					this->m_REQ->recv(message.get());
					this->m_REQ->send(reply);

					if (message->size() < IPC_HEADER_LEN)
					{
						continue;
					}
					
					std::shared_ptr<Message> msg = this->parseMessage(message);
					
					ipc_id = msg->getIp();
					ipc_id = (ipc_id << 16) + msg->getPort();
					
					auto it = m_clientIds.find(ipc_id);
					if (it == m_clientIds.end()) //invalid client
					{
						continue;
					}
					
					if (msg->getServiceId() == 0xFFFF) //
					{
						if (msg->getMethodId() != 0xFFFF) //MethodId == 0xFFFF as find service message, others as subscribe message
						{
							if (msg->getSession() == 0xFFFF) //subscribe
							{
								m_subscriberStatus[m_clientIds[ipc_id]] = true;
						
								for (auto em : m_eventMessages)
								{
									std::shared_ptr<zmq::message_t> m = buildMessage(em.second);
									publish(m_REPs[m_clientIds[ipc_id]], m);
								}
							}
							else //unsubscribe
							{
								m_subscriberStatus[m_clientIds[ipc_id]] = false;
							}
						}
						else //as find service message,init, unsubscribe
						{
							timeval active;
							gettimeofday(&active, NULL);
							m_clientActive[m_clientIds[ipc_id]] = active;
						}
					}
					else
					{	
						msg->setId(m_clientIds[ipc_id]);				
						this->onMessage(msg);
					}
				}
			});
			tr.detach();
		}
		
		//monitor thread
		std::thread tm([this](){
			while (!this->isTerminated())
			{
				this->checkClientActive();
				sleep(1);
			}
		});
		tm.detach();
	}
	else //client
	{	 
		//reply
		std::shared_ptr<Endpoint> ep = m_endpoint->m_client[0];
		std::shared_ptr<zmq::socket_t> subscriber = std::make_shared<zmq::socket_t>(m_context, ZMQ_SUB);
		ipv4_address_t ip = ep->getIp();
		std::stringstream ss_client;
		ss_client << "tcp://" << (int)ip[0] << "." << (int)ip[1] << "." << (int)ip[2] << "." << (int)ip[3] << ":" << ep->getPort();
		
		subscriber->connect(ss_client.str().c_str());
		subscriber->setsockopt(ZMQ_SUBSCRIBE, "", 0);
		
		m_ip = ip[0];
		m_ip = (m_ip << 8) + ip[1];
		m_ip = (m_ip << 8) + ip[2];
		m_ip = (m_ip << 8) + ip[3];
		m_port =  ep->getPort();
		uint64_t ipc_id = m_ip;
		ipc_id = (ipc_id << 16) + m_port;
		m_REPs[0] = subscriber;
		
		std::thread tr([this](){
			std::shared_ptr<zmq::message_t> message(new zmq::message_t);
			
			while (!this->isTerminated())
			{
				this->m_REPs[0]->recv(message.get());
				
				if (message->size() < IPC_HEADER_LEN)
				{
					continue;
				}
				
				std::shared_ptr<Message> msg = this->parseMessage(message);

				this->onMessage(msg);
			}
		});
		tr.detach();

		//request
		ep = m_endpoint->m_server[0];
	
		std::shared_ptr<zmq::socket_t> requester = std::make_shared<zmq::socket_t>(m_context, ZMQ_REQ);

		std::stringstream ss_server;
		ss_server << "tcp://" << (int)ip[0] << "." << (int)ip[1] << "." << (int)ip[2] << "." << (int)ip[3] << ":" << ep->getPort();

		requester->connect(ss_server.str().c_str());
		
		m_REQ = requester;
		
		std::shared_ptr<Message> initMsg(new Message);
		initMsg->setServiceId(0xFFFF);
		initMsg->setMethodId(0xFFFF);
		initMsg->setIp(m_ip);
		initMsg->setPort(m_port);
		
		request(m_REQ, initMsg);
		
		//monitor thread
		std::thread tm([this, requester, initMsg](){
			while (!this->isTerminated())
			{
				sleep(1);
				this->request(requester, initMsg);
			}
		});
		tm.detach();
	}
}

IpcBinding::~IpcBinding()
{
	m_terminated = true;
}

void IpcBinding::checkClientActive()
{
	timeval now;
	gettimeofday(&now, NULL);
	for (auto it : m_clientActive)
	{
		if (now.tv_sec - it.second.tv_sec > 3)
		{
			m_subscriberStatus[it.first] = false;
		}
	}
}

bool IpcBinding::isTerminated()
{
	return m_terminated;
}

std::shared_ptr<zmq::message_t> IpcBinding::buildMessage(std::shared_ptr<Message> msg)
{	
	Serializer serializer(ByteOrderEnum::BigEndian);
	
	std::shared_ptr<Payload> payload = msg->getPayload();
	
	uint32_t length = IPC_HEADER_SECOND_LEN + (payload ? payload->getSize() : 0);
	uint8_t protocol_version = 1;
	uint8_t interface_version = 1;
	uint8_t type = (uint8_t)msg->getType();
	uint8_t code = (uint8_t)msg->getCode();
	
	serializer.serialize(msg->getServiceId());
	serializer.serialize(msg->getMethodId());
	serializer.serialize(length);
	serializer.serialize(msg->getClientId());
	serializer.serialize(msg->getSession());
	serializer.serialize(msg->getIp());
	serializer.serialize(msg->getPort());
	serializer.serialize(protocol_version);
	serializer.serialize(interface_version);
	serializer.serialize(type);
	serializer.serialize(code);
	
	std::shared_ptr<zmq::message_t> message(new zmq::message_t(length + IPC_HEADER_FIRST_LEN));
	
	std::memcpy(message->data(), serializer.getData(), serializer.getSize());
	if (payload)
	{
		std::memcpy(static_cast<uint8_t*>(message->data()) + serializer.getSize(), payload->getData(), payload->getSize());
	}
	
	return message;
}

std::shared_ptr<Message> IpcBinding::parseMessage(std::shared_ptr<zmq::message_t> message)
{
	std::shared_ptr<Message> msg(new Message);
	
	uint8_t* data = static_cast<uint8_t*>(message->data());
	uint16_t serviceId = 0;
	uint16_t methodId = 0;
	uint32_t length = 0;
	uint16_t clientId = 0;
	uint16_t session = 0;
	uint32_t ip = 0;
	uint16_t port = 0;
	uint8_t protocol_version = 1;
	uint8_t interface_version = 1;
	uint8_t type;
	uint8_t code;
	
	Deserializer deserializer(ByteOrderEnum::BigEndian, data, IPC_HEADER_LEN);
	
	deserializer.deserialize(serviceId);
	deserializer.deserialize(methodId);
	deserializer.deserialize(length);
	deserializer.deserialize(clientId);
	deserializer.deserialize(session);
	deserializer.deserialize(ip);
	deserializer.deserialize(port);
	deserializer.deserialize(protocol_version);
	deserializer.deserialize(interface_version);
	deserializer.deserialize(type);
	deserializer.deserialize(code);
	
	msg->setServiceId(serviceId);
	msg->setInstanceId(m_instanceId);
	msg->setMethodId(methodId);
	msg->setClientId(clientId);
	msg->setSession(session);
	msg->setIp(ip);
	msg->setPort(port);
	msg->setType((MessageType)type);
	msg->setCode((ReturnCode)code);
	
	if (length - IPC_HEADER_SECOND_LEN > 0)
	{
		std::shared_ptr<Payload> payload(new Payload(length - IPC_HEADER_SECOND_LEN, data + IPC_HEADER_LEN));
		msg->setPayload(payload);
	}
	
	return msg;
}

bool IpcBinding::send(std::shared_ptr<Message> msg)
{	
	bool result = false;
	
	if (msg->getType() == MessageType::MT_NOTIFICATION) //event
	{	
		m_eventMessages[msg->getMethodId()] = msg;
		
    	std::shared_ptr<zmq::message_t> message = buildMessage(msg);
    	for (auto subscriber : m_subscriberStatus)
    	{
    		if (subscriber.second)
    		{
    			result = publish(m_REPs[subscriber.first], message);
    		}
    	}
	}
	else if (msg->getType() == MessageType::MT_REQUEST || msg->getType() == MessageType::MT_REQUEST_NO_RETURN) //method request
	{
		msg->setIp(m_ip);
		msg->setPort(m_port);
		result = request(m_REQ, msg);
	}
	else //method response or others
	{
		result = publish(m_REPs[msg->getId()], msg);
	}
	
	return result;
}

void IpcBinding::setReceiveHandler(MessageReceiveHandler handler)
{
	m_handler = handler;
}

void IpcBinding::onMessage(std::shared_ptr<Message> msg)
{
	if (m_handler)
	{
		m_handler(msg);
	}
}

void IpcBinding::subscribe(uint16_t eventgroupId)
{
	std::shared_ptr<Message> msg(new Message);
	
	msg->setServiceId(0xFFFF);
	msg->setMethodId(eventgroupId);
	msg->setSession(0xFFFF);
	msg->setIp(m_ip);
	msg->setPort(m_port);
	
	request(m_REQ, msg);
}

void IpcBinding::unsubscribe(uint16_t eventgroupId)
{
	std::shared_ptr<Message> msg(new Message);
	
	msg->setServiceId(0xFFFF);
	msg->setMethodId(eventgroupId);
	msg->setSession(0x0);
	msg->setIp(m_ip);
	msg->setPort(m_port);
	
	request(m_REQ, msg);
}

void IpcBinding::addSubscriber(uint16_t eventgroupId, std::shared_ptr<Endpoint> endpoint)
{
}

void IpcBinding::delSubscriber(uint16_t eventgroupId, std::shared_ptr<Endpoint> endpoint)
{
}

bool IpcBinding::publish(std::shared_ptr<zmq::socket_t> sock, std::shared_ptr<Message> msg)
{
	return publish(sock, buildMessage(msg));
}

bool IpcBinding::publish(std::shared_ptr<zmq::socket_t> sock, std::shared_ptr<zmq::message_t> message)
{
	std::lock_guard<std::mutex> guard(m_mutex);
	return sock->send(*(message.get()));
}

bool IpcBinding::request(std::shared_ptr<zmq::socket_t> sock, std::shared_ptr<Message> msg)
{
	return request(sock, buildMessage(msg));
}

bool IpcBinding::request(std::shared_ptr<zmq::socket_t> sock, std::shared_ptr<zmq::message_t> message)
{
	zmq::message_t reply;

	std::lock_guard<std::mutex> guard(m_mutex);
	
	sock->send(*(message.get()));
	sock->recv(&reply);
	
	return true;
}

} // namespace com
} // namespace ara
