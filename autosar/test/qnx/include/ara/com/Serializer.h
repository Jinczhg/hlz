/*
 * Copyright (c) 2017, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2017-12-14
 * Author: ryan
 */

#ifndef ARA_COM_SERIALIZER_H_
#define ARA_COM_SERIALIZER_H_

#include "DataTypes.h"

namespace ara
{
	namespace com
	{
		class Serializer
		{
		public:
			Serializer(ByteOrderEnum byteOrder);
			virtual ~Serializer(){}
			
			const uint8_t* getData() const;
			
			uint32_t getSize() const;
			
			bool serialize(boolean value);
			bool serialize(uint8 value);
			bool serialize(uint16 value);
			bool serialize(uint32 value);
			bool serialize(uint64 value);
			bool serialize(sint8 value);
			bool serialize(sint16 value);
			bool serialize(sint32 value);
			bool serialize(sint64 value);
			bool serialize(float32 value);
			bool serialize(float64 value);
			
		protected:
			std::vector<uint8_t> m_data;
			ByteOrderEnum m_byteOrder;
		};
	} // namespace com
} // namespace ara

#endif // ARA_COM_SERIALIZER_H_
