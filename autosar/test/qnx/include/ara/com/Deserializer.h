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
			
			virtual bool deserialize(boolean& value);
			virtual bool deserialize(uint8& value);
			virtual bool deserialize(uint16& value);
			virtual bool deserialize(uint32& value);
			virtual bool deserialize(uint64& value);
			virtual bool deserialize(sint8& value);
			virtual bool deserialize(sint16& value);
			virtual bool deserialize(sint32& value);
			virtual bool deserialize(sint64& value);
			virtual bool deserialize(float32& value);
			virtual bool deserialize(float64& value);
			
		protected:
			std::vector<uint8_t> m_data;
			uint32_t m_size;
			std::vector<uint8_t>::iterator m_pos;
			ByteOrderEnum m_byteOrder;
		};
	} // namespace com
} // namespace ara

#endif // ARA_COM_DESERIALIZER_H_
