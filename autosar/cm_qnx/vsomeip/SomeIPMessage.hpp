/*
 * SomeIPMessage.h
 *
 *  Created on: 2017年12月13日
 *  Author: xuhongchao
 *  Description: provide the packet info of some ip
 */

#ifndef INTERFACE_VSOMEIP_SOMEIPMESSAGE_H_
#define INTERFACE_VSOMEIP_SOMEIPMESSAGE_H_

#include <vector>
#include <string>
#include <vsomeip/primitive_types.hpp>
#include <vsomeip/enumeration_types.hpp>
#include <vsomeip/SomeIPPort.hpp>
namespace vsomeip {
class SomeIPMessage {
public:
	SomeIPMessage(){}
	~SomeIPMessage() {
	}

	/* the following method to set the values of ip message */
	void set_service_id(service_t serviceId);
	void set_method_id(method_t methodId);
	void set_client_id(client_t clientId);
	void set_session_id(session_t sessionId);
	void set_message_type(message_type_e msgType);
	void set_ret_code(return_code_e retCode);
	void set_length(length_t len);
	void set_payload(std::vector<byte_t>& payload);

	void set_sip_port(std::shared_ptr<SomeIPPort> ipPort);

	/* the following method to get the value of some ip */
	service_t get_service_id();
	method_t get_method_id();
	client_t get_client_id();
	session_t get_session_id();
	message_type_e get_message_type();
	return_code_e get_ret_code();
	length_t get_length(length_t len);
	byte_t* get_payload();
	std::shared_ptr<SomeIPPort> get_sip_port();
private:
	service_t service_id;
	method_t method_id;
	client_t client_id;
	session_t session_id;
	message_type_e message_type;
	return_code_e return_code;
	length_t length;
	std::vector<byte_t> payload;
	std::shared_ptr<SomeIPPort> sipPort;
};

} /* namespace vsomeip */

#endif /* INTERFACE_VSOMEIP_SOMEIPMESSAGE_H_ */
