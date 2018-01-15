/*
 * Copyright (c) 2017, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2017-12-14
 * Author: ryan
 */
 
#ifndef ARA_COM_DATATYPES_H_
#define ARA_COM_DATATYPES_H_

#include <cstdint>
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <cstring>
#include <cstdlib>
#include <array>

#include "BaseTypes.h"

namespace ara
{
	namespace com
	{	
		enum class ByteOrderEnum : uint8_t {
            BigEndian = 0,
            LittleEndian = 1
        };
		
		//Service Identifier Data Types
        class InstanceIdentifier {
        public:
            static const InstanceIdentifier Any;
            explicit InstanceIdentifier(std::string value);
            ~InstanceIdentifier(){}

            InstanceIdentifier& operator=(const InstanceIdentifier& other);
            
            bool operator==(const InstanceIdentifier& other) const;
            
            bool operator<(const InstanceIdentifier& other) const;
            
            std::string toString() const;
            
            uint16_t getId() const;
            
        private:
        	uint16_t m_id;
        	std::string m_value;
        };
        
        class FindServiceHandle
        {
        public:
        	FindServiceHandle(uint16_t serviceId, uint16_t instanceId);
        	~FindServiceHandle(){}
        	
        	FindServiceHandle& operator=(FindServiceHandle& other);
        	bool operator==(FindServiceHandle& other) const;
        	bool operator<(FindServiceHandle& other) const;
        	
        	uint16_t getServiceId() const;
        	uint16_t getInstanceId() const;
        	uint32_t getId() const;
        	
        private:
        	uint16_t m_serviceId;
        	uint16_t m_instanceId;
        	uint32_t m_id;
        };
        
        template <typename T>
        using ServiceHandleContainer = std::vector<T>;
        
        template <typename T>
        using FindServiceHandler = std::function<void(ServiceHandleContainer<T>)>;
        
        // Event Related Data Types
        enum class EventCacheUpdatePolicy : uint8_t {
            kLastN,
            kNewestN
        };
        
        template <typename T>
        using SamplePtr = std::shared_ptr<T>;
        
        template <typename T>
        using SampleContainer = std::vector<T>;
        
        template <typename T>
        using SampleAllocateePtr = std::unique_ptr<T>;
        
        using EventReceiveHandler = std::function<void()>;
        
        enum class SubscriptionState : uint8_t {
            kSubscribed,
            kNotSubscribed
        };
        
        using SubscriptionStateChangeHandler = std::function<void(SubscriptionState)>;
        
        template <typename T>
        using FilterFunction = std::function<bool(const T&)>;
        
        // Method Related Data Types
        enum class MethodCallProcessingMode : uint8_t {
            kPoll,
            kEvent,
            kEventSingleThread
        };
        
        class Payload
        {
        	uint32_t m_size;
        	uint8_t* m_data;
        	
        public:
        	Payload(uint32_t size, const uint8_t *data);
        	~Payload();
        	
        	uint32_t getSize() const;
        	uint8_t* getData() const;
        };
        
        enum class NetWorkBindingType
        {
        	SOMEIP,
        	IPC
        };

		enum class MessageType : uint8_t {
			MT_REQUEST = 0x00,
			MT_REQUEST_NO_RETURN = 0x01,
			MT_NOTIFICATION = 0x02,
			MT_REQUEST_ACK = 0x40,
			MT_REQUEST_NO_RETURN_ACK = 0x41,
			MT_NOTIFICATION_ACK = 0x42,
			MT_RESPONSE = 0x80,
 			MT_ERROR = 0x81,
			MT_RESPONSE_ACK = 0xC0,
			MT_ERROR_ACK = 0xC1,
			MT_UNKNOWN = 0xFF
		};


		enum class ReturnCode : uint8_t {
			E_OK = 0x00,
			E_NOT_OK = 0x01,
			E_UNKNOWN_SERVICE = 0x02,
			E_UNKNOWN_METHOD = 0x03,
			E_NOT_READY = 0x04,
 			E_NOT_REACHABLE = 0x05,
 			E_TIMEOUT = 0x06,
			E_WRONG_PROTOCOL_VERSION = 0x07,
			E_WRONG_INTERFACE_VERSION = 0x08,
			E_MALFORMED_MESSAGE = 0x09,
			E_WRONG_MESSAGE_TYPE = 0xA,
			E_UNKNOWN = 0xFF
		};
        
        class Message
        {
        public:
        	Message();
        	~Message(){}
        	
        	void setServiceId(uint16_t serviceId);
        	void setInstanceId(uint16_t instanceId);
        	void setClientId(uint16_t clientId);
        	void setMethodId(uint16_t methodId);
        	void setSession(uint16_t session);
        	void setId(uint16_t id);
        	void setType(MessageType type);
        	void setCode(ReturnCode code);
        	void setPayload(std::shared_ptr<Payload> payload);
        	
        	uint16_t getServiceId();
        	uint16_t getInstanceId();
        	uint16_t getClientId();
        	uint16_t getMethodId();
        	uint16_t getSession();
        	uint16_t getId();
        	MessageType getType();
        	ReturnCode getCode();
        	std::shared_ptr<Payload> getPayload();
        	
        private:
			uint16_t m_serviceId;
        	uint16_t m_methodId;
        	uint16_t m_clientId;
        	uint16_t m_instanceId;
        	uint16_t m_session;
        	uint16_t m_id;
        	MessageType m_type;
        	ReturnCode m_code;
        	std::shared_ptr<Payload> m_payload;
        };
        
        enum class TransportProtocol : uint8_t
        {
        	udp,
        	tcp
        };
        
        using ipv4_address_t = std::array<uint8_t, 4>;
        
        struct Endpoint
        {
        public:
        	Endpoint(ipv4_address_t ip, uint16_t port, TransportProtocol protocol);
        	Endpoint(Endpoint& e);
        	~Endpoint(){}
        	
        	Endpoint& operator=(Endpoint& e);
        	
        	ipv4_address_t getIp() const;
        	uint16_t getPort() const;
        	TransportProtocol getProtocol() const;
        	
        private:
        	ipv4_address_t m_ip;
        	uint16_t m_port;
        	TransportProtocol m_protocol;
        };
        
        
        using MessageReceiveHandler = std::function<void (std::shared_ptr<Message>)>;
        
	} // namespace com
} // namespace ara

#endif // ARA_COM_DATATYPES_H_
