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
#include <iostream>
 
#include <signal.h>
#include <semaphore.h>
 
namespace ara
{
namespace com
{

std::vector<sem_t*> g_sems;

void terminate_handler(int sig)
{	
	for (auto s : g_sems)
	{
		sem_post(s);
		sem_close(s);
	}
	
	if (sig == SIGINT)
	{
		exit(0);
	}
}

ManagementFactory::ManagementFactory()
{
	signal(SIGABRT, terminate_handler);
	signal(SIGINT, terminate_handler);
	signal(SIGSEGV, terminate_handler);
}		
			
ManagementFactory::~ManagementFactory()
{
}
			
ManagementFactory* ManagementFactory::get()
{
	static ManagementFactory *s_instance = new ManagementFactory();
	
	return s_instance;
}
			
ServiceProvider* ManagementFactory::createServiceProvider(uint16_t serviceId, uint16_t instanceId, MethodCallProcessingMode mode, std::shared_ptr<Configuration> conf)
{
	uint32_t key = (serviceId << 16) + instanceId;
	if (m_serviceProviders.find(key) != m_serviceProviders.end())
	{
		return m_serviceProviders[key];
	}
	
	ServiceProvider *provider = new ServiceProvider(serviceId, instanceId, mode, conf);
	
	m_serviceProviders[key] = provider;
	
	return provider;
}

ServiceRequester* ManagementFactory::createServiceRequester(uint16_t serviceId, uint16_t instanceId, std::shared_ptr<Configuration> conf)
{
	uint32_t key = (serviceId << 16) + instanceId;
	if (m_serviceRequesters.find(key) != m_serviceRequesters.end())
	{
		return m_serviceRequesters[key];
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
