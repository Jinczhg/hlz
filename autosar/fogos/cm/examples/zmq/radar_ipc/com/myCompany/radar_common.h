#ifndef RADAR_COMMON_H_
#define RADAR_COMMON_H_

#include "ara/com/types.h"

struct RadarObjects
{
	boolean active;
	std::vector<uint8> objects;
};

struct Position
{
	uint32 x;
	uint32 y;
	uint32 z;
};

class RadarSerializer : public ara::com::Serializer
{
public:
	using ara::com::Serializer::serialize;
	
	RadarSerializer()
	: ara::com::Serializer(ara::com::ByteOrderEnum::LittleEndian)
	{
	}
	
	~RadarSerializer(){}
	
	virtual bool serialize(RadarObjects radarObjects)
	{
		serialize(radarObjects.active);
		uint32 size = radarObjects.objects.size();
		serialize(size);
		for (auto v : radarObjects.objects)
		{
			serialize(v);
		}
		
		return true;
	}
	
	virtual bool serialize(Position position)
	{
		serialize(position.x);
		serialize(position.y);
		serialize(position.z);
		
		return true;
	}
};

class RadarDeserializer : public ara::com::Deserializer
{
public:
	using ara::com::Deserializer::deserialize;
	
	RadarDeserializer(const uint8_t *data, uint32_t size)
	: ara::com::Deserializer(ara::com::ByteOrderEnum::LittleEndian, data, size)
	{
	}
	
	~RadarDeserializer(){}
	
	virtual bool deserialize(RadarObjects& radarObjects)
	{
		deserialize(radarObjects.active);
		uint32 size;
		deserialize(size);
		for (uint32 i = 0; i < size; i++)
		{
			uint8 v;
			deserialize(v);
			radarObjects.objects.push_back(v);
		}
		
		return true;
	}
	
	virtual bool deserialize(Position& position)
	{
		deserialize(position.x);
		deserialize(position.y);
		deserialize(position.z);
		
		return true;
	}
};

#endif //RADAR_COMMON_H_
