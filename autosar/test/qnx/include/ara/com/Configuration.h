/*
 * Copyright (c) 2017, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2017-12-14
 * Author: ryan
 */
 
#ifndef ARA_COM_CONFIGURATION_H_
#define ARA_COM_CONFIGURATION_H_

#include "DataTypes.h"

namespace ara
{
	namespace com
	{
		class Configuration
		{
		public:
			Configuration();
			virtual ~Configuration();
			
			void setServerEndpoint(std::shared_ptr<Endpoint> serverEndpoint);
			void setClientEndpoint(std::shared_ptr<Endpoint> clientEndpoint);
			void setMulticastEndpoint(std::shared_ptr<Endpoint> multicastEndpoint);
			
			std::shared_ptr<Endpoint> getServerEndpoint() const;
			std::shared_ptr<Endpoint> getClientEndpoint() const;
			std::shared_ptr<Endpoint> getMulticastEndpoint() const;
			
		private:
			std::shared_ptr<Endpoint> m_serverEndpoint;
			std::shared_ptr<Endpoint> m_clientEndpoint;
			std::shared_ptr<Endpoint> m_multicastEndpoint;
		};
	} // namespace com
} // namespace ara

#endif // ARA_COM_CONFIGURATION_H_
