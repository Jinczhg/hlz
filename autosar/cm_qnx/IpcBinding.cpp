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

IpcBinding::IpcBinding(uint16_t serviceId, uint16_t instanceId, std::shared_ptr<IpcEndpoint> endpoint)
: m_serviceId(serviceId), m_instanceId(instanceId), m_endpoint(endpoint), m_context(1)
{
	//m_context = std::make_shared<zmq::context_t>(1);
	
	if (m_endpoint->m_isServer) //server
	{	
		//reply
		for (auto ep : m_endpoint->m_server)
		{
			std::shared_ptr<zmq::socket_t> publisher = std::make_shared<zmq::socket_t>(m_context, ZMQ_PUB);
	
			std::stringstream ss_client;
			ss_client << "tcp://*:" << ep->getPort();

			int sndhwm = 0;
			publisher->setsockopt(ZMQ_SNDHWM, &sndhwm, sizeof(sndhwm));
	
			publisher->bind(ss_client.str().c_str());
			
			uint16_t id = m_REPs.size();
			m_REPs.push_back(publisher);
			m_subscriberStatus[id] = false;
		}
		
		//request
		for (auto ep : m_endpoint->m_client)
		{
			std::shared_ptr<zmq::socket_t> subscriber = std::make_shared<zmq::socket_t>(m_context, ZMQ_SUB);
			ipv4_address_t ip = ep->getIp();
			std::stringstream ss_server;
			ss_server << "tcp://" << (int)ip[0] << "." << (int)ip[1] << "." << (int)ip[2] << "." << (int)ip[3] << ":" << ep->getPort();
	
			subscriber->connect(ss_server.str().c_str());
			subscriber->setsockopt(ZMQ_SUBSCRIBE, "", 0);
			
			uint16_t id = m_REQs.size();
			m_REQs.push_back(subscriber);
			
			std::thread tr([this,id](){
				while (1)
				{
					std::shared_ptr<zmq::message_t> message(new zmq::message_t);
					this->m_REQs[id]->recv(message.get());
					
					std::shared_ptr<Message> msg = this->parseMessage(message);
					
					if (msg->getServiceId() == 0xFFFF)
					{
						if (msg->getMethodId() != 0xFFFF)
						{
							m_subscriberStatus[id] = true;
						
							for (auto em : m_eventMessages)
							{
								std::shared_ptr<zmq::message_t> m = buildMessage(em.second);
								m_REPs[id]->send(*(m.get()));
							}
						}
						else
						{
							m_REPs[id]->send(*(message.get()));
						}
					}
					else
					{
						msg->setId(id);
					
						this->onMessage(msg);
					}
				}
			});
			tr.detach();
		}
	}
	else //client
	{
		
		std::shared_ptr<bool> connected(new bool);
		 
		//reply
		std::shared_ptr<Endpoint> ep = m_endpoint->m_server[0];
		std::shared_ptr<zmq::socket_t> subscriber = std::make_shared<zmq::socket_t>(m_context, ZMQ_SUB);
		ipv4_address_t ip = ep->getIp();
		std::stringstream ss_server;
		ss_server << "tcp://" << (int)ip[0] << "." << (int)ip[1] << "." << (int)ip[2] << "." << (int)ip[3] << ":" << ep->getPort();
		
		subscriber->connect(ss_server.str().c_str());
		subscriber->setsockopt(ZMQ_SUBSCRIBE, "", 0);
		
		uint16_t id = m_REPs.size();
		m_REPs.push_back(subscriber);
		
		std::thread tr([this,id,connected](){
			while (1)
			{
				std::shared_ptr<zmq::message_t> message(new zmq::message_t);
				this->m_REPs[id]->recv(message.get());
				
				std::shared_ptr<Message> msg = this->parseMessage(message);
				
				if (msg->getServiceId() == 0xFFFF && msg->getMethodId() == 0xFFFF)
				{
					*connected.get() = true;
				}
				else
				{
					msg->setId(id);
					this->onMessage(msg);
				}
			}
		});
		tr.detach();

		//request
		ep = m_endpoint->m_client[0];
	
		std::shared_ptr<zmq::socket_t> publisher = std::make_shared<zmq::socket_t>(m_context, ZMQ_PUB);

		std::stringstream ss_client;
		ss_client << "tcp://*:" << ep->getPort();
	
		int sndhwm = 0;
		publisher->setsockopt(ZMQ_SNDHWM, &sndhwm, sizeof(sndhwm));

		publisher->bind(ss_client.str().c_str());
		
		m_REQs.push_back(publisher);
		
		std::shared_ptr<Message> initMsg(new Message);
		initMsg->setServiceId(0xFFFF);
		initMsg->setMethodId(0xFFFF);
		
		std::shared_ptr<zmq::message_t> initIpcMsg = buildMessage(initMsg);
		
		m_REQs[0]->send(*(buildMessage(initMsg).get()));
		
		while (!(*connected.get()))
		{
			usleep(10000);
			m_REQs[0]->send(*(buildMessage(initMsg).get()));
		}
	}
}

IpcBinding::~IpcBinding()
{
}

std::shared_ptr<zmq::message_t> IpcBinding::buildMessage(std::shared_ptr<Message> msg)
{	
	Serializer serializer(ByteOrderEnum::BigEndian);
	
	std::shared_ptr<Payload> payload = msg->getPayload();
	
	uint32_t length = 8 + (payload ? payload->getSize() : 0);
	uint8_t protocol_version = 1;
	uint8_t interface_version = 1;
	uint8_t type = (uint8_t)msg->getType();
	uint8_t code = (uint8_t)msg->getCode();
	
	serializer.serialize(msg->getServiceId());
	serializer.serialize(msg->getMethodId());
	serializer.serialize(length);
	serializer.serialize(msg->getClientId());
	serializer.serialize(msg->getSession());
	serializer.serialize(protocol_version);
	serializer.serialize(interface_version);
	serializer.serialize(type);
	serializer.serialize(code);
	
	std::shared_ptr<zmq::message_t> message(new zmq::message_t(length + 8));
	
	std::memcpy(message->data(), serializer.getData(), serializer.getSize());
	if (payload)
	{
		std::memcpy(static_cast<uint8_t*>(message->data()) + serializer.getSize(), payload->getData(), payload->getSize());
	}
	
	return message;
}

std::shared_ptr<Message> IpcBinding::parseMessage(std::shared_ptr<zmq::message_t> msg)
{
	std::shared_ptr<Message> message(new Message);
	
	uint8_t* data = static_cast<uint8_t*>(msg->data());
	uint16_t serviceId = 0;
	uint16_t methodId = 0;
	uint32_t length = 0;
	uint16_t clientId = 0;
	uint16_t session = 0;
	uint8_t protocol_version = 1;
	uint8_t interface_version = 1;
	uint8_t type;
	uint8_t code;
	
	Deserializer deserializer(ByteOrderEnum::BigEndian, data, 16);
	
	deserializer.deserialize(serviceId);
	deserializer.deserialize(methodId);
	deserializer.deserialize(length);
	deserializer.deserialize(clientId);
	deserializer.deserialize(session);
	deserializer.deserialize(protocol_version);
	deserializer.deserialize(interface_version);
	deserializer.deserialize(type);
	deserializer.deserialize(code);
	
	message->setServiceId(serviceId);
	message->setInstanceId(m_instanceId);
	message->setMethodId(methodId);
	message->setClientId(clientId);
	message->setSession(session);
	message->setType((MessageType)type);
	message->setCode((ReturnCode)code);
	
	if (length - 8 > 0)
	{
		std::shared_ptr<Payload> payload(new Payload(length - 8, data + 16));
		message->setPayload(payload);
	}
	
	return message;
}

bool IpcBinding::send(std::shared_ptr<Message> msg)
{	
	if (msg->getType() == MessageType::MT_NOTIFICATION) //event
	{	
		m_eventMessages[msg->getMethodId()] = msg;
		
    	std::shared_ptr<zmq::message_t> message = buildMessage(msg);
    	for (auto subscriber : m_subscriberStatus)
    	{
    		if (subscriber.second)
    		{
    			m_REPs[subscriber.first]->send(*(message.get()));
    		}
    	}
	}
	else if (msg->getType() == MessageType::MT_REQUEST || msg->getType() == MessageType::MT_REQUEST_NO_RETURN) //method request
	{
		return m_REQs[0]->send(*(buildMessage(msg).get()));
	}
	else //method response or others
	{
		return m_REPs[msg->getId()]->send(*(buildMessage(msg).get()));
	}
	
	return true;
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
	std::shared_ptr<Message> message(new Message);
	
	message->setServiceId(0xFFFF);
	message->setMethodId(eventgroupId);
	
	m_REQs[0]->send(*(buildMessage(message).get()));
}

void IpcBinding::unsubscribe(uint16_t eventgroupId)
{
}

void IpcBinding::addSubscriber(uint16_t eventgroupId, std::shared_ptr<Endpoint> endpoint)
{
}

void IpcBinding::delSubscriber(uint16_t eventgroupId, std::shared_ptr<Endpoint> endpoint)
{
}

} // namespace com
} // namespace ara
