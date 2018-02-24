#ifndef _TUNER_PROXY_API_H_
#define _TUNER_PROXY_API_H_

typedef int (*SwitchStateEventCallback)(std::string);
typedef int (*ChangModeEventCallback)(std::string);
typedef int (*GetFreqEventCallback)(sint32);
typedef int (*GetRtDataEventCallback)(SObtainDataInfo);
typedef int (*ActionStateEventCallback)(SActionState);
typedef int (*PlayListEventCallback)(SPlayList);
typedef int (*FavoritesListEventCallback)(SPlayList);

struct TunerEventCallbackGroup
{
	SwitchStateEventCallback	SwitchStateEvent;
	ChangModeEventCallback		ChangModeEvent;
	GetFreqEventCallback		GetFreqEvent;
	ActionStateEventCallback	ActionStateEvent;
	PlayListEventCallback		PlayListEvent;
	GetRtDataEventCallback		RtDataEvent;
	FavoritesListEventCallback	FavoritesList;
};

class TunerProxyAPI : public com::hinge::proxy::TunerProxy
{
public:
	TunerProxyAPI();
	virtual ~TunerProxyAPI();
	
	int registerEventCallBack(TunerEventCallbackGroup *eventCallbackGroup);
	int switchRadio(std::string cmd);
	int setPlayFrequency(sint32 curFreq);
	int setRemoteOrLocal(sint32 mode);
	int setStereo(sint32 mode);
	int setPTY(uint8 uPtyValue, uint8 uPiValue);
	int setPI(uint8 uPtyValue, uint8 uPiValue);
	int setStep(uint8 uStep);
	int setTA(uint8 uSwitch);
	int jumpFreqPoint(sint32 iIndex);
	int saveFreqPoint(sint32 iIndex);
	int saveFavoriteStation(std::string mode, int iCurStation);
	int clearFavoritesList(std::string mode);
	int seekAction(std::string cmd);
	int scanAction(sint32 mode);
	int updateAction(std::string cmd);
	int setVolume(sint32 iVolume);
	int deleteFavoriteStation(std::string mode, int iCurStation);
	int changeModeForRadio(std::string cmd);
	int getRadioState(sint32 sec, std::string &out);
	int getModeForRadio(sint32 sec, std::string &out);
	int getCurPlayFrequnecy(sint32 sec);
	int getFrequencyList(sint32 sec, std::string mode, std::vector<sint32> &frequencyList, sint32 &iCurrentItem);
	int getFavoritesList(std::string mode);
	int getVersion(sint32 sec, std::string &out);
	int getTunerInfo(sint32 sec, STunerInfo &out);
	int getRadioStatus(sint32 sec, SRadioStatu &out);
	int getRdsInfo(sint32 sec, SRdsInfo &out);
	int getPtyInfo(sint32 sec);
	int getPIInfo(sint32 sec);
	int getPSInfo(sint32 sec);
	int getPsList(sint32 sec, std::vector<std::string> &out);
	int getRdsText(sint32 sec, std::vector<std::string> &out);
	int getSettingInit(sint32 sec, SRadioSettingInit &out);
	int getSettingArea(sint32 sec, Earea &out);
	int getSettingFmBandNum(sint32 sec);
	int getSettingAmBandNum(sint32 sec);
	int getBandFreqPonitNum(sint32 sec);
	int getRdsSwitchStatus(sint32 sec, SRdsSettingInit &out);
	int getSettingVersion(sint32 sec, std::string &out);
	int getHwInfo(sint32 sec, std::string &out);
	int serviceInit(sint32 record);
	void serviceDestroy();
	int setVolumeMute(sint32 mute);
	int setAreaInfo(Earea eArea);
	int setFmNum(sint32 num);
	int setAmNum(sint32 num);
	int setListItemNum(sint32 num);
	int setRDSInfo(SRdsSettingInit stRdsInfo);
};

#endif //_TUNER_PROXY_API_H_
