/*
 * Copyright (c) 2017, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2017-12-14
 * Author: ryan
 */
 
#include "ServiceSkeleton.h"
#include "ManagementFactory.h"
 
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <stdexcept>
#include <string>
#include <sstream>

namespace ara
{
namespace com
{
ServiceSkeleton::ServiceSkeleton(uint16_t serviceId, InstanceIdentifier instance, MethodCallProcessingMode mode)
:m_serviceId(serviceId), m_instanceId(instance.getId()), m_mode(mode)
{
	uint32_t key = (m_serviceId << 16) + m_instanceId;
	std::stringstream ss;
	ss << "/autosar_cm_" << key;
	std::string name = ss.str();
	
	if ((m_sem = sem_open(name.c_str(), O_CREAT | O_EXCL, S_IRWXU, 1)) == NULL)
	{
		throw std::runtime_error("exist the service instance");
	}

	if (SEM_FAILED == m_sem)
	{
		throw std::runtime_error("fail to create the service instance");
	}
}

ServiceSkeleton::~ServiceSkeleton()
{
	uint32_t key = (m_serviceId << 16) + m_instanceId;
	std::stringstream ss;
	ss << "/autosar_cm_" << key;
	std::string name = ss.str();
	
	sem_close(m_sem);
	sem_unlink(name.c_str());
	
	ManagementFactory::get()->destroyServiceProvider(m_serviceId, m_instanceId);
}
			
bool ServiceSkeleton::Init(Configuration* conf)
{
	try
	{
		ManagementFactory::get()->createServiceProvider(m_serviceId, m_instanceId, m_mode, conf);
	}
	catch(std::runtime_error& e)
	{
		return false;
	}
	
	return true;
}
			
void ServiceSkeleton::OfferService()
{
}

void ServiceSkeleton::StopOfferService()
{
}

ara::com::Future<bool> ServiceSkeleton::ProcessNextMethodCall()
{
	if (m_mode != MethodCallProcessingMode::kPoll)
	{
		throw std::runtime_error("ProcessNextMethodCall only available in polling mode");
	}
	
	ara::com::Promise<bool> p;
	auto f = p.get_future();
	
	std::thread t([this](ara::com::Promise<bool> p){
		ServiceProvider *sp = ManagementFactory::get()->getServiceProvider(this->getServiceId(), this->getInstanceId());
		sp->processRequest();
		p.set_value(sp->hasRequest());
	},
	std::move(p)
	);
	
	t.detach();
	
	return f;
}
			
uint16_t ServiceSkeleton::getServiceId() const
{
	return m_serviceId;
}
			
uint16_t ServiceSkeleton::getInstanceId() const
{
	return m_instanceId;
}
			
} // namespace com
} // namespace ara
