/*
 * Copyright (c) 2017, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2017-12-14
 * Author: ryan
 */
 
#ifndef ARA_COM_SOMEIPBINDING_H_
#define ARA_COM_SOMEIPBINDING_H_

#include "DataTypes.h"

namespace ara
{
	namespace com
	{
		class SomeIpBinding
		{
		public:
			SomeIpBinding();
			~SomeIpBinding();
			
			virtual bool send(std::shared_ptr<Message> msg);
			virtual void setReceiveHandler(MessageReceiveHandler handler);
			virtual void onMessage(std::shared_ptr<Message> msg);
			virtual void subscribe(uint16_t eventgroupId);
			virtual void unsubscribe(uint16_t eventgroupId);
			virtual void addSubscriber(uint16_t eventgroupId, Endpoint endpoint);
			virtual void delSubscriber(uint16_t eventgroupId, Endpoint endpoint);
		};
	} // namespace com
} // namespace ara

#endif // ARA_COM_SOMEIPBINDING_H_
