/*
 * Copyright (c) 2017, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2017-12-14
 * Author: ryan
 */
 
#ifndef ARA_COM_IPCBINDING_H_
#define ARA_COM_IPCBINDING_H_

#include "BaseNetworkBinding.h"

#include <thread>
#include <map>

namespace ara
{
	namespace com
	{
		class IpcBinding: public BaseNetworkBinding
		{
		public:
			IpcBinding(uint16_t serverPort);
			IpcBinding(uint16_t serverPort, uint16_t clientPort);
			virtual ~IpcBinding();
			virtual bool send(std::shared_ptr<Message> msg);
			virtual void setReceiveHandler(MessageReceiveHandler handler);
			virtual void onMessage(std::shared_ptr<Message> msg);
			virtual void subscribe(uint16_t eventgroupId);
			virtual void unsubscribe(uint16_t eventgroupId);
			virtual void addSubscriber(uint16_t eventgroupId, Endpoint endpoint);
			virtual void delSubscriber(uint16_t eventgroupId, Endpoint endpoint);
			
		private:
			bool m_isServer;
			int m_fd;
			std::thread m_acceptThread;
			std::map<int,std::thread> m_processThreads;
		};
	} // namespace com
} // namespace ara

#endif // ARA_COM_IPCBINDING_H_
