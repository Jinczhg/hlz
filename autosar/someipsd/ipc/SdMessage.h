/*
 * Copyright (c) 2017, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2017-12-14
 * Author: ryan
 */
 
#ifndef ARA_COM_SDMESSAGE_H_
#define ARA_COM_SDMESSAGE_H_

#include <cstdint>
#include <cstring>
#include <memory>
#include <vector>

namespace ara
{
	namespace com
	{
		enum class SdType : uint8_t {
			SD_FINDSERVICE = 0x00,
			SD_OFFERSERVICE = 0x01,
			SD_SUBSCRIBEEVENTGROUP = 0x06,
			SD_SUBSCRIBEEVENTGROUPACK = 0x07,
			SD_UNKNOWN = 0xFF
		};
		
		struct SdEntry
		{
			SdType type;
			uint16_t serviceId;
			uint16_t instanceId;
			uint8_t majorVersion;
			uint32_t ttl;
			union {
				struct {
					uint32_t minorVersion;
				} service;
				struct {
					uint8_t initial;
					uint8_t counter;
					uint16_t eventgroupId;
				} eventgroup;
			} entry;
		};
		
		struct SdOption
		{
			uint16_t length;
			uint8_t type;
			uint8_t reserved;
		};
		
		struct ConfigurationOption : public SdOption
		{
			char configuration[1024];
		};
		
		struct LoadBalancingOption : public SdOption
		{
			uint16_t priority;
			uint16_t weight;
		};
		
		struct IPv4EndpointOption : public SdOption
		{
			uint32_t ip;
			uint8_t reserved1;
			uint8_t protocol;
			uint16_t port;
		};
		
		struct IPv4MulticastOption : public SdOption
		{
			uint32_t ip;
			uint8_t reserved1;
			uint8_t protocol;
			uint16_t port;
		};
		
		struct IPv4SdEndpointOption : public SdOption
		{
			uint32_t ip;
			uint8_t reserved1;
			uint8_t protocol;
			uint16_t port;
		};
		
		class SdMessage
		{
		public:
			SdMessage(int client = 0)
			: m_client(client)
			{
			}
			
			virtual ~SdMessage(){}
			
			bool parseData(char *data, int len)
			{
				int offset = 0;
				
				memcpy(&m_session, data + offset, sizeof(m_session));
				offset += sizeof(m_session);
				
				memcpy(&m_client, data + offset, sizeof(m_client));
				offset += sizeof(m_client);
				
				std::memcpy(&m_entry, data + offset, sizeof(m_entry));
				offset += sizeof(m_entry);
				
				while (offset < len)
				{
					SdOption *op = reinterpret_cast<SdOption*>(data + offset);
					switch (op->type)
					{
					case 0x01:
					{
						std::shared_ptr<SdOption> option(new ConfigurationOption);
						std::memcpy(option.get(), data + offset, op->length + 3);
						m_options.push_back(option);
						offset += op->length + 3;
						break;
					}
					
					case 0x02:
					{
						std::shared_ptr<SdOption> option(new LoadBalancingOption);
						std::memcpy(option.get(), data + offset, op->length + 3);
						m_options.push_back(option);
						offset += op->length + 3;
						break;
					}
					
					case 0x04:
					{
						std::shared_ptr<SdOption> option(new IPv4EndpointOption);
						std::memcpy(option.get(), data + offset, op->length + 3);
						m_options.push_back(option);
						offset += op->length + 3;
						break;
					}
					
					case 0x14:
					{
						std::shared_ptr<SdOption> option(new IPv4MulticastOption);
						std::memcpy(option.get(), data + offset, op->length + 3);
						m_options.push_back(option);
						offset += op->length + 3;
						break;
					}
					
					case 0x24:
					{
						std::shared_ptr<SdOption> option(new IPv4SdEndpointOption);
						std::memcpy(option.get(), data + offset, op->length + 3);
						m_options.push_back(option);
						offset += op->length + 3;
						break;
					}
					
					default:
						break;
					}
				}
				
				return true;
			}
			
			int getData(char *data)
			{
				int offset = 0;
				
				std::memcpy(data + offset, &m_session, sizeof(m_session));
				offset += sizeof(m_session);
				
				std::memcpy(data + offset, &m_client, sizeof(m_client));
				offset += sizeof(m_client);
				
				std::memcpy(data + offset, &m_entry, sizeof(m_entry));
				offset += sizeof(m_entry);
				
				for (auto option : m_options)
				{
					std::memcpy(data + offset, option.get(), option->length + 3);
					offset += option->length + 3;
				}
				
				return offset;
			}
			
			void setSession(uint32_t session)
			{
				m_session = session;
			}
			
			int getClient() const
			{
				return m_client;
			}
			
			uint32_t getSession()
			{
				return m_session;
			}
			
			const SdEntry* getEntry() const
			{
				return &m_entry;
			}
			
		private:
			uint32_t m_session;
			int m_client;
			SdEntry m_entry;
			std::vector<std::shared_ptr<SdOption>> m_options;
		};
	} // namespace com
} // namespace ara

#endif // ARA_COM_SDMESSAGE_H_
