/*
 * Copyright (c) 2017, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2017-12-14
 * Author: ryan
 */
 
#include "DataTypes.h"

#include <algorithm>

namespace ara
{
namespace com
{
//InstanceIdentifier

const InstanceIdentifier InstanceIdentifier::Any("Any");

InstanceIdentifier::InstanceIdentifier(std::string value)
: m_value(value)
{
	std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) -> unsigned char { return std::toupper(c); });
	
	if (value == std::string("ANY"))
	{
		m_id = 0xFFFF;
	}
	else
	{
		m_id = atoi(value.c_str());
	}
}

InstanceIdentifier& InstanceIdentifier::operator=(const InstanceIdentifier& other)
{
	m_value = other.m_value;
	m_id = other.m_id;
	
	return *this;
}

bool InstanceIdentifier::operator== (const InstanceIdentifier& other) const
{
	return (this->m_id == other.m_id);
}

bool InstanceIdentifier::operator< (const InstanceIdentifier& other) const
{
	return (this->m_id < other.m_id);
}

std::string InstanceIdentifier::toString() const
{
	return m_value;
}

uint16_t InstanceIdentifier::getId() const
{
	return m_id;
}
//end InstanceIdentifier

//FindServiceHandle
FindServiceHandle::FindServiceHandle(uint16_t serviceId, uint16_t instanceId)
: m_serviceId(serviceId), m_instanceId(instanceId)
{
	m_id = (m_serviceId << 16) + m_instanceId;
}
        	
FindServiceHandle& FindServiceHandle::operator=(FindServiceHandle& other)
{
	m_serviceId = other.m_serviceId;
	m_instanceId = other.m_instanceId;
	m_id = other.m_id;
	
	return *this;
}

bool FindServiceHandle::operator==(FindServiceHandle& other) const
{
	return m_id == other.m_id;
}

bool FindServiceHandle::operator<(FindServiceHandle& other) const
{
	return m_id < other.m_id;
}

uint16_t FindServiceHandle::getServiceId() const
{
	return m_serviceId;
}

uint16_t FindServiceHandle::getInstanceId() const
{
	return m_instanceId;
}

uint32_t FindServiceHandle::getId() const
{
	return m_id;
}
//end FindServiceHandle

//Payload
Payload::Payload(uint32_t size, uint8_t *data)
: m_size(size), m_data(new uint8_t[size])
{
	std::memcpy(m_data, data, m_size);
}

Payload::~Payload()
{
	delete[] m_data;
}

uint32_t Payload::getSize() const
{
	return m_size;
}

uint8_t* Payload::getData() const
{
	return m_data;
}
//end Payload
 
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
