/*
 * SomeIPManager.h
 *
 *  Created on: 2017年12月13日
 *  Author: xuhongchao
 *  Description: manage some ip
 */

#ifndef INTERFACE_VSOMEIP_SOMEIPMANAGER_HPP_
#define INTERFACE_VSOMEIP_SOMEIPMANAGER_HPP_

#include <vsomeip/SomeIPMessage.hpp>
#include <vsomeip/SomeIPPort.hpp>
#include <vsomeip/handler.hpp>
namespace vsomeip {

class SomeIPManager {
public:
	virtual ~SomeIPManager() {
	}
	static std::shared_ptr<SomeIPManager> get();

	virtual int createSomeIP(std::shared_ptr<SomeIPPort> ipPort) = 0;
	virtual int closeSomeIP() = 0;
	virtual int sendSomeIP(std::shared_ptr<SomeIPMessage> ipMsg) = 0;

	/* this method return the someip state with runtime*/
	virtual void register_sip_conn_handler(sip_conn_handler_t _handler) = 0;
	virtual void unregister_sip_conn_handler() = 0;

	/* this method will receive the return message*/
	virtual void register_sip_message_handler(sip_message_handler_t handler) = 0;
	virtual void unregister_sip_message_handler() = 0;
};

} /* namespace vsomeip */

#endif /* INTERFACE_VSOMEIP_SOMEIPMANAGER_HPP_ */
