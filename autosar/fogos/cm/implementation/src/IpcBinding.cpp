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
	m_protocol = "ipc";
	
	if (m_endpoint->m_server[0]->getProtocol() == TransportProtocol::ipc)
	{
		m_protocol = "ipc";
	}
	else if (m_endpoint->m_server[0]->getProtocol() == TransportProtocol::tcp)
	{
		m_protocol = "tcp";
	}
	
	if (m_endpoint->m_isServer) //server
	{
		serverStartup();
	}
	else //client
	{	 
		clientStartup();
	}
}

IpcBinding::~IpcBinding()
{
	m_terminated = true;
}

void IpcBinding::serverStartup()
{
	//request
	std::shared_ptr<Endpoint> ep =  m_endpoint->m_server[0];
	std::shared_ptr<zmq::socket_t> ipc_request = std::make_shared<zmq::socket_t>(m_context, ZMQ_REP);
	std::shared_ptr<zmq::socket_t> tcp_request = std::make_shared<zmq::socket_t>(m_context, ZMQ_REP);
	ipv4_address_t ip = ep->getIp();
	std::stringstream ipc_server;
	std::stringstream tcp_server;

	ipc_server << "ipc://tmp/" << m_serviceId << "." << m_instanceId << ":" << ep->getPort();
	tcp_server << "tcp://*:" << ep->getPort();

	m_ip = ip[0];
	m_ip = (m_ip << 8) + ip[1];
	m_ip = (m_ip << 8) + ip[2];
	m_ip = (m_ip << 8) + ip[3];
	m_port =  ep->getPort();
	
	ipc_request->bind(ipc_server.str().c_str());
	tcp_request->bind(tcp_server.str().c_str());

	m_REP[0] = ipc_request;
	m_REP[1] = tcp_request;
	
	//ipc thread
	std::thread ipc_tr([this](){
		std::shared_ptr<zmq::message_t> message(new zmq::message_t);
		zmq::message_t reply(1);
		uint64_t ipc_id = 0;
		while (!this->isTerminated())
		{
			try
			{
				if (!this->m_REP[0]->recv(message.get()))
				{
					continue;
				}
				this->m_REP[0]->send(reply);
			}
			catch (std::exception &e)
			{
				std::cout << e.what() << std::endl;
				continue;
			}

			if (message->size() < IPC_HEADER_LEN)
			{
				continue;
			}
			
			std::shared_ptr<Message> msg = this->parseMessage(message);
			
			ipc_id = msg->getIp();
			ipc_id = (ipc_id << 16) + msg->getPort();
			ipc_id = ipc_id | 0x8000000000000000;
			
			auto it = this->m_clientIds.find(ipc_id);
			if (it == this->m_clientIds.end()) //not exist this client
			{
				this->addClient(ipc_id);
			}
			
			if (msg->getServiceId() == 0xFFFF) //
			{
				if (msg->getMethodId() != 0xFFFF) //MethodId == 0xFFFF as find service message, others as subscribe message
				{
					if (msg->getSession() == 0xFFFF) //subscribe
					{
						this->m_subscriberStatus[this->m_clientIds[ipc_id]] = true;
						auto it = this->m_eventMessages.find(msg->getMethodId());
						this->m_subscriberEventStatus[this->m_clientIds[ipc_id]][msg->getMethodId()] = true;
						
						if (it != this->m_eventMessages.end())
						{
							std::shared_ptr<zmq::message_t> m = this->buildMessage(it->second);
							this->publish(this->m_PUBs[this->m_clientIds[ipc_id]], m);
						}
					}
					else //unsubscribe
					{
						auto it = this->m_subscriberEventStatus[this->m_clientIds[ipc_id]].find(msg->getMethodId());
						if (it != this->m_subscriberEventStatus[this->m_clientIds[ipc_id]].end())
						{
							it->second = false;
						}
					}
				}
				else //as find service message,init, unsubscribe
				{
					timeval active;
					gettimeofday(&active, NULL);
					this->m_clientActive[this->m_clientIds[ipc_id]] = active;
				}
			}
			else
			{	
				msg->setId(this->m_clientIds[ipc_id]);				
				this->onMessage(msg);
			}
		}
	});
	ipc_tr.detach();
	
	//tcp thread
	std::thread tcp_tr([this](){
		std::shared_ptr<zmq::message_t> message(new zmq::message_t);
		zmq::message_t reply(1);
		uint64_t ipc_id = 0;
		while (!this->isTerminated())
		{
			try
			{
				if (!this->m_REP[1]->recv(message.get()))
				{
					continue;
				}
				this->m_REP[1]->send(reply);
			}
			catch (std::exception &e)
			{
				std::cout << e.what() << std::endl;
				continue;
			}

			if (message->size() < IPC_HEADER_LEN)
			{
				continue;
			}
			
			std::shared_ptr<Message> msg = this->parseMessage(message);
			
			ipc_id = msg->getIp();
			ipc_id = (ipc_id << 16) + msg->getPort();
			ipc_id = ipc_id & 0x7FFFFFFFFFFFFFFF;
			
			auto it = this->m_clientIds.find(ipc_id);
			if (it == this->m_clientIds.end()) //not exist this client
			{
				this->addClient(ipc_id);
			}
			
			if (msg->getServiceId() == 0xFFFF) //
			{
				if (msg->getMethodId() != 0xFFFF) //MethodId == 0xFFFF as find service message, others as subscribe message
				{
					if (msg->getSession() == 0xFFFF) //subscribe
					{
						this->m_subscriberStatus[this->m_clientIds[ipc_id]] = true;
						auto it = this->m_eventMessages.find(msg->getMethodId());
						this->m_subscriberEventStatus[this->m_clientIds[ipc_id]][msg->getMethodId()] = true;
						
						if (it != this->m_eventMessages.end())
						{
							std::shared_ptr<zmq::message_t> m = this->buildMessage(it->second);
							this->publish(this->m_PUBs[this->m_clientIds[ipc_id]], m);
						}
					}
					else //unsubscribe
					{
						auto it = this->m_subscriberEventStatus[this->m_clientIds[ipc_id]].find(msg->getMethodId());
						if (it != this->m_subscriberEventStatus[this->m_clientIds[ipc_id]].end())
						{
							it->second = false;
						}
					}
				}
				else //as find service message,init, unsubscribe
				{
					timeval active;
					gettimeofday(&active, NULL);
					this->m_clientActive[this->m_clientIds[ipc_id]] = active;
				}
			}
			else
			{	
				msg->setId(this->m_clientIds[ipc_id]);				
				this->onMessage(msg);
			}
		}
	});
	tcp_tr.detach();
	
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
			
void IpcBinding::clientStartup()
{
	//reply
	std::shared_ptr<Endpoint> ep = m_endpoint->m_client[0];
	std::shared_ptr<zmq::socket_t> subscriber = std::make_shared<zmq::socket_t>(m_context, ZMQ_SUB);
	ipv4_address_t ip = ep->getIp();
	std::stringstream ss_client;
	if (m_protocol == "ipc")
	{
		ss_client << m_protocol << "://tmp/" << m_serviceId << "." << m_instanceId << ":" << ep->getPort();
	}
	else
	{
		ss_client << m_protocol << "://" << (int)ip[0] << "." << (int)ip[1] << "." << (int)ip[2] << "." << (int)ip[3] << ":" << ep->getPort();
	}
	
	subscriber->connect(ss_client.str().c_str());
	subscriber->setsockopt(ZMQ_SUBSCRIBE, "", 0);
	
	m_ip = ip[0];
	m_ip = (m_ip << 8) + ip[1];
	m_ip = (m_ip << 8) + ip[2];
	m_ip = (m_ip << 8) + ip[3];
	m_port =  ep->getPort();
	uint64_t ipc_id = m_ip;
	ipc_id = (ipc_id << 16) + m_port;
	m_SUB = subscriber;
	
	std::thread tr([this](){
		std::shared_ptr<zmq::message_t> message(new zmq::message_t);
		
		while (!this->isTerminated())
		{
			this->m_SUB->recv(message.get());
			
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
	if (m_protocol == "ipc")
	{
		ss_server << m_protocol << "://tmp/" << m_serviceId << "." << m_instanceId << ":" << ep->getPort();
	}
	else
	{
		ss_server << m_protocol << "://" << (int)ip[0] << "." << (int)ip[1] << "." << (int)ip[2] << "." << (int)ip[3] << ":" << ep->getPort();
	}

	requester->connect(ss_server.str().c_str());
	
	m_REQ = requester;
	
	std::shared_ptr<Message> initMsg(new Message);
	initMsg->setServiceId(0xFFFF);
	initMsg->setMethodId(0xFFFF);
	initMsg->setIp(m_ip);
	initMsg->setPort(m_port);
	
	request(m_REQ, initMsg);
	
	//heartbeat thread
	std::thread tm([this, initMsg](){
		while (!this->isTerminated())
		{
			sleep(1);
			this->request(this->m_REQ, initMsg);
		}
	});
	tm.detach();
}

void IpcBinding::addClient(uint64_t ipc_id)
{
	if (m_clientIds.find(ipc_id) == m_clientIds.end())
	{
		std::shared_ptr<zmq::socket_t> publisher = std::make_shared<zmq::socket_t>(m_context, ZMQ_PUB);
		uint16_t port = ipc_id & 0xFFFF;
		std::stringstream ss_client;
		if (ipc_id & 0x8000000000000000)
		{
			ss_client << "ipc://tmp/" << this->m_serviceId << "." << this->m_instanceId << ":" << port;
		}
		else
		{
			ss_client << "tcp://*:" << port;
		}

		int sndhwm = 0;
		publisher->setsockopt(ZMQ_SNDHWM, &sndhwm, sizeof(sndhwm));

		publisher->bind(ss_client.str().c_str());
		
		//lock
		{
			std::lock_guard<std::mutex> guard(m_mutex);
			
			m_PUBs[m_clientCount] = publisher;
			m_subscriberStatus[m_clientCount] = false;
			m_clientIds[ipc_id] = m_clientCount;
			if (m_subscriberEventStatus.find(m_clientCount) != m_subscriberEventStatus.end())
			{
				m_subscriberEventStatus[m_clientCount].clear();
			}
		
			m_clientCount++;
		}
	}
}

void IpcBinding::checkClientActive()
{
	timeval now;
	gettimeofday(&now, NULL);
	for (auto it : m_clientActive)
	{
		if (now.tv_sec - it.second.tv_sec > 3)
		{
			auto subscriberStatus = m_subscriberStatus.find(it.first);
			if (subscriberStatus != m_subscriberStatus.end())
			{
				subscriberStatus->second = false;
			}
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
    		if (subscriber.second
    		&& m_subscriberEventStatus.find(subscriber.first) != m_subscriberEventStatus.end()
    		&& m_subscriberEventStatus[subscriber.first].find(msg->getMethodId()) != m_subscriberEventStatus[subscriber.first].end() 
    		&& m_subscriberEventStatus[subscriber.first][msg->getMethodId()])
    		{
    			result = publish(m_PUBs[subscriber.first], message);
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
		result = publish(m_PUBs[msg->getId()], msg);
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
