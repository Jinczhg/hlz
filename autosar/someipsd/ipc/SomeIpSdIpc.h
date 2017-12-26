/*
 * Copyright (c) 2017, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2017-12-14
 * Author: ryan
 */
 
#ifndef ARA_COM_SOMEIPSDIPC_H_
#define ARA_COM_SOMEIPSDIPC_H_

#include <cstdint>
#include <functional>
#include <memory>

#include "SdMessage.h"

namespace ara
{
	namespace com
	{
		struct IpcMessage
		{
			int len;
			char data[0];
		};
		
		class SomeIpSdIpc
		{
		public:
			SomeIpSdIpc(int sd, std::function<void(std::shared_ptr<SdMessage>)> messageHandler, std::function<void(int)> errorHandler);
			virtual ~SomeIpSdIpc();
			
			bool send(std::shared_ptr<SdMessage> message);
		
		private:
			int m_sd;
			uint16_t m_serviceId;
			uint16_t m_instanceId;
			std::function<void(std::shared_ptr<SdMessage>)> m_messageHandler;
			std::function<void(int)> m_errorHandler;
			bool m_terminated;
		};
	} // namespace com
} // namespace ara

#endif // ARA_COM_SOMEIPSDIPC_H_
