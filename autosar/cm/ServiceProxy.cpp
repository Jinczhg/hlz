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

ServiceProxy::HandleType::HandleType(uint16_t serviceId, uint16_t instanceId, std::shared_ptr<Configuration> conf)
: m_serviceId(serviceId), m_instanceId(instanceId), m_conf(conf)
{
}

ServiceProxy::HandleType::~HandleType()
{
}
	
ServiceProxy::HandleType& ServiceProxy::HandleType::operator=(HandleType& other)
{
	m_serviceId = other.m_serviceId;
	m_instanceId = other.m_instanceId;
	m_conf = other.m_conf;
	
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

std::shared_ptr<Configuration> ServiceProxy::HandleType::getConf() const
{
	return m_conf;
}

			
ServiceProxy::ServiceProxy(ServiceProxy::HandleType handle)
: m_serviceId(handle.getServiceId()), m_instanceId(handle.getInstanceId()), m_handle(handle)
{
}

ServiceProxy::~ServiceProxy()
{
	ManagementFactory::get()->destroyServiceRequester(m_serviceId, m_instanceId);
}

ServiceHandleContainer<ServiceProxy::HandleType> ServiceProxy::FindService(uint16_t serviceId, InstanceIdentifier instance)
{
	ServiceHandleContainer<ServiceProxy::HandleType> container;
	
	return container;
}

FindServiceHandle ServiceProxy::StartFindService(FindServiceHandler<ServiceProxy::HandleType> handler, uint16_t serviceId, InstanceIdentifier instance)
{
	FindServiceHandle handle(serviceId, instance.getId());
	
	return handle;
}

void ServiceProxy::StopFindService(FindServiceHandle handle)
{
}

			
bool ServiceProxy::Init(std::shared_ptr<Configuration> conf)
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
