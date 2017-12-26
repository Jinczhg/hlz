/*
 * Copyright (c) 2017, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2017-12-14
 * Author: ryan
 */
 
#ifndef ARA_COM_SDCLIENT_H_
#define ARA_COM_SDCLIENT_H_

#include "SomeIpSdIpc.h"

namespace ara
{
	namespace com
	{	
		class SdClient
		{
			SdClient();
		public:
			virtual ~SdClient();
			
			static SdClient* get();
			bool send();
			bool onMessae(std::shared_ptr<SdMessage> msg);
		
		private:
			int m_sd;
			std::shared_ptr<SomeIpSdIpc> m_server;
		};
	} // namespace com
} // namespace ara

#endif // ARA_COM_SDCLIENT_H_
