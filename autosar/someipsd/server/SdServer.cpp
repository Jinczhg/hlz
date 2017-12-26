/*
 * Copyright (c) 2017, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2017-12-14
 * Author: ryan
 */
 
#include "SdServer.h"

#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <thread>
#include <iostream>

#include <errno.h>

namespace ara
{
namespace com
{

SdServer::SdServer()
: m_terminated(false)
{
	std::thread t([this](){
		struct sockaddr_un servaddr, cliaddr;
		socklen_t clilen;
		int connsd = 0;
		const char* serverPath = "/tmp/someipsd.server.30490";
	
		this->m_sd = socket(AF_LOCAL, SOCK_STREAM, 0);
	
		unlink(serverPath);
		memset(&servaddr, 0, sizeof(servaddr));
		servaddr.sun_family = AF_LOCAL;
		strcpy(servaddr.sun_path, serverPath);
	
		if (bind(this->m_sd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
		{
			std::cout << "bind error:" << strerror(errno) << std::endl;
		}
		
		listen(this->m_sd, 1024);
	
		while (!this->m_terminated)
		{
			if ((connsd = accept(this->m_sd, (struct sockaddr*)&cliaddr, &clilen)) < 0)
			{
				if (errno == EINTR)
				{
					continue;
				}
				else
				{
					this->m_terminated = true;
					break;
				}
			}
		
			std::shared_ptr<SomeIpSdIpc> ipc(new SomeIpSdIpc(connsd,
			[this](std::shared_ptr<SdMessage> msg){
				this->onMessae(msg);
			},
			[this](int sd){
				std::map<int,std::shared_ptr<SomeIpSdIpc>>::iterator it = this->m_clients.find(sd);
				if (it != this->m_clients.end())
				{
					std::map<uint32_t,std::map<int,std::shared_ptr<SomeIpSdIpc>>>::iterator proxys;
					for (proxys = this->m_proxys.begin(); proxys != this->m_proxys.end(); proxys++)
					{
						std::map<int,std::shared_ptr<SomeIpSdIpc>>::iterator proxy;
						for (proxy = proxys->second.begin(); proxy != proxys->second.end(); proxy++)
						{
							if (proxy->first == sd)
							{
								proxys->second.erase(proxy);
								break;
							}
						}
					}
					
					std::map<uint32_t,std::shared_ptr<SomeIpSdIpc>>::iterator skeleton;
					for (skeleton = this->m_skeletons.begin(); skeleton != this->m_skeletons.end(); skeleton++)
					{
						if (skeleton->second == it->second)
						{
							this->m_skeletons.erase(skeleton);
							break;
						}
					}
					
					this->m_clients.erase(it);
				}
			}));
			
			this->m_clients[connsd] = ipc;
		}
	});
	t.detach();
}

SdServer::~SdServer()
{
}

SdServer* SdServer::get()
{
	static SdServer* s_instance = new SdServer();
	return s_instance;
}

bool SdServer::onMessae(std::shared_ptr<SdMessage> msg)
{
	const SdEntry *entry = msg->getEntry();
	int sd = msg->getClient();
	
	if (entry->type == SdType::SD_FINDSERVICE)
	{
		if (entry->ttl != 0)
		{
			uint32_t key = (entry->serviceId << 16) + entry->instanceId;
			
			std::map<uint32_t,std::map<int,std::shared_ptr<SomeIpSdIpc>>>::iterator proxys = m_proxys.find(key);
			if (proxys != m_proxys.end())
			{
				if (proxys->second.find(msg->getClient()) == proxys->second.end())
				{
					proxys->second[sd] = m_clients[sd];
				}
			}
			else
			{
				m_proxys[key][sd] = m_clients[sd];
			}
		}
		
		//TODO
	}
	else if (entry->type == SdType::SD_OFFERSERVICE)
	{
		if (entry->ttl != 0)
		{
			uint32_t key = (entry->serviceId << 16) + entry->instanceId;
			
			std::map<uint32_t,std::shared_ptr<SomeIpSdIpc>>::iterator skeleton = m_skeletons.find(key);
			if (skeleton == m_skeletons.end())
			{
				m_skeletons[key] = m_clients[sd];
			}
		}
		
		//TODO
	}
	else if (entry->type == SdType::SD_SUBSCRIBEEVENTGROUP)
	{
		//TODO
	}
	else if (entry->type == SdType::SD_SUBSCRIBEEVENTGROUPACK)
	{
		//TODO
	}
	
	return true;
}
				
} // namespace com
} // namespace ara
