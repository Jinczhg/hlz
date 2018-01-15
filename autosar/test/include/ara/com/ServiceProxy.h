/*
 * Copyright (c) 2017, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2017-12-14
 * Author: ryan
 */

#ifndef ARA_COM_SERVICEPROXY_H_
#define ARA_COM_SERVICEPROXY_H_

#include "DataTypes.h"
#include "Configuration.h"

namespace ara
{
	namespace com
	{		
		class ServiceProxy
		{
		public:
			class HandleType
			{
			public:
				HandleType(uint16_t serviceId, uint16_t instanceId, std::shared_ptr<Configuration> conf);
				virtual ~HandleType();
				
				HandleType& operator=(HandleType& other);
				
				uint16_t getServiceId() const;
				uint16_t getInstanceId() const;
				std::shared_ptr<Configuration> getConf() const;
				
			private:
				uint16_t m_serviceId;
				uint16_t m_instanceId;
				std::shared_ptr<Configuration> m_conf;
			};
			
			explicit ServiceProxy(HandleType handle);
			virtual ~ServiceProxy();
			
			static ServiceHandleContainer<HandleType> FindService(uint16_t serviceId, InstanceIdentifier instance);
			static FindServiceHandle StartFindService(FindServiceHandler<HandleType> handler, uint16_t serviceId, InstanceIdentifier instance);
			static void StopFindService(FindServiceHandle handle);
			
			uint16_t getServiceId() const;
			
			uint16_t getInstanceId() const;
		
		protected:
			bool Init(std::shared_ptr<Configuration> conf);
			
		private:
			uint16_t m_serviceId;
			uint16_t m_instanceId;
			HandleType m_handle;
		};
	} // namespace com
} // namespace ara

#endif // ARA_COM_SERVICEPROXY_H_
