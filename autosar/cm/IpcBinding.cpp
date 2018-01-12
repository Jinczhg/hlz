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

namespace ara
{
namespace com
{

IpcBinding::IpcBinding(uint16_t serviceId, uint16_t instanceId, std::shared_ptr<IpcEndpoint> endpoint)
: m_serviceId(serviceId), m_instanceId(instanceId), m_endpoint(endpoint)
{
	//TODO
	m_context = std::make_shared<zmq::context_t>(1);
	
	if (m_endpoint->m_isServer) //server
	{
		//publish
		m_PUB_SUB = std::make_shared<zmq::socket_t>(*m_context.get(), ZMQ_PUB);
	
		std::stringstream ss_multicast;
		ss_multicast << "tcp://*:" << m_endpoint->m_multicast->getPort();
		
		int sndhwm = 0;
    	m_PUB_SUB->setsockopt(ZMQ_SNDHWM, &sndhwm, sizeof(sndhwm));
	
		m_PUB_SUB->bind(ss_multicast.str().c_str());
		
		//request
		for (auto ep : m_endpoint->m_server)
		{
			std::shared_ptr<zmq::socket_t> subscriber = std::make_shared<zmq::socket_t>(*m_context.get(), ZMQ_SUB);
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
					
					msg->setId(id);
					
					this->onMessage(msg);
				}
			});
			tr.detach();
		}
		
		//reply
		for (auto ep : m_endpoint->m_client)
		{
			std::shared_ptr<zmq::socket_t> publisher = std::make_shared<zmq::socket_t>(*m_context.get(), ZMQ_PUB);
	
			std::stringstream ss_client;
			ss_client << "tcp://*:" << ep->getPort();
		
			sndhwm = 0;
			publisher->setsockopt(ZMQ_SNDHWM, &sndhwm, sizeof(sndhwm));
	
			publisher->bind(ss_client.str().c_str());
			
			m_REPs.push_back(publisher);
		}
	}
	else //client
	{
		//subscribe
		m_PUB_SUB = std::make_shared<zmq::socket_t>(*m_context.get(), ZMQ_SUB);
	
		ipv4_address_t ip = m_endpoint->m_multicast->getIp();
		std::stringstream ss_multicast;
		ss_multicast << "tcp://" << (int)ip[0] << "." << (int)ip[1] << "." << (int)ip[2] << "." << (int)ip[3] << ":" << m_endpoint->m_multicast->getPort();
	
		std::cout << ss_multicast.str() << std::endl;
		
		m_PUB_SUB->connect(ss_multicast.str().c_str());
		m_PUB_SUB->setsockopt(ZMQ_SUBSCRIBE, "", 0);
		
		std::thread t([this](){
			while (1)
			{
				std::shared_ptr<zmq::message_t> message(new zmq::message_t);
    			this->m_PUB_SUB->recv(message.get());
    			
    			std::shared_ptr<Message> msg = this->parseMessage(message);
    			
    			this->onMessage(msg);
			}
		});
		t.detach();
		
		//request
		std::shared_ptr<Endpoint> ep = m_endpoint->m_client[0];
		std::shared_ptr<zmq::socket_t> subscriber = std::make_shared<zmq::socket_t>(*m_context.get(), ZMQ_SUB);
		ip = ep->getIp();
		std::stringstream ss_client;
		ss_client << "tcp://" << (int)ip[0] << "." << (int)ip[1] << "." << (int)ip[2] << "." << (int)ip[3] << ":" << ep->getPort();

		std::cout << ss_client.str() << std::endl;

		subscriber->connect(ss_client.str().c_str());
		subscriber->setsockopt(ZMQ_SUBSCRIBE, "", 0);
		
		uint16_t id = m_REQs.size();
		m_REPs.push_back(subscriber);
		
		std::thread tr([this,id](){
			while (1)
			{
				std::shared_ptr<zmq::message_t> message(new zmq::message_t);
				this->m_REPs[id]->recv(message.get());
				
				std::shared_ptr<Message> msg = this->parseMessage(message);
				
				msg->setId(id);
				
				this->onMessage(msg);
			}
		});
		tr.detach();
		
		
		//reply
		ep = m_endpoint->m_server[0];
	
		std::shared_ptr<zmq::socket_t> publisher = std::make_shared<zmq::socket_t>(*m_context.get(), ZMQ_PUB);

		std::stringstream ss_server;
		ss_server << "tcp://*:" << ep->getPort();
		
		std::cout << ss_server.str() << std::endl;
	
		int sndhwm = 0;
		publisher->setsockopt(ZMQ_SNDHWM, &sndhwm, sizeof(sndhwm));

		publisher->bind(ss_server.str().c_str());
		
		m_REQs.push_back(publisher);
	}
}

IpcBinding::~IpcBinding()
{
}

std::shared_ptr<zmq::message_t> IpcBinding::buildMessage(std::shared_ptr<Message> msg)
{	
	Serializer serializer(ByteOrderEnum::BigEndian);
	
	uint32_t length = 8 + msg->getPayload()->getSize();
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
	std::memcpy(static_cast<uint8_t*>(message->data()) + serializer.getSize(), msg->getPayload()->getData(), msg->getPayload()->getSize());
	
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
	
	std::shared_ptr<Payload> payload(new Payload(length - 8, data + 16));
	
	message->setPayload(payload);
	
	return message;
}

bool IpcBinding::send(std::shared_ptr<Message> msg)
{	
	if (msg->getType() == MessageType::MT_NOTIFICATION) //event
	{
		std::cout << "notify:" << (char*)msg->getPayload()->getData() << std::endl;
		
		m_eventMessages[msg->getMethodId] = msg;
		
    	return m_PUB_SUB->send(*(buildMessage(msg).get()));
	}
	else if (msg->getType() == MessageType::MT_REQUEST || msg->getType() == MessageType::MT_REQUEST_NO_RETURN) //method request
	{
		std::cout << "method request:" << (char*)msg->getPayload()->getData() << std::endl;
		return m_REQs[0]->send(*(buildMessage(msg).get()));
	}
	else //method response or others
	{
		std::cout << "method response[" << (int)msg->getCode() << "]" << ":" << (char*)msg->getPayload()->getData() << std::endl;
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
