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
