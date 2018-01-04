/*
 * Copyright (c) 2017, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2017-12-14
 * Author: ryan
 */
 
#ifndef ARA_COM_METHOD_H_
#define ARA_COM_METHOD_H_

#include "DataTypes.h"
#include "ServiceProxy.h"

namespace ara
{
	namespace com
	{
		using MethodResponseHandler = std::function<void (std::shared_ptr<Payload>)>;
		
		class Method
		{
		public:
			Method(ServiceProxy* proxy, uint16_t methodId);
			~Method();
			
			bool operator()(std::shared_ptr<Payload> payload, MethodResponseHandler handler);
			
		protected:
			ServiceProxy* m_owner;
			uint16_t m_methodId;
		};
	} // namespace com
} // namespace ara

#endif // ARA_COM_METHOD_H_
