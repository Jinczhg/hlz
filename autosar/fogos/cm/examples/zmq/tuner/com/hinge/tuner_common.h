#ifndef RADAR_COMMON_H_
#define RADAR_COMMON_H_

#include "ara/com/types.h"

struct SObtainDataInfo
{
	uint8 szValue[36];
	uint8 uArrayItem;
	uint8 uMsgId;
};

struct STunerInfo
{
	sint8 szInfo[16];
	sint32  iRadioStatus;
	sint32  iInitFinish;
};

struct SPlayList
{
	sint32  iBand;
	sint32  iCurrNum;
	sint32  iArray[18];
};

struct SActionState
{
	sint8 szActionName[64];
	sint8 szState[32];
};

struct SRadioStatu
{
	uint8 flg_seek:1;
	uint8 flg_as:1;     
	uint8 flg_scan:1;
	uint8 flg_stereo:1;
	uint8 flg_lacal:1;
	uint8 rev:3;
};

struct SRdsInfo
{
	uint8 flg_ta:1; 
	uint8 flg_pty:1;
	uint8 flg_af:1;
	uint8 flg_eon:1;
	uint8 flg_reg:1; 
	uint8 flg_tp:1;
	uint8 rev:2;
};

class TunerSerializer : public ara::com::Serializer
{
public:
	using ara::com::Serializer::serialize;
	
	TunerSerializer()
	: ara::com::Serializer(ara::com::ByteOrderEnum::LittleEndian)
	{
	}
	
	~TunerSerializer(){}
	
	virtual bool serialize(SObtainDataInfo sObtainDataInfo)
	{	
		uint32 size = sizeof(sObtainDataInfo.szValue) / sizeof(uint8);
		for (uint32 i = 0; i < size; i++)
		{
			serialize(sObtainDataInfo.szValue[i]);
		}
		serialize(sObtainDataInfo.uArrayItem);
		serialize(sObtainDataInfo.uMsgId);
		
		return true;
	}
	
	virtual bool serialize(STunerInfo sTunerInfo)
	{		
		uint32 size = sizeof(sTunerInfo.szInfo) / sizeof(sint8);
		for (uint32 i = 0; i < size; i++)
		{
			serialize(sTunerInfo.szInfo[i]);
		}
		serialize(sTunerInfo.iRadioStatus);
		serialize(sTunerInfo.iInitFinish);
		
		return true;
	}
	
	virtual bool serialize(SPlayList sPlayList)
	{
		serialize(sPlayList.iBand);
		serialize(sPlayList.iCurrNum);
		uint32 size = sizeof(sPlayList.iArray) / sizeof(sint32);
		for (int i = 0; i < size; i++)
		{
			serialize(sPlayList.iArray[i]);
		}
		
		return true;
	}
	
	virtual bool serialize(SActionState sActionState)
	{	
		uint32 size = sizeof(sActionState.szActionName) / sizeof(sint8);
		for (uint32 i = 0; i < size; i++)
		{
			serialize(sActionState.szActionName[i]);
		}
		size = sizeof(sActionState.szState) / sizeof(sint8);
		for (uint32 i = 0; i < size; i++)
		{
			serialize(sActionState.szState[i]);
		}
		
		return true;
	}
	
	virtual bool serialize(SRadioStatu sRadioStatu)
	{	
		serialize(*(uint8*)&sActionState);
		
		return true;
	}
	
	virtual bool serialize(SRdsInfo sRdsInfo)
	{	
		serialize(*(uint8*)&sRdsInfo);
		
		return true;
	}
};

class TunerDeserializer : public ara::com::Deserializer
{
public:
	using ara::com::Deserializer::deserialize;
	
	TunerDeserializer(const uint8_t *data, uint32_t size)
	: ara::com::Deserializer(ara::com::ByteOrderEnum::LittleEndian, data, size)
	{
	}
	
	~TunerDeserializer(){}
	
	virtual bool deserialize(SObtainDataInfo& sObtainDataInfo)
	{	
		uint32 size;
		deserialize(size);
		for (uint32 i = 0; i < size; i++)
		{
			deserialize(sObtainDataInfo.szValue[i]);
		}
		deserialize(sObtainDataInfo.uArrayItem);
		deserialize(sObtainDataInfo.uMsgId);
		
		return true;
	}
	
	virtual bool deserialize(STunerInfo& sTunerInfo)
	{		
		uint32 size;
		deserialize(size);
		for (uint32 i = 0; i < size; i++)
		{
			deserialize(sTunerInfo.szInfo[i]);
		}
		deserialize(sTunerInfo.iRadioStatus);
		deserialize(sTunerInfo.iInitFinish);
		
		return true;
	}
	
	virtual bool deserialize(SPlayList& sPlayList)
	{
		deserialize(sPlayList.iBand);
		deserialize(sPlayList.iCurrNum);
		uint32 size;
		deserialize(size);
		for (uint32 i = 0; i < size; i++)
		{
			deserialize(sPlayList.iArray[i]);
		}
		
		return true;
	}
	
	virtual bool deserialize(SActionState& sActionState)
	{	
		uint32 size;
		deserialize(size);
		for (uint32 i = 0; i < size; i++)
		{
			deserialize(sActionState.szActionName[i]);
		}
		deserialize(size);
		for (uint32 i = 0; i < size; i++)
		{
			deserialize(sActionState.szState[i]);
		}
		
		return true;
	}
	
	virtual bool deserialize(SRadioStatu& sRadioStatu)
	{	
		deserialize(*(uint8*)&sActionState);
		
		return true;
	}
	
	virtual bool deserialize(SRdsInfo& sRdsInfo)
	{	
		deserialize(*(uint8*)&sRdsInfo);
		
		return true;
	}
};

#endif //RADAR_COMMON_H_
