/*
 * Copyright (c) 2017, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2017-12-14
 * Author: ryan
 */
 
#include "Method.h"
#include "ManagementFactory.h"
 
namespace ara
{
namespace com
{

Method::Method(ServiceProxy* proxy, uint16_t methodId)
: m_owner(proxy), m_methodId(methodId)
{
}

Method::~Method()
{
}

uint16_t Method::operator()(std::shared_ptr<Payload> payload, MethodResponseHandler handler)
{
	ServiceRequester *sr = ManagementFactory::get()->getServiceRequester(m_owner->getServiceId(), m_owner->getInstanceId());

	return sr->request(m_methodId, payload, [handler](std::shared_ptr<Message> msg){
		handler(msg->getPayload());
	});
}

void Method::cancel(uint16_t session)
{
	ServiceRequester *sr = ManagementFactory::get()->getServiceRequester(m_owner->getServiceId(), m_owner->getInstanceId());
	sr->cancelRequest(session);
}

} // namespace com
} // namespace ara
