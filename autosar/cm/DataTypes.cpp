/*
 * Copyright (c) 2017, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2017-12-14
 * Author: ryan
 */
 
#include "DataTypes.h"

namespace ara
{
namespace com
{

// Message
void Message::setServiceId(uint16_t serviceId)
{
	m_serviceId = serviceId;
}

void Message::setInstanceId(uint16_t instanceId)
{
	m_instanceId = instanceId;
}

void Message::setClientId(uint16_t clientId)
{
	m_clientId = clientId;
}

void Message::setMethodId(uint16_t methodId)
{
	m_methodId = methodId;
}

void Message::setSession(uint16_t session)
{
	m_session = session;
}

void Message::setId(uint16_t id)
{
	m_id = id;
}
void Message::setType(MessageType type)
{
	m_type = type;
}

void Message::setCode(ReturnCode code)
{
	m_code = code;
}

void Message::setPayload(std::shared_ptr<Payload> payload)
{
	m_payload = payload;
}
        	
uint16_t Message::getServiceId()
{
	return m_serviceId;
}

uint16_t Message::getInstanceId()
{
	return m_instanceId;
}

uint16_t Message::getClientId()
{
	return m_clientId;
}

uint16_t Message::getMethodId()
{
	return m_methodId;
}

uint16_t Message::getSession()
{
	return m_session;
}

uint16_t Message::getId()
{
	return m_id;
}

MessageType Message::getType()
{
	return m_type;
}

ReturnCode Message::getCode()
{
	return m_code;
}

std::shared_ptr<Payload> Message::getPayload()
{
	return m_payload;
}

// end Message

// Endpoint
Endpoint::Endpoint(ipv4_address_t ip, uint16_t port, TransportProtocol protocol)
: m_ip(ip), m_port(port), m_protocol(protocol)
{
}

Endpoint::Endpoint(Endpoint& e)
{
	this->m_ip = e.m_ip;
	this->m_port = e.m_port;
	this->m_protocol = e.m_protocol;
}

Endpoint& Endpoint::operator=(Endpoint& e)
{
	this->m_ip = e.m_ip;
	this->m_port = e.m_port;
	this->m_protocol = e.m_protocol;
	
	return *this;
}

ipv4_address_t Endpoint::getIp() const
{
	return m_ip;
}

uint16_t Endpoint::getPort() const
{
	return m_port;
}

TransportProtocol Endpoint::getProtocol() const
{
	return m_protocol;
}

//end Endpoint

} // namespace com
} // namespace ara
