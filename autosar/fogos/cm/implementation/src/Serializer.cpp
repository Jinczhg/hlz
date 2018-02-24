/*
 * Copyright (c) 2017, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2017-12-14
 * Author: ryan
 */
 
#include "Serializer.h"
 
namespace ara
{
namespace com
{

Serializer::Serializer(ByteOrderEnum byteOrder)
: m_byteOrder(byteOrder)
{
}

const uint8_t* Serializer::getData() const
{
	return m_data.data();
}

uint32_t Serializer::getSize() const
{
	return static_cast<std::uint32_t>(m_data.size());
}
			
bool Serializer::serialize(boolean value)
{
	uint8_t val = value;
	m_data.push_back(val);
	
	return true;
}

bool Serializer::serialize(uint8 value)
{
	m_data.push_back(value);
			
	return true;
}

bool Serializer::serialize(uint16 value)
{
	uint8_t val = 0;
	
	if (m_byteOrder == ByteOrderEnum::BigEndian)
	{
		val = (value >> 8) & 0xFF;
		m_data.push_back(val);
		
		val = value & 0xFF;
		m_data.push_back(val);
	}
	else
	{
		val = value & 0xFF;
		m_data.push_back(val);
		
		val = (value >> 8) & 0xFF;
		m_data.push_back(val);
	}
	
	return true;
}

bool Serializer::serialize(uint32 value)
{
	uint8_t val = 0;
	
	if (m_byteOrder == ByteOrderEnum::BigEndian)
	{
		val = (value >> 24) & 0xFF;
		m_data.push_back(val);
		
		val = (value >> 16) & 0xFF;
		m_data.push_back(val);
		
		val = (value >> 8) & 0xFF;
		m_data.push_back(val);
		
		val = value & 0xFF;
		m_data.push_back(val);
	}
	else
	{
		val = value & 0xFF;
		m_data.push_back(val);
		
		val = (value >> 8) & 0xFF;
		m_data.push_back(val);
		
		val = (value >> 16) & 0xFF;
		m_data.push_back(val);
		
		val = (value >> 24) & 0xFF;
		m_data.push_back(val);
	}
	
	return true;
}

bool Serializer::serialize(uint64 value)
{
	uint8_t val = 0;
	
	if (m_byteOrder == ByteOrderEnum::BigEndian)
	{
		val = (value >> 56) & 0xFF;
		m_data.push_back(val);
		
		val = (value >> 48) & 0xFF;
		m_data.push_back(val);
		
		val = (value >> 40) & 0xFF;
		m_data.push_back(val);
		
		val = (value >> 32) & 0xFF;
		m_data.push_back(val);
		
		val = (value >> 24) & 0xFF;
		m_data.push_back(val);
		
		val = (value >> 16) & 0xFF;
		m_data.push_back(val);
		
		val = (value >> 8) & 0xFF;
		m_data.push_back(val);
		
		val = value & 0xFF;
		m_data.push_back(val);
	}
	else
	{
		val = value & 0xFF;
		m_data.push_back(val);
		
		val = (value >> 8) & 0xFF;
		m_data.push_back(val);
		
		val = (value >> 16) & 0xFF;
		m_data.push_back(val);
		
		val = (value >> 24) & 0xFF;
		m_data.push_back(val);
		
		val = (value >> 32) & 0xFF;
		m_data.push_back(val);
		
		val = (value >> 40) & 0xFF;
		m_data.push_back(val);
		
		val = (value >> 48) & 0xFF;
		m_data.push_back(val);
		
		val = (value >> 56) & 0xFF;
		m_data.push_back(val);
	}
	
	return true;
}

bool Serializer::serialize(sint8 value)
{
	m_data.push_back(value);
	
	return true;
}

bool Serializer::serialize(sint16 value)
{
	uint8_t val = 0;
	
	if (m_byteOrder == ByteOrderEnum::BigEndian)
	{
		val = (value >> 8) & 0xFF;
		m_data.push_back(val);
		
		val = value & 0xFF;
		m_data.push_back(val);
	}
	else
	{
		val = value & 0xFF;
		m_data.push_back(val);
		
		val = (value >> 8) & 0xFF;
		m_data.push_back(val);
	}
	
	return true;
}

bool Serializer::serialize(sint32 value)
{
	uint8_t val = 0;
	
	if (m_byteOrder == ByteOrderEnum::BigEndian)
	{
		val = (value >> 24) & 0xFF;
		m_data.push_back(val);
		
		val = (value >> 16) & 0xFF;
		m_data.push_back(val);
		
		val = (value >> 8) & 0xFF;
		m_data.push_back(val);
		
		val = value & 0xFF;
		m_data.push_back(val);
	}
	else
	{
		val = value & 0xFF;
		m_data.push_back(val);
		
		val = (value >> 8) & 0xFF;
		m_data.push_back(val);
		
		val = (value >> 16) & 0xFF;
		m_data.push_back(val);
		
		val = (value >> 24) & 0xFF;
		m_data.push_back(val);
	}
	
	return true;
}

bool Serializer::serialize(sint64 value)
{
	uint8_t val = 0;
	
	if (m_byteOrder == ByteOrderEnum::BigEndian)
	{
		val = (value >> 56) & 0xFF;
		m_data.push_back(val);
		
		val = (value >> 48) & 0xFF;
		m_data.push_back(val);
		
		val = (value >> 40) & 0xFF;
		m_data.push_back(val);
		
		val = (value >> 32) & 0xFF;
		m_data.push_back(val);
		
		val = (value >> 24) & 0xFF;
		m_data.push_back(val);
		
		val = (value >> 16) & 0xFF;
		m_data.push_back(val);
		
		val = (value >> 8) & 0xFF;
		m_data.push_back(val);
		
		val = value & 0xFF;
		m_data.push_back(val);
	}
	else
	{
		val = value & 0xFF;
		m_data.push_back(val);
		
		val = (value >> 8) & 0xFF;
		m_data.push_back(val);
		
		val = (value >> 16) & 0xFF;
		m_data.push_back(val);
		
		val = (value >> 24) & 0xFF;
		m_data.push_back(val);
		
		val = (value >> 32) & 0xFF;
		m_data.push_back(val);
		
		val = (value >> 40) & 0xFF;
		m_data.push_back(val);
		
		val = (value >> 48) & 0xFF;
		m_data.push_back(val);
		
		val = (value >> 56) & 0xFF;
		m_data.push_back(val);
	}
	
	return true;
}

bool Serializer::serialize(float32 value)
{
	uint8_t val = 0;
	
	uint32_t tmp = 0;
	
	std::memcpy(&tmp, &value, 4);

	if (m_byteOrder == ByteOrderEnum::BigEndian)
	{
		val = (tmp >> 24) & 0xFF;
		m_data.push_back(val);
		
		val = (tmp >> 16) & 0xFF;
		m_data.push_back(val);
		
		val = (tmp >> 8) & 0xFF;
		m_data.push_back(val);
		
		val = tmp & 0xFF;
		m_data.push_back(val);
	}
	else
	{
		val = tmp & 0xFF;
		m_data.push_back(val);
		
		val = (tmp >> 8) & 0xFF;
		m_data.push_back(val);
		
		val = (tmp >> 16) & 0xFF;
		m_data.push_back(val);
		
		val = (tmp >> 24) & 0xFF;
		m_data.push_back(val);
	}

	return true;
}

bool Serializer::serialize(float64 value)
{
	uint8_t val = 0;
	uint64_t tmp = 0;
	
	std::memcpy(&tmp, &value, 8);

	if (m_byteOrder == ByteOrderEnum::BigEndian)
	{
		val = (tmp >> 56) & 0xFF;
		m_data.push_back(val);
		
		val = (tmp >> 48) & 0xFF;
		m_data.push_back(val);
		
		val = (tmp >> 40) & 0xFF;
		m_data.push_back(val);
		
		val = (tmp >> 32) & 0xFF;
		m_data.push_back(val);
		
		val = (tmp >> 24) & 0xFF;
		m_data.push_back(val);
		
		val = (tmp >> 16) & 0xFF;
		m_data.push_back(val);
		
		val = (tmp >> 8) & 0xFF;
		m_data.push_back(val);
		
		val = tmp & 0xFF;
		m_data.push_back(val);
	}
	else
	{
		val = tmp & 0xFF;
		m_data.push_back(val);
		
		val = (tmp >> 8) & 0xFF;
		m_data.push_back(val);
		
		val = (tmp >> 16) & 0xFF;
		m_data.push_back(val);
		
		val = (tmp >> 24) & 0xFF;
		m_data.push_back(val);
		
		val = (tmp >> 32) & 0xFF;
		m_data.push_back(val);
		
		val = (tmp >> 40) & 0xFF;
		m_data.push_back(val);
		
		val = (tmp >> 48) & 0xFF;
		m_data.push_back(val);
		
		val = (tmp >> 56) & 0xFF;
		m_data.push_back(val);
	}

	return true;
}

bool Serializer::serialize(std::string value, std::string encoding)
{
	uint8_t val = 0;
	uint32_t len = value.length();
	
	if (encoding == "UTF-8")
	{
		if (value.length() > 3 && value[0] == 0xEF && value[1] == 0xBB && value[2] == 0xBF) //with BOM
		{
			serialize(len);
			
			for (int i = 0; i < value.length(); i++)
			{
				val = value[i];
				serialize(val);
			}
		}
		else
		{
			len += 3;
			serialize(len);
			
			// add BOM
			val = 0xEF;
			serialize(val);
			val = 0xBB;
			serialize(val);
			val = 0xBF;
			serialize(val);
			
			for (int i = 0; i < value.length(); i++)
			{
				val = value[i];
				serialize(val);
			}
		}
		
		val = 0x00;
		serialize(val);
	}
	else
	{
		return false;
	}
	
	return true;
}

} // namespace com
} // namespace ara
