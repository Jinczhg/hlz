/*
 * Copyright (c) 2017, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2017-12-14
 * Author: ryan
 */
 
#ifndef ARA_COM_SOMEIPSD_H_
#define ARA_COM_SOMEIPSD_H_

#include "DataTypes.h"
#include "Configuration.h"

namespace ara
{
	namespace com
	{
		class SomeIpSd
		{
			friend class ManagementFactory;
			
			SomeIpSd();
			
		public:
			virtual ~SomeIpSd();
		};
	} // namespace com
} // namespace ara

#endif // ARA_COM_SOMEIPSD_H_
