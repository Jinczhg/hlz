/*
 *  primitive_types.hpp
 *
 *  Created on: 2017年12月13日
 *  Author: xuhongchao
 *  Description: redefine the basic data type
 */

#ifndef INTERFACE_VSOMEIP_PRIMITIVE_TYPES_HPP_
#define INTERFACE_VSOMEIP_PRIMITIVE_TYPES_HPP_

#include <array>
#include <cstdint>

namespace vsomeip {

typedef uint32_t message_t;
typedef uint16_t service_t;
typedef uint16_t method_t;
typedef uint16_t event_t;

typedef uint16_t instance_t;
typedef uint16_t eventgroup_t;

typedef uint8_t  major_version_t;
typedef uint32_t minor_version_t;

typedef uint32_t ttl_t;

typedef uint32_t request_t;
typedef uint16_t client_t;
typedef uint16_t session_t;

typedef uint32_t length_t;

typedef uint8_t  protocol_version_t;
typedef uint8_t  interface_version_t;

/* the protocol type
 * udp: 0
 * tcp: 1
 * if the value of tcp is 0 ~ 255,and indicate the max number of the tcp connection
 * */
typedef uint8_t protocol_type_t;
typedef uint16_t port_t;

typedef uint32_t socket_fd_t;

typedef uint8_t byte_t;

/* Addresses */
typedef std::array<byte_t, 4> ipv4_address_t;
typedef std::array<byte_t, 16> ipv6_address_t;

} /* namespace vsomeip */

#endif /* INTERFACE_VSOMEIP_PRIMITIVE_TYPES_HPP_ */
