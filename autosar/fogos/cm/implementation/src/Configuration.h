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
			
			void setServerEndpoint(std::vector<std::shared_ptr<Endpoint>> serverEndpoint);
			void setClientEndpoint(std::vector<std::shared_ptr<Endpoint>> clientEndpoint);
			void setMulticastEndpoint(std::shared_ptr<Endpoint> multicastEndpoint);
			void setNetWorkBindingType(NetWorkBindingType type);
			
			std::vector<std::shared_ptr<Endpoint>> getServerEndpoint() const;
			std::vector<std::shared_ptr<Endpoint>> getClientEndpoint() const;
			std::shared_ptr<Endpoint> getMulticastEndpoint() const;
			NetWorkBindingType getNetWorkBindingType() const;
			
		private:
			std::vector<std::shared_ptr<Endpoint>> m_serverEndpoint;
			std::vector<std::shared_ptr<Endpoint>> m_clientEndpoint;
			std::shared_ptr<Endpoint> m_multicastEndpoint;
			
			NetWorkBindingType m_networkBindingType;
		};
	} // namespace com
} // namespace ara

#endif // ARA_COM_CONFIGURATION_H_
