#ifndef TUNER_PROXY_H_
#define TUNER_PROXY_H_

#include "tuner_common.h"

namespace com
{
namespace hinge
{
namespace proxy
{
	
namespace events
{
	class SwitchStateEvent : public ara::com::SubscribeEvent
	{
	public:
		using SampleType = std::string;
		
		SwitchStateEvent(ara::com::ServiceProxy* proxy, uint16_t eventId);
		virtual ~SwitchStateEvent(){}
		
		bool Update(ara::com::FilterFunction<SampleType> filter = {});
		
		const ara::com::SampleContainer<ara::com::SamplePtr<const SampleType>>& GetCachedSamples() const;
		 
	private:
		ara::com::SampleContainer<ara::com::SamplePtr<const SampleType>> m_samples;
	};
	
	class ChangModeEvent : public ara::com::SubscribeEvent
	{
	public:
		using SampleType = std::string;
		
		ChangModeEvent(ara::com::ServiceProxy* proxy, uint16_t eventId);
		virtual ~ChangModeEvent(){}
		
		bool Update(ara::com::FilterFunction<SampleType> filter = {});
		
		const ara::com::SampleContainer<ara::com::SamplePtr<const SampleType>>& GetCachedSamples() const;
		 
	private:
		ara::com::SampleContainer<ara::com::SamplePtr<const SampleType>> m_samples;
	};
	
	class GetFreqEvent : public ara::com::SubscribeEvent
	{
	public:
		using SampleType = sint32;
		
		GetFreqEvent(ara::com::ServiceProxy* proxy, uint16_t eventId);
		virtual ~GetFreqEvent(){}
		
		bool Update(ara::com::FilterFunction<SampleType> filter = {});
		
		const ara::com::SampleContainer<ara::com::SamplePtr<const SampleType>>& GetCachedSamples() const;
		 
	private:
		ara::com::SampleContainer<ara::com::SamplePtr<const SampleType>> m_samples;
	};
	
	class GetRtDataEvent : public ara::com::SubscribeEvent
	{
	public:
		using SampleType = SObtainDataInfo;
		
		GetRtDataEvent(ara::com::ServiceProxy* proxy, uint16_t eventId);
		virtual ~GetRtDataEvent(){}
		
		bool Update(ara::com::FilterFunction<SampleType> filter = {});
		
		const ara::com::SampleContainer<ara::com::SamplePtr<const SampleType>>& GetCachedSamples() const;
		 
	private:
		ara::com::SampleContainer<ara::com::SamplePtr<const SampleType>> m_samples;
	};
	
	class ActionStateEvent : public ara::com::SubscribeEvent
	{
	public:
		using SampleType = SActionState;
		
		ActionStateEvent(ara::com::ServiceProxy* proxy, uint16_t eventId);
		virtual ~ActionStateEvent(){}
		
		bool Update(ara::com::FilterFunction<SampleType> filter = {});
		
		const ara::com::SampleContainer<ara::com::SamplePtr<const SampleType>>& GetCachedSamples() const;
		 
	private:
		ara::com::SampleContainer<ara::com::SamplePtr<const SampleType>> m_samples;
	};
	
	class PlayListEvent : public ara::com::SubscribeEvent
	{
	public:
		using SampleType = SPlayList;
		
		PlayListEvent(ara::com::ServiceProxy* proxy, uint16_t eventId);
		virtual ~PlayListEvent(){}
		
		bool Update(ara::com::FilterFunction<SampleType> filter = {});
		
		const ara::com::SampleContainer<ara::com::SamplePtr<const SampleType>>& GetCachedSamples() const;
		 
	private:
		ara::com::SampleContainer<ara::com::SamplePtr<const SampleType>> m_samples;
	};
	
	class FavoritesListEvent : public ara::com::SubscribeEvent
	{
	public:
		using SampleType = SPlayList;
		
		FavoritesListEvent(ara::com::ServiceProxy* proxy, uint16_t eventId);
		virtual ~FavoritesListEvent(){}
		
		bool Update(ara::com::FilterFunction<SampleType> filter = {});
		
		const ara::com::SampleContainer<ara::com::SamplePtr<const SampleType>>& GetCachedSamples() const;
		 
	private:
		ara::com::SampleContainer<ara::com::SamplePtr<const SampleType>> m_samples;
	};
} // namespace events

namespace methods
{
	class switchRadio : public ara::com::Method
	{
	public:
		
		switchRadio(ara::com::ServiceProxy* proxy, uint16_t methodId);
		virtual ~switchRadio(){}
		
		ara::com::Future<void> operator()(const std::string cmd);
	};
	
	class setPlayFrequency : public ara::com::Method
	{
	public:
		
		setPlayFrequency(ara::com::ServiceProxy* proxy, uint16_t methodId);
		virtual ~setPlayFrequency(){}
		
		ara::com::Future<void> operator()(const sint32 curFreq);
	};
	
	class setRemoteOrLocal : public ara::com::Method
	{
	public:
		
		setRemoteOrLocal(ara::com::ServiceProxy* proxy, uint16_t methodId);
		virtual ~setRemoteOrLocal(){}
		
		ara::com::Future<void> operator()(const sint32 mode);
	};
	
	class setStereo : public ara::com::Method
	{
	public:
		
		setStereo(ara::com::ServiceProxy* proxy, uint16_t methodId);
		virtual ~setStereo(){}
		
		ara::com::Future<void> operator()(const sint32 mode);
	};
	
	class setPTY : public ara::com::Method
	{
	public:
		
		setPTY(ara::com::ServiceProxy* proxy, uint16_t methodId);
		virtual ~setPTY(){}
		
		ara::com::Future<void> operator()(const uint8 uPtyValue, const uint8 uPiValue);
	};
	
	class setPI : public ara::com::Method
	{
	public:
		
		setPI(ara::com::ServiceProxy* proxy, uint16_t methodId);
		virtual ~setPI(){}
		
		ara::com::Future<void> operator()(const uint8 uPtyValue, const uint8 uPiValue);
	};
	
	class setStep : public ara::com::Method
	{
	public:
		
		setStep(ara::com::ServiceProxy* proxy, uint16_t methodId);
		virtual ~setStep(){}
		
		ara::com::Future<void> operator()(const uint8 uStep);
	};
	
	class setTA : public ara::com::Method
	{
	public:
		
		setTA(ara::com::ServiceProxy* proxy, uint16_t methodId);
		virtual ~setTA(){}
		
		ara::com::Future<void> operator()(const uint8 uSwitch);
	};
	
	class jumpFreqPoint : public ara::com::Method
	{
	public:
		
		jumpFreqPoint(ara::com::ServiceProxy* proxy, uint16_t methodId);
		virtual ~jumpFreqPoint(){}
		
		ara::com::Future<void> operator()(const sint32 iIndex);
	};
	
	class saveFreqPoint : public ara::com::Method
	{
	public:
		
		saveFreqPoint(ara::com::ServiceProxy* proxy, uint16_t methodId);
		virtual ~saveFreqPoint(){}
		
		ara::com::Future<void> operator()(const sint32 iIndex);
	};
	
	class saveFavoriteStation : public ara::com::Method
	{
	public:
		
		saveFavoriteStation(ara::com::ServiceProxy* proxy, uint16_t methodId);
		virtual ~saveFavoriteStation(){}
		
		ara::com::Future<void> operator()(std::string mode, int iCurStation);
	};
	
	class clearFavoritesList : public ara::com::Method
	{
	public:
		
		clearFavoritesList(ara::com::ServiceProxy* proxy, uint16_t methodId);
		virtual ~clearFavoritesList(){}
		
		ara::com::Future<void> operator()(std::string mode);
	};
	
	class seekAction : public ara::com::Method
	{
	public:
		
		seekAction(ara::com::ServiceProxy* proxy, uint16_t methodId);
		virtual ~seekAction(){}
		
		ara::com::Future<void> operator()(std::string mode);
	};
	
	class scanAction : public ara::com::Method
	{
	public:
		
		scanAction(ara::com::ServiceProxy* proxy, uint16_t methodId);
		virtual ~scanAction(){}
		
		ara::com::Future<void> operator()(sint32 mode);
	};
	
	class updateAction : public ara::com::Method
	{
	public:
		
		updateAction(ara::com::ServiceProxy* proxy, uint16_t methodId);
		virtual ~updateAction(){}
		
		ara::com::Future<void> operator()(std::string mode);
	};
	
	class setVolume : public ara::com::Method
	{
	public:
		
		setVolume(ara::com::ServiceProxy* proxy, uint16_t methodId);
		virtual ~setVolume(){}
		
		ara::com::Future<void> operator()(sint32 iVolume);
	};
	
	class deleteFavoriteStation : public ara::com::Method
	{
	public:
		
		deleteFavoriteStation(ara::com::ServiceProxy* proxy, uint16_t methodId);
		virtual ~deleteFavoriteStation(){}
		
		ara::com::Future<void> operator()(std::string mode, int iCurStation);
	};
	
	class changeModeForRadio : public ara::com::Method
	{
	public:
		
		changeModeForRadio(ara::com::ServiceProxy* proxy, uint16_t methodId);
		virtual ~changeModeForRadio(){}
		
		ara::com::Future<void> operator()(std::string cmd);
	};
	
	class getRadioState : public ara::com::Method
	{
	public:
		struct Output
		{
			std::string state;
		};
		
		getRadioState(ara::com::ServiceProxy* proxy, uint16_t methodId);
		virtual ~getRadioState(){}
		
		ara::com::Future<Output> operator()(sint32 sec);
	};
	
	class getModeForRadio : public ara::com::Method
	{
	public:
		struct Output
		{
			std::string mode;
		};
		
		getModeForRadio(ara::com::ServiceProxy* proxy, uint16_t methodId);
		virtual ~getModeForRadio(){}
		
		ara::com::Future<Output> operator()(sint32 sec);
	};
	
	class getCurPlayFrequnecy : public ara::com::Method
	{
	public:
		
		getCurPlayFrequnecy(ara::com::ServiceProxy* proxy, uint16_t methodId);
		virtual ~getCurPlayFrequnecy(){}
		
		ara::com::Future<void> operator()(sint32 sec);
	};
	
	class getFrequencyList : public ara::com::Method
	{
	public:
		struct Output
		{
			std::vector<sint32> frequencyList;
			sint32 iCurrentItem;
		};
		
		getFrequencyList(ara::com::ServiceProxy* proxy, uint16_t methodId);
		virtual ~getFrequencyList(){}
		
		ara::com::Future<Output> operator()(sint32 sec, std::string mode);
	};
	
	class getFavoritesList : public ara::com::Method
	{
	public:
		
		getFavoritesList(ara::com::ServiceProxy* proxy, uint16_t methodId);
		virtual ~getFavoritesList(){}
		
		ara::com::Future<void> operator()(std::string mode);
	};
	
	class getVersion : public ara::com::Method
	{
	public:
		struct Output
		{
			std::string version;
		};
		
		getVersion(ara::com::ServiceProxy* proxy, uint16_t methodId);
		virtual ~getVersion(){}
		
		ara::com::Future<Output> operator()(sint32 sec);
	};
	
	class getTunerInfo : public ara::com::Method
	{
	public:
		struct Output
		{
			STunerInfo tunerInfo;
		};
		
		getTunerInfo(ara::com::ServiceProxy* proxy, uint16_t methodId);
		virtual ~getTunerInfo(){}
		
		ara::com::Future<Output> operator()(sint32 sec);
	};
	
	class getRadioStatus : public ara::com::Method
	{
	public:
		struct Output
		{
			SRadioStatu radioStatu;
		};
		
		getRadioStatus(ara::com::ServiceProxy* proxy, uint16_t methodId);
		virtual ~getRadioStatus(){}
		
		ara::com::Future<Output> operator()(sint32 sec);
	};
	
	class getRdsInfo : public ara::com::Method
	{
	public:
		struct Output
		{
			SRdsInfo rdsInfo;
		};
		
		getRdsInfo(ara::com::ServiceProxy* proxy, uint16_t methodId);
		virtual ~getRdsInfo(){}
		
		ara::com::Future<Output> operator()(sint32 sec);
	};
} //namespace methods

	class TunerProxy : public ara::com::ServiceProxy
	{
	public:
		explicit TunerProxy(ara::com::ServiceProxy::HandleType handle);
		virtual ~TunerProxy(){}
			
		static ara::com::ServiceHandleContainer<ara::com::ServiceProxy::HandleType> FindService(ara::com::InstanceIdentifier instance);
		static ara::com::FindServiceHandle StartFindService(ara::com::FindServiceHandler<ara::com::ServiceProxy::HandleType> handler, ara::com::InstanceIdentifier instance);
		
		events::SwitchStateEvent SwitchStateEvent;
		methods::Calibrate Calibrate;
		methods::Adjust Adjust;
	};
} // namespace proxy
} // namespace second
} // namespace first

#endif //TUNER_PROXY_H_
