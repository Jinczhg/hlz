#ifndef _TUNER_SKELETON_API_H_
#define _TUNER_SKELETON_API_H_

#include "tuner_skeleton.h"

using namespace using namespace com::hinge::skeleton;

class TunerSkeketionAPI : public TunerSkeketion
{
public:
	TunerSkeketionAPI(bool supportMultiThread);
	virtual ~TunerSkeketionAPI();
	
	//event
	int SwitchStateEventSend(char* pState);
	int ChangModeEventSend(char* pMode);
	int GetFreqEventSend(int iFreq);
	int GetRtDataEventSend(SObtainDataInfo* pData);
	int ActionStateEventSend(SActionState* pActionState);
	int PlayListEventSend(SPlayList* pPlayList);
	int FavoritesListSend(SPlayList* pPlayList);
	
	//method
	int switchRadio(char *pCmd);
	int setPlayFrequency(int iCurFreq);
	int setRemoteOrLocal(int iMode);
	int setStereo(int iMode);
	int setPTY(uint8 uPtyValue, uint8 uPiValue);
	int setPI(uint8 uPtyValue, uint8 uPiValue);
	int setStep(uint8 uStep);
	int setTA(uint8 uSwitch);
	int jumpFreqPoint(int iIndex);
	int saveFreqPoint(int iIndex);
	int saveFavoriteStation(char *pMode, int iCurStation);
	int clearFavoritesList(char *pMode);
	int seekAction(char *pCmd);
	int scanAction(int iMode);
	int updateAction(char *pCmd);
	int setVolume(int iVolume);
	int deleteFavoriteStation(char *pMode, int iCurStation);
	int changeModeForRadio(char *pCmd);
	int getRadioState(int iSec, char *pOut);
	int getModeForRadio(int iSec, char *pOut);
	int getCurPlayFrequnecy(int iSec);
	int getFrequencyList(int iSec, char * pMode, int arry[], int iArrySize, int *iCurrentItem);
	int getFavoritesList(char *pMOde);
	int getVersion(int iSec, char *pOut);
	int getTunerInfo(int iSec, STunerInfo* pOut);
	int getRadioStatus(int iSec, SRadioStatu *pOut);
	int getRdsInfo(int iSec, SRdsInfo *pOut);
	int getPtyInfo(int iSec);
	int getPIInfo(int iSec);
	int getPSInfo(int iSec);
	int getPsList(int iSec, char **pOut);
	int getRdsText(int iSec, char **pOut);
	int getSettingInit(int iSec, SRadioSettingInit * pOut);
	int getSettingArea(int iSec, Earea *pOut);
	int getSettingFmBandNum(int iSec);
	int getSettingAmBandNum(int iSec);
	int getBandFreqPonitNum(int iSec);
	int getRdsSwitchStatus(int iSec, SRdsSettingInit *pOut);
	int getSettingVersion(int iSec, char *pOut);
	int getHwInfo(int iSec, char *pOut);
	int serviceInit(int iRecord);
	void serviceDestroy(void);
	int setVolumeMute(int bMute);
	int setAreaInfo(Earea eArea);
	int setFmNum(int iNum);
	int setAmNum(int iNum);
	int setListItemNum(int iNum);
	int setRDSInfo(SRdsSettingInit stRdsInfo);
	
	//method implement
	int switchRadioImp(char *pCmd) = 0;
	int setPlayFrequencyImp(int iCurFreq) = 0;
	int setRemoteOrLocalImp(int iMode) = 0;
	int setStereoImp(int iMode) = 0;
	int setPTYImp(uint8 uPtyValue, uint8 uPiValue) = 0;
	int setPIImp(uint8 uPtyValue, uint8 uPiValue) = 0;
	int setStepImp(uint8 uStep) = 0;
	int setTAImp(uint8 uSwitch) = 0;
	int jumpFreqPointImp(int iIndex) = 0;
	int saveFreqPointImp(int iIndex) = 0;
	int saveFavoriteStationImp(char *pMode, int iCurStation) = 0;
	int clearFavoritesListImp(char *pMode) = 0;
	int seekActionImp(char *pCmd) = 0;
	int scanActionImp(int iMode) = 0;
	int updateActionImp(char *pCmd) = 0;
	int setVolumeImp(int iVolume) = 0;
	int deleteFavoriteStationImp(char *pMode, int iCurStation) = 0;
	int changeModeForRadioImp(char *pCmd) = 0;
	int getRadioStateImp(int iSec, char *pOut) = 0;
	int getModeForRadioImp(int iSec, char *pOut) = 0;
	int getCurPlayFrequnecyImp(int iSec) = 0;
	int getFrequencyListImp(int iSec, char * pMode, int arry[], int iArrySize, int *iCurrentItem) = 0;
	int getFavoritesListImp(char *pMOde) = 0;
	int getVersionImp(int iSec, char *pOut) = 0;
	int getTunerInfoImp(int iSec, STunerInfo* pOut) = 0;
	int getRadioStatusImp(int iSec, SRadioStatu *pOut) = 0;
	int getRdsInfoImp(int iSec, SRdsInfo *pOut) = 0;
	int getPtyInfoImp(int iSec) = 0;
	int getPIInfoImp(int iSec) = 0;
	int getPSInfoImp(int iSec) = 0;
	int getPsListImp(int iSec, char **pOut) = 0;
	int getRdsTextImp(int iSec, char **pOut) = 0;
	int getSettingInitImp(int iSec, SRadioSettingInit * pOut) = 0;
	int getSettingAreaImp(int iSec, Earea *pOut) = 0;
	int getSettingFmBandNumImp(int iSec) = 0;
	int getSettingAmBandNumImp(int iSec) = 0;
	int getBandFreqPonitNumImp(int iSec) = 0;
	int getRdsSwitchStatusImp(int iSec, SRdsSettingInit *pOut) = 0;
	int getSettingVersionImp(int iSec, char *pOut) = 0;
	int getHwInfoImp(int iSec, char *pOut) = 0;
	int serviceInitImp(int iRecord) = 0;
	void serviceDestroyImp(void) = 0;
	int setVolumeMuteImp(int bMute) = 0;
	int setAreaInfoImp(Earea eArea) = 0;
	int setFmNumImp(int iNum) = 0;
	int setAmNumImp(int iNum) = 0;
	int setListItemNumImp(int iNum) = 0;
	int setRDSInfoImp(SRdsSettingInit stRdsInfo) = 0;
};

#endif //_TUNER_SKELETON_API_H_
