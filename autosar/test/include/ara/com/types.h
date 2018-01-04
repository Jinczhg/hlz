/*
 * Copyright (c) 2017, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2017-12-14
 * Author: ryan
 */

#ifndef ARA_COM_TYPES_H_
#define ARA_COM_TYPES_H_

#include "Future.h"
#include "ServiceSkeleton.h"
#include "ServiceProxy.h"
#include "PublishEvent.h"
#include "SubscribeEvent.h"
#include "Method.h"
#include "ManagementFactory.h"
#include "DataTypes.h"

namespace ara
{
	namespace com
	{
		using boolean = bool;
		using uint8 = uint8_t;
		using uint16 = uint16_t;
		using uint32 = uint32_t;
		using uint64 = uint64_t;
		using sint8 = int8_t;
		using sint16 = int16_t;
		using sint32 = int32_t;
		using sint64 = int64_t;
		using float32 = float;
		using float64 = double;
	} // namespace com
} // namespace ara

#endif // ARA_COM_TYPES_H_
