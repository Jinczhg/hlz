/*
 * Copyright (c) 2017, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2017-12-14
 * Author: ryan
 */
 
#include "SomeIpSdIpc.h"

#include <thread>
#include <functional>
#include <memory>
#include <unistd.h>

namespace ara
{
namespace com
{

SomeIpSdIpc::SomeIpSdIpc(int sd, std::function<void(std::shared_ptr<SdMessage>)> messageHandler, std::function<void(int)> errorHandler)
: m_sd(sd), m_messageHandler(messageHandler), m_errorHandler(errorHandler), m_terminated(false)
{
	std::thread t([this](){
		char buf[4096];
		IpcMessage *ipc;
		ssize_t n;
		
		while (!m_terminated)
		{
			ipc = (IpcMessage *)buf;
			n = read(m_sd, buf, 4096);
			if (n > 0)
			{
				while(n > 0)
				{
					if (ipc->len < n)
					{
						std::shared_ptr<SdMessage> msg(new SdMessage(m_sd));		
						msg->parseData(ipc->data, ipc->len - sizeof(ipc->len));
						if (this->m_messageHandler)
						{
							m_messageHandler(msg);
						}
					}
					
					ipc = ipc + ipc->len;
					n -= ipc->len;
				}
			}
			else
			{
				if (errno == EINTR)
				{
					continue;
				}
				if (this->m_errorHandler)
				{
					m_errorHandler(this->m_sd);
				}
				m_terminated = true;
			}
		}
	});
	
	t.detach();
}
			
SomeIpSdIpc::~SomeIpSdIpc()
{
	m_terminated = true;
}

bool SomeIpSdIpc::send(std::shared_ptr<SdMessage> message)
{
	char buf[4096];
	IpcMessage *ipc = (IpcMessage *)buf;
	ipc->len = sizeof(ipc->len);
	
	ipc->len += message->getData(ipc->data);
	
	return ipc->len == write(m_sd, buf, ipc->len);
}
			
} // namespace com
} // namespace ara
