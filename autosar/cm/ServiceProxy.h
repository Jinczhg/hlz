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
			class HandleType;
			
			explicit ServiceProxy(HandleType handle);
			virtual ~ServiceProxy();
			
			bool Init(Configuration* conf);
			
			uint16_t getServiceId() const;
			
			uint16_t getInstanceId() const;
			
		private:
			uint16_t m_serviceId;
			uint16_t m_instanceId;
		};
	} // namespace com
} // namespace ara

#endif // ARA_COM_SERVICEPROXY_H_
