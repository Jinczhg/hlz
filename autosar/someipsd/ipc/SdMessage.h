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
				
				std::memcpy(&m_entry, data, sizeof(m_entry));
				offset += sizeof(m_entry);
				
				return true;
			}
			
			int getData(char *data)
			{
				int offset = 0;
				
				std::memcpy(data, &m_entry, sizeof(m_entry));
				offset += sizeof(m_entry);
				
				return offset;
			}
			
			int getClient() const
			{
				return m_client;
			}
			
			const SdEntry* getEntry() const
			{
				return &m_entry;
			}
			
		private:
			int m_client;
			SdEntry m_entry;
		};
	} // namespace com
} // namespace ara

#endif // ARA_COM_SDMESSAGE_H_
