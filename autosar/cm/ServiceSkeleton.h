/*
 * Copyright (c) 2017, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2017-12-14
 * Author: ryan
 */

#ifndef ARA_COM_SERVICESKELETON_H_
#define ARA_COM_SERVICESKELETON_H_

#include "DataTypes.h"

namespace ara
{
	namespace com
	{
		class ServiceSkeleton
		{
		public:
			explicit ServiceSkeleton(InstanceIdentifier instance, MethodCallProcessingMode mode = MethodCallProcessingMode::kEvent);
			virtual ~ServiceSkeleton();
			
			bool Init(Configuration* conf);
			
			void OfferService();
			void StopOfferService();
			
			uint16_t getServiceId() const;
			
			uint16_t getInstanceId() const;
			
		private:
			uint16_t m_serviceId;
			uint16_t m_instanceId;
			MethodCallProcessingMode m_mode;
		};
	} // namespace com
} // namespace ara

#endif // ARA_COM_SERVICESKELETON_H_
