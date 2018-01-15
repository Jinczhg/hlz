/*
 * Copyright (c) 2017, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2017-12-14
 * Author: ryan
 */

#ifndef ARA_COM_DESERIALIZER_H_
#define ARA_COM_DESERIALIZER_H_

#include "DataTypes.h"

namespace ara
{
	namespace com
	{
		class Deserializer
		{
		public:
			Deserializer(ByteOrderEnum byteOrder, const uint8_t *data, uint32_t size);
			virtual ~Deserializer(){}
			
			bool deserialize(boolean& value);
			bool deserialize(uint8& value);
			bool deserialize(uint16& value);
			bool deserialize(uint32& value);
			bool deserialize(uint64& value);
			bool deserialize(sint8& value);
			bool deserialize(sint16& value);
			bool deserialize(sint32& value);
			bool deserialize(sint64& value);
			bool deserialize(float32& value);
			bool deserialize(float64& value);
			
		protected:
			std::vector<uint8_t> m_data;
			uint32_t m_size;
			std::vector<uint8_t>::iterator m_pos;
			ByteOrderEnum m_byteOrder;
		};
	} // namespace com
} // namespace ara

#endif // ARA_COM_DESERIALIZER_H_
