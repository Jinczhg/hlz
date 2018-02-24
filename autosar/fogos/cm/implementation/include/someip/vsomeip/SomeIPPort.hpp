/*
 * someipport.h
 *
 *  Created on: 2017年12月13日
 *  Author: xuhongchao
 *  Description: port info
 */

#ifndef IMPLEMENTATION_SOMEIPPORT_INCLUDE_SOMEIPPORT_H_
#define IMPLEMENTATION_SOMEIPPORT_INCLUDE_SOMEIPPORT_H_
#include <string>
#include <vsomeip/primitive_types.hpp>
namespace vsomeip {

class SomeIPPort {
public:
	SomeIPPort();
	~SomeIPPort() {
	}
	/* the following method to set port value*/
	void set_protocol_type(protocol_type_t tcpFlg);
	void set_src_ip(std::string srcIp);
	void set_dst_ip(std::string dstIp);
	void set_src_port(port_t srcPort);
	void set_dst_port(port_t dstPort);

	/* the following method to get the port value info*/
	protocol_type_t get_protocol_type();
	std::string get_src_ip();
	std::string get_dst_ip();
	port_t get_src_port();
	port_t get_dst_port();

private:
	protocol_type_t tcp_flg;
	std::string src_ip;
	std::string dst_ip;
	port_t src_port;
	port_t dst_port;
};

} /* namespace vsomeip */

#endif /* IMPLEMENTATION_SOMEIPPORT_INCLUDE_SOMEIPPORT_H_ */
