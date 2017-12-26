/*
 * Copyright (c) 2017, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2017-12-14
 * Author: ryan
 */
 
#include "SdClient.h"

#include <sys/socket.h>
#include <sys/un.h>
#include <sstream>
#include <iostream>

#include <errno.h>
#include <unistd.h>

namespace ara
{
namespace com
{

SdClient::SdClient()
{
	struct sockaddr_un servaddr, cliaddr;
	std::stringstream ss;
	ss << "/tmp/someipsd.client." << getpid();
	std::string path = ss.str();
	const char* clientPath = path.c_str();
	const char* serverPath = "/tmp/someipsd.server.30490";

	m_sd = socket(AF_LOCAL, SOCK_STREAM, 0);

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sun_family = AF_LOCAL;
	strcpy(servaddr.sun_path, serverPath);
	
	memset(&cliaddr, 0, sizeof(cliaddr));
	cliaddr.sun_family = AF_LOCAL;
	strcpy(cliaddr.sun_path, clientPath);

	if (bind(m_sd, (struct sockaddr*)&cliaddr, sizeof(cliaddr)) < 0)
	{
		std::cout << "bind error:" << strerror(errno) << std::endl;
	}
	
	if (connect(m_sd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
	{
		std::cout << "connect error:" << strerror(errno) << std::endl;
	}
	
	std::shared_ptr<SomeIpSdIpc> ipc(new SomeIpSdIpc(m_sd,
	[this](std::shared_ptr<SdMessage> msg){
		this->onMessae(msg);
	},
	[this](int sd){
		
	}));
	
	m_server = ipc;
}

SdClient::~SdClient()
{
}

SdClient* SdClient::get()
{
	static SdClient* s_instance = new SdClient();
	return s_instance;
}

bool SdClient::onMessae(std::shared_ptr<SdMessage> msg)
{
	return true;
}

} // namespace com
} // namespace ara
