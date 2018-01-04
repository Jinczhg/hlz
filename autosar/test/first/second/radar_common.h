#ifndef RADAR_COMMON_H_
#define RADAR_COMMON_H_

#include "ara/com/types.h"

struct RadarObjects
{
	ara::com::boolean active;
	std::vector<ara::com::uint8> objects;
};

struct Position
{
	ara::com::uint32 x;
	ara::com::uint32 y;
	ara::com::uint32 z;
};

#endif //RADAR_COMMON_H_
