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
			Deserializer(ByteOrderEnum byteOrder, uint8_t *data, uint32_t size);
			virtual ~Deserializer(){}
			
			void serialize(boolean& value);
			void serialize(uint8& value);
			void serialize(uint16& value);
			void serialize(uint32& value);
			void serialize(uint64& value);
			void serialize(sint8& value);
			void serialize(sint16& value);
			void serialize(sint32& value);
			void serialize(sint64& value);
			void serialize(float32& value);
			void serialize(float64& value);
			
		protected:
			std::vector<uint8_t> m_data;
			uint32_t m_size;
			ByteOrderEnum m_byteOrder;
		};
	} // namespace com
} // namespace ara

#endif // ARA_COM_DESERIALIZER_H_
