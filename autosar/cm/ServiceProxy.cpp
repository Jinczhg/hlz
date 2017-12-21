/*
 * Copyright (c) 2017, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2017-12-14
 * Author: ryan
 */
 
#include "ServiceProxy.h"
#include "ManagementFactory.h"

namespace ara
{
namespace com
{

ServiceProxy::HandleType::HandleType(uint16_t serviceId, uint16_t instanceId, std::shared_ptr<Endpoint> endpoint)
: m_serviceId(serviceId), m_instanceId(instanceId), m_endpoint(endpoint)
{
}

ServiceProxy::HandleType::~HandleType()
{
}
	
ServiceProxy::HandleType& ServiceProxy::HandleType::operator=(HandleType& other)
{
	m_serviceId = other.m_serviceId;
	m_instanceId = other.m_instanceId;
	m_endpoint = other.m_endpoint;
	
	return *this;
}
			
uint16_t ServiceProxy::HandleType::getServiceId() const
{
	return m_serviceId;
}

uint16_t ServiceProxy::HandleType::getInstanceId() const
{
	return m_instanceId;
}

std::shared_ptr<Endpoint> ServiceProxy::HandleType::getEndpoit() const
{
	return m_endpoint;
}

			
ServiceProxy::ServiceProxy(ServiceProxy::HandleType handle)
: m_serviceId(handle.getServiceId()), m_instanceId(handle.getInstanceId()), m_handle(handle)
{
}

ServiceProxy::~ServiceProxy()
{
	ManagementFactory::get()->destroyServiceRequester(m_serviceId, m_instanceId);
}

			
bool ServiceProxy::Init(Configuration* conf)
{
	ManagementFactory::get()->createServiceRequester(m_serviceId, m_instanceId, conf);
	
	return true;
}
			
uint16_t ServiceProxy::getServiceId() const
{
	return m_serviceId;
}
			
uint16_t ServiceProxy::getInstanceId() const
{
	return m_instanceId;
}

} // namespace com
} // namespace ara
