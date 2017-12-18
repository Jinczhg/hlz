/*
 * Copyright (c) 2017, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2017-12-14
 * Author: ryan
 */
 
#include "IpcBinding.h"
#include "DataTypes.h"

#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <cstring>
#include <sstream>

namespace ara
{
namespace com
{

IpcBinding::IpcBinding(uint16_t serverPort)
: m_isServer(true), m_fd(-1)
{
	std::stringstream ss;
	ss << "autosar_cm_ipc_" << serverPort;
	
	struct sockaddr_un un;
	std::memset(&un, 0, sizeof(un));
	un.sun_family = AF_UNIX;
	std::strcpy(un.sun_path, ss.str().c_str());
	m_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (m_fd < 0)
	{
		throw std::runtime_error("socket error");
	}
	
	unlink(un.sun_path);
	
	if (bind(m_fd, (struct sockaddr*)&un, sizeof(un)) < 0)
	{
		throw std::runtime_error("socket error");
	}
	
	m_acceptThread = std::thread([this](){
		struct sockaddr_un cliaddr;
		socklen_t len = 0;
		int rxLen = 0;
		uint8_t rxBuf[1500];
		while (m_fd > 0)
		{
			if ((rxLen = recvfrom(this->m_fd, rxBuf, 1500, 0, (struct sockaddr*)&cliaddr, &len)) <= 0)
			{
				if (errno == EINTR)
				{
					continue;
				}
				else
				{
					break;
				}
			}
		}
	});
}

IpcBinding::IpcBinding(uint16_t serverPort, uint16_t clientPort)
: m_isServer(false), m_fd(-1)
{
}

IpcBinding::~IpcBinding()
{
	if (m_fd > 0)
	{
		close(m_fd);
		m_fd = -1;
	}
	
	m_acceptThread.join();
}

bool IpcBinding::send(std::shared_ptr<Message> msg)
{
	int len = sizeof(MessageHeader) + msg->m_header.m_len;
	
	return (write(m_fd, msg.get(), len) == len);
}

void IpcBinding::setReceiveHandler(MessageReceiveHandler handler)
{
}

void IpcBinding::onMessage(std::shared_ptr<Message> msg)
{
}

void IpcBinding::subscribe(uint16_t eventgroupId)
{
}

void IpcBinding::unsubscribe(uint16_t eventgroupId)
{
}

void IpcBinding::addSubscriber(uint16_t eventgroupId, Endpoint endpoint)
{
}

void IpcBinding::delSubscriber(uint16_t eventgroupId, Endpoint endpoint)
{
}
			
} // namespace com
} //namespace ara
