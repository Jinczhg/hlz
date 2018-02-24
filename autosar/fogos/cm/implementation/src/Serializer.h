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
			
			virtual bool serialize(boolean value);
			virtual bool serialize(uint8 value);
			virtual bool serialize(uint16 value);
			virtual bool serialize(uint32 value);
			virtual bool serialize(uint64 value);
			virtual bool serialize(sint8 value);
			virtual bool serialize(sint16 value);
			virtual bool serialize(sint32 value);
			virtual bool serialize(sint64 value);
			virtual bool serialize(float32 value);
			virtual bool serialize(float64 value);
			
			virtual bool serialize(std::string value, std::string encoding = "UTF-8");
			
		protected:
			std::vector<uint8_t> m_data;
			ByteOrderEnum m_byteOrder;
		};
	} // namespace com
} // namespace ara

#endif // ARA_COM_SERIALIZER_H_
