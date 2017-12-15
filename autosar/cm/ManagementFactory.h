/*
 * Copyright (c) 2017, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2017-12-14
 * Author: ryan
 */
 
#ifndef ARA_COM_MANAGEMENTFACTORY_H_
#define ARA_COM_MANAGEMENTFACTORY_H_

#include "ServiceProvider.h"
#include "ServiceRequester.h"
#include "SomeIpSd.h"

namespace ara
{
	namespace com
	{
		class ManagementFactory
		{
			std::map<uint32_t, ServiceProvider*> m_serviceProviders;
			std::map<uint32_t, ServiceRequester*> m_serviceRequesters;
			
			ManagementFactory();
			
		public:
			
			virtual ~ManagementFactory();
			
			static ManagementFactory* get();
			
			ServiceProvider* createServiceProvider(uint16_t serviceId, uint16_t instanceId, Configuration* conf);
			ServiceRequester* createServiceRequester(uint16_t serviceId, uint16_t instanceId, Configuration* conf);
			
			void destroyServiceProvider(uint16_t serviceId, uint16_t instanceId);
			void destroyServiceRequester(uint16_t serviceId, uint16_t instanceId);
			
			ServiceProvider* getServiceProvider(uint16_t serviceId, uint16_t instanceId);
			ServiceRequester* getServiceRequester(uint16_t serviceId, uint16_t instanceId);
			
			SomeIpSd* getSomeIpSd();
		};
	} // namespace com
} // namespace ara

#endif // ARA_COM_MANAGEMENTFACTORY_H_
