/*
 * Copyright (c) 2017, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2017-12-14
 * Author: ryan
 */
 
#ifndef ARA_COM_BASENETWORKBINDING_H_
#define ARA_COM_BASENETWORKBINDING_H_

#include "DataTypes.h"

namespace ara
{
	namespace com
	{
		class BaseNetworkBinding
		{
		public:
			virtual bool send(std::shared_ptr<Message> msg) = 0;
			virtual void setReceiveHandler(MessageReceiveHandler handler) = 0;
			virtual void onMessage(std::shared_ptr<Message> msg) = 0;
			virtual void subscribe(uint16_t eventgroupId) = 0;
			virtual void unsubscribe(uint16_t eventgroupId) = 0;
			virtual void addSubscriber(uint16_t eventgroupId, std::shared_ptr<Endpoint> endpoint) = 0;
			virtual void delSubscriber(uint16_t eventgroupId, std::shared_ptr<Endpoint> endpoint) = 0;
		};
	} // namespace com
} // namespace ara

#endif // ARA_COM_BASENETWORKBINDING_H_
