/*
 * Copyright (c) 2017, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2017-12-14
 * Author: ryan
 */
 
 #include "ManagementFactory.h"
 #include "ServiceRequester.h"
 #include "ServiceProvider.h"
 #include "SomeIpSd.h"
 
 #include <stdexcept>
 
namespace ara
{
namespace com
{

ManagementFactory::ManagementFactory()
{
}		
			
ManagementFactory::~ManagementFactory()
{
}
			
ManagementFactory* ManagementFactory::get()
{
	static ManagementFactory *s_instance = new ManagementFactory();
	
	return s_instance;
}
			
ServiceProvider* ManagementFactory::createServiceProvider(uint16_t serviceId, uint16_t instanceId, MethodCallProcessingMode mode, Configuration* conf)
{
	uint32_t key = (serviceId << 16) + instanceId;
	if (m_serviceProviders.find(key) != m_serviceProviders.end())
	{
		throw std::runtime_error("exist the service instance");
	}
	
	ServiceProvider *provider = new ServiceProvider(serviceId, instanceId, mode, conf);
	
	m_serviceProviders[key] = provider;
	
	return provider;
}

ServiceRequester* ManagementFactory::createServiceRequester(uint16_t serviceId, uint16_t instanceId, Configuration* conf)
{
	uint32_t key = (serviceId << 16) + instanceId;
	if (m_serviceRequesters.find(key) != m_serviceRequesters.end())
	{
		throw std::runtime_error("exist the service instance");
	}
	
	ServiceRequester *requester = new ServiceRequester(serviceId, instanceId, conf);
	
	m_serviceRequesters[key] = requester;
	
	return requester;
}
			
void ManagementFactory::destroyServiceProvider(uint16_t serviceId, uint16_t instanceId)
{
	uint32_t key = (serviceId << 16) + instanceId;
	auto m = m_serviceProviders.find(key);
	if (m != m_serviceProviders.end())
	{
		delete m->second;
		m_serviceProviders.erase(m);
	}
}

void ManagementFactory::destroyServiceRequester(uint16_t serviceId, uint16_t instanceId)
{
	uint32_t key = (serviceId << 16) + instanceId;
	auto m = m_serviceRequesters.find(key);
	if (m != m_serviceRequesters.end())
	{
		delete m->second;
		m_serviceRequesters.erase(m);
	}
}
			
ServiceProvider* ManagementFactory::getServiceProvider(uint16_t serviceId, uint16_t instanceId)
{
	uint32_t key = (serviceId << 16) + instanceId;
	auto m = m_serviceProviders.find(key);
	if (m != m_serviceProviders.end())
	{
		return m->second;
	}
	
	return NULL;
}

ServiceRequester* ManagementFactory::getServiceRequester(uint16_t serviceId, uint16_t instanceId)
{
	uint32_t key = (serviceId << 16) + instanceId;
	auto m = m_serviceRequesters.find(key);
	if (m != m_serviceRequesters.end())
	{
		return m->second;
	}
	
	return NULL;
}
			
SomeIpSd* ManagementFactory::getSomeIpSd()
{
	static SomeIpSd* s_someipSd = new SomeIpSd();
	
	return s_someipSd;
}
			
} // namespace com
} //namespace ara
