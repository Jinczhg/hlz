/*
 * handler.hpp
 *
 *  Created on: 2017年12月13日
 *      Author: xuhongchao
 */

#ifndef INTERFACE_VSOMEIP_HANDLER_HPP_
#define INTERFACE_VSOMEIP_HANDLER_HPP_
#include <vsomeip/enumeration_types.hpp>
#include <functional>
namespace vsomeip{

typedef std::function< void (conn_type_e) > sip_conn_handler_t;
typedef std::function< void (const std::shared_ptr< SomeIPMessage > &) > sip_message_handler_t;

}
#endif /* INTERFACE_VSOMEIP_HANDLER_HPP_ */
