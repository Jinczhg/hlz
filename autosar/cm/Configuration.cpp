/*
 * Copyright (c) 2017, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2017-12-14
 * Author: ryan
 */
 
#include "Configuration.h"
 
namespace ara
{
namespace com
{

Configuration::Configuration()
{
}

Configuration::~Configuration()
{
}
			
void Configuration::setServerEndpoint(std::vector<std::shared_ptr<Endpoint>> serverEndpoint)
{
	m_serverEndpoint = serverEndpoint;
}

void Configuration::setClientEndpoint(std::vector<std::shared_ptr<Endpoint>> clientEndpoint)
{
	m_clientEndpoint = clientEndpoint;
}

void Configuration::setMulticastEndpoint(std::shared_ptr<Endpoint> multicastEndpoint)
{
	m_multicastEndpoint = multicastEndpoint;
}

void Configuration::setNetWorkBindingType(NetWorkBindingType type)
{
	m_networkBindingType = type;
}

std::vector<std::shared_ptr<Endpoint>> Configuration::getServerEndpoint() const
{
	return m_serverEndpoint;
}

std::vector<std::shared_ptr<Endpoint>> Configuration::getClientEndpoint() const
{
	return m_clientEndpoint;
}

std::shared_ptr<Endpoint> Configuration::getMulticastEndpoint() const
{
	return m_multicastEndpoint;
}

NetWorkBindingType Configuration::getNetWorkBindingType() const
{
	return m_networkBindingType;
}

} // namespace com
} // namespace ara
