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

class RadarSerializer : public ara::com:Serializer
{
public:
	RadarSerializer()
	: ara::com:Serializer(ara::com::ByteOrderEnum::LittleEndian)
	{
	}
	
	~RadarSerializer(){}
	
	bool serialize(RadarObjects radarObjects)
	{
		serialize(radarObjects.active);
		ara::com::uint32 size = radarObjects.size();
		serialize(size);
		for (auto v : radarObjects.objects)
		{
			serialize(v);
		}
		
		return true;
	}
	
	bool serialize(Position position)
	{
		serialize(position.x);
		serialize(position.y);
		serialize(position.z);
		
		return true;
	}
};

class RadarDeserializer : public ara::com:Deserializer
{
public:
	RadarDeserializer(uint8_t *data, uint32_t size)
	: ara::com:Deserializer(ara::com::ByteOrderEnum::LittleEndian, data, size)
	{
	}
	
	~RadarDeserializer(){}
	
	bool deserialize(RadarObjects& radarObjects)
	{
		deserialize(radarObjects.active);
		ara::com::uint32 size;
		deserialize(size);
		for (ara::com::uint32 i = 0; i < size; i++)
		{
			ara::com::uint8 v;
			deserialize(v);
			radarObjects.objects.push_back(v);
		}
		
		return true;
	}
	
	bool deserialize(Position& position)
	{
		deserialize(position.x);
		deserialize(position.y);
		deserialize(position.z);
		
		return true;
	}
};

#endif //RADAR_COMMON_H_
