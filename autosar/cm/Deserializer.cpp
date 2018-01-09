/*
 * Copyright (c) 2017, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2017-12-14
 * Author: ryan
 */
 
#include "Deserializer.h"
 
namespace ara
{
namespace com
{

Deserializer::Deserializer(ByteOrderEnum byteOrder, const uint8_t *data, uint32_t size)
: m_data(data, data + size), m_size(size), m_pos(m_data.begin()), m_byteOrder(byteOrder) 
{
}
			
bool Deserializer::deserialize(boolean& value)
{
	if (m_size == 0)
	{
		return false;
    }
    
	value = (boolean)*m_pos++;
	m_size--;
	
	return true;
}

bool Deserializer::deserialize(uint8& value)
{
	if (m_size == 0)
	{
		return false;
    }
    
	value = *m_pos++;
	m_size--;
	
	return true;
}

bool Deserializer::deserialize(uint16& value)
{
	if (m_size < 2)
	{
		return false;
    }
    
    if (m_byteOrder == ByteOrderEnum::BigEndian)
	{
		value = *m_pos++ << 8;
		value += *m_pos++;
	}
	else
	{
		value = *m_pos++;
		value += *m_pos++ << 8;
	}
	
	m_size -= 2;
	
	return true;
}

bool Deserializer::deserialize(uint32& value)
{
	if (m_size < 4)
	{
		return false;
    }
    
    if (m_byteOrder == ByteOrderEnum::BigEndian)
	{
		value = *m_pos++ << 24;
		value += *m_pos++ << 16;
		value += *m_pos++ << 8;
		value += *m_pos++;
	}
	else
	{
		value = *m_pos++;
		value += *m_pos++ << 8;
		value += *m_pos++ << 16;
		value += *m_pos++ << 24;
	}
	
	m_size -= 4;
	
	return true;
}

bool Deserializer::deserialize(uint64& value)
{
	if (m_size < 8)
	{
		return false;
    }
    
    if (m_byteOrder == ByteOrderEnum::BigEndian)
	{
		value = *m_pos++;
		value = value << 8;
		value += *m_pos++;
		value = value << 8;
		value += *m_pos++;
		value = value << 8;
		value += *m_pos++;
		value = value << 8;
		value += *m_pos++;
		value = value << 8;
		value += *m_pos++;
		value = value << 8;
		value += *m_pos++;
		value = value << 8;
		value += *m_pos++;
	}
	else
	{
		uint64 tmp = 0;
		value = *m_pos++;
		value += *m_pos++ << 8;
		value += *m_pos++ << 16;
		value += *m_pos++ << 24;
		
		tmp += *m_pos++;
		tmp += *m_pos++ << 8;
		tmp += *m_pos++ << 16;
		tmp += *m_pos++ << 24;
		
		value += tmp << 32;
	}
	
	m_size -= 8;
	
	return true;
}

bool Deserializer::deserialize(sint8& value)
{
	if (m_size == 0)
	{
		return false;
    }
    
	value = *m_pos++;
	m_size--;
	
	return true;
}

bool Deserializer::deserialize(sint16& value)
{
	if (m_size < 2)
	{
		return false;
    }
    
    if (m_byteOrder == ByteOrderEnum::BigEndian)
	{
		value = *m_pos++ << 8;
		value += *m_pos++;
	}
	else
	{
		value = *m_pos++;
		value += *m_pos++ << 8;
	}
	
	m_size -= 2;
	
	return true;
}

bool Deserializer::deserialize(sint32& value)
{
	if (m_size < 4)
	{
		return false;
    }
    
    if (m_byteOrder == ByteOrderEnum::BigEndian)
	{
		value = *m_pos++ << 24;
		value += *m_pos++ << 16;
		value += *m_pos++ << 8;
		value += *m_pos++;
	}
	else
	{
		value = *m_pos++;
		value += *m_pos++ << 8;
		value += *m_pos++ << 16;
		value += *m_pos++ << 24;
	}
	
	m_size -= 4;
	
	return true;
}

bool Deserializer::deserialize(sint64& value)
{
	if (m_size < 8)
	{
		return false;
    }
    
    if (m_byteOrder == ByteOrderEnum::BigEndian)
	{
		value = *m_pos++;
		value = value << 8;
		value += *m_pos++;
		value = value << 8;
		value += *m_pos++;
		value = value << 8;
		value += *m_pos++;
		value = value << 8;
		value += *m_pos++;
		value = value << 8;
		value += *m_pos++;
		value = value << 8;
		value += *m_pos++;
		value = value << 8;
		value += *m_pos++;
	}
	else
	{
		uint64 tmp = 0;
		
		tmp = *m_pos++;
		value = tmp;
		tmp = *m_pos++;
		value += tmp << 8;
		tmp = *m_pos++;
		value += tmp << 16;
		tmp = *m_pos++;
		value += tmp << 24;
		tmp = *m_pos++;
		value += tmp << 32;
		tmp = *m_pos++;
		value += tmp << 40;
		tmp = *m_pos++;
		value += tmp << 48;
		tmp = *m_pos++;
		value += tmp << 56;
	}
	
	m_size -= 8;
	
	return true;
}

bool Deserializer::deserialize(float32& value)
{
	if (m_size < 4)
	{
		return false;
    }
    
    uint32_t tmp = 0;
    
    if (m_byteOrder == ByteOrderEnum::BigEndian)
	{
		tmp = *m_pos++ << 24;
		tmp += *m_pos++ << 16;
		tmp += *m_pos++ << 8;
		tmp += *m_pos++;
	}
	else
	{
		tmp = *m_pos++;
		tmp += *m_pos++ << 8;
		tmp += *m_pos++ << 16;
		tmp += *m_pos++ << 24;
	}
	
	std::memcpy(&value, &tmp, 4);
	
	m_size -= 4;
	
	return true;
}

bool Deserializer::deserialize(float64& value)
{
	if (m_size < 8)
	{
		return false;
    }
    
    uint64_t tmp = 0;
    
    if (m_byteOrder == ByteOrderEnum::BigEndian)
	{
		tmp = *m_pos++;
		tmp = tmp << 8;
		tmp += *m_pos++;
		tmp = tmp << 8;
		tmp += *m_pos++;
		tmp = tmp << 8;
		tmp += *m_pos++;
		tmp = tmp << 8;
		tmp += *m_pos++;
		tmp = tmp << 8;
		tmp += *m_pos++;
		tmp = tmp << 8;
		tmp += *m_pos++;
		tmp = tmp << 8;
		tmp += *m_pos++;
	}
	else
	{
		uint64 tmp1 = 0;
		tmp = *m_pos++;
		tmp += *m_pos++ << 8;
		tmp += *m_pos++ << 16;
		tmp += *m_pos++ << 24;
		
		tmp1 += *m_pos++;
		tmp1 += *m_pos++ << 8;
		tmp1 += *m_pos++ << 16;
		tmp1 += *m_pos++ << 24;
		
		tmp += tmp1 << 32;
	}
	
	std::memcpy(&value, &tmp, 8);
	
	m_size -= 8;
	
	return true;
}

} // namespace com
} // namespace ara
