/*
 * Copyright (c) 2017, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2017-12-14
 * Author: ryan
 */
 
#ifndef ARA_COM_SDSERVER_H_
#define ARA_COM_SDSERVER_H_

#include "SomeIpSdIpc.h"

#include <map>

namespace ara
{
	namespace com
	{	
		class SdServer
		{
			SdServer();
			
		public:
			virtual ~SdServer();
			
			static SdServer* get();
			
			bool onMessae(std::shared_ptr<SdMessage> msg);
		
		private:
			int m_sd;
			bool m_terminated;
			std::map<int,std::shared_ptr<SomeIpSdIpc>> m_clients;
			std::map<uint32_t,std::shared_ptr<SomeIpSdIpc>> m_skeletons;
			std::map<uint32_t,std::map<int,std::shared_ptr<SomeIpSdIpc>>> m_proxys;
		};
	} // namespace com
} // namespace ara

#endif // ARA_COM_SDSERVER_H_
