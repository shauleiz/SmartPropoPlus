// AudioInput.h: interface for the CAudioInput class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AUDIOINPUT_H__9FACD8AD_2945_4282_9FA6_F366CCBA46BD__INCLUDED_)
#define AFX_AUDIOINPUT_H__9FACD8AD_2945_4282_9FA6_F366CCBA46BD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <Mmsystem.h>
/* Global functions */
long GetSelSrcLineID(HMIXER hMixerDevice, unsigned int DstLineID);
MMRESULT GetLineControlList(HMIXEROBJ hMixerDevice, unsigned long LineID, MIXERCONTROL * ControlList, int * cControls=NULL);
MMRESULT GetControlDetailList(HMIXER hmxobj, LPMIXERCONTROLDETAILS_LISTTEXT list, DWORD cMultipleItems, unsigned int dwControlID);
MMRESULT GetControlDetailList(HMIXER hmxobj, LPMIXERCONTROLDETAILS_BOOLEAN list, DWORD cMultipleItems, unsigned int dwControlID);
bool GetVolumeValue(HMIXER hMixerDevice, unsigned int ControlID, long * value);
int SetVolumeValue(HMIXER hMixerDevice, unsigned int ControlID, long  value);
MMRESULT GetLineControlByID(HMIXEROBJ hMixerDevice, unsigned long ControlID, MIXERCONTROL * Control);
bool isSpk(unsigned int type);
bool GetMuteValue(HMIXER hMixerDevice, unsigned int ControlID, bool * value);
MMRESULT Mute(HMIXER hMixerDevice, unsigned int ControlID, bool value=true);
MMRESULT SetControlDetailList(HMIXER hmxobj, LPMIXERCONTROLDETAILS_BOOLEAN list, DWORD cMultipleItems, unsigned int dwControlID);
MMRESULT SetControlDetailList(HMIXER hmxobj, LPMIXERCONTROLDETAILS_UNSIGNED list, DWORD cMultipleItems, unsigned int dwControlID);


class CAudioInput  
{
protected: 
	class CMixerDevice
	{
	protected:
		class CPhysicalDevice
		{
		public: //  CPhysicalDevice
			virtual ~CPhysicalDevice();
			CPhysicalDevice();
			CPhysicalDevice(HMIXER hMixerDevice, unsigned long DstLineID, unsigned long muteCtrlID,bool mute, unsigned long volumeCtrlID, int volume);
			void SetVolume(int volume=50);
			void SetMute(bool mute=true);
			void restore();
		protected: //  CPhysicalDevice
			unsigned long m_DstLineID;
			unsigned long m_muteCtrlID;
			unsigned long m_volumeCtrlID;
			bool m_mute;
			int m_volume;
			HMIXER m_hMixerDevice;
		};//  CPhysicalDevice

		class CInputLine
		{
		struct sMuteLine {bool mute; ULONG LineID; ULONG CtrlID;};
		public: // CInputLine
			int StoreDefaultVolume();
			bool isInitSelected(void);
			void RestoreMute(void);
			bool SetMute(bool mute=true, bool temporary = false);
			int RestoreVolume(void);
			void SetDefaultVolume(void);
			unsigned long GetSrcID(void);
			const char * GetName(void);
			void SetSrcType(unsigned long type);
			void SetName(const char * name);
			CInputLine();
			CInputLine(HMIXER hMixerDevice, unsigned long id, const char * Name, unsigned long Type, long Selected);
			virtual ~CInputLine();

		protected: // CInputLine
			HMIXER m_hMixerDevice;
			unsigned long m_ID;
			const char * m_Name;
			unsigned long m_SrcType;
			int	m_InitVolume;
			bool m_InitSelected;
			CArray< sMuteLine ,sMuteLine> * m_ArrayMuteControl;
			CArray< sMuteLine ,sMuteLine>  * CreateArrayMuteCtrl(ULONG m_SrcType, const char * m_Name);
		}; // CInputLine


		public: //CMixerDevice
			int SetSpeakers(bool restore=false, bool mute=true);
			bool MuteSelectedInputLine(unsigned int line, bool mute=true, bool temporary=false);
			void RestoreSelectedInputLine(unsigned int line);
			bool GetInputLineSrcID(unsigned int * SrcID, unsigned int Index);
			bool GetInputLineIndex(unsigned int SrcID, unsigned int * Index);
			bool GetSelectedInputLine(unsigned int * iLine);
			int Restore(void);
			void SelectInputLine(unsigned int SrcID);
			void AdjustInputVolume(unsigned long SrcID);
			bool MuteOutputLine(unsigned long SrcID, bool mute=true, bool temporary=false);
			bool SetSelectedInputLine(unsigned int line);
			const char * GetInputLineName(int i);
			LPCWSTR GetName();
			virtual ~CMixerDevice();
			CMixerDevice(int Index);

		protected: //CMixerDevice
			CMixerDevice();
			int CreatePhysicalDevArray(void);
			int CreateInputLineArray(void);

		protected: //CMixerDevice
			HMIXER m_hMixerDevice;
			int m_ID;
			LPWSTR m_name;
			long m_CurrentSrcID;
			CArray< CInputLine *,CInputLine *> m_ArrayInputLine;
			CArray< CPhysicalDevice *,CPhysicalDevice *> m_ArrayPhysicalDev;
	}; // CMixerDevice
	

	public: // CAudioInput
		virtual int GetCurrentMixerDevice(void);
		virtual bool SetCurrentMixerDevice(int i);
		virtual int GetMixerDeviceIndex(LPCWSTR mixer);
		virtual void Restore(void);
		virtual LPCWSTR GetMixerDeviceName(int index);
		virtual LPCWSTR GetMixerDeviceUniqueName(int index);
		virtual const char * GetMixerDeviceInputLineName(int Mixer, int Line);
		virtual bool GetMixerDeviceInputLineSrcID(int Mixer, unsigned int * SrcID, unsigned int Index);
		virtual bool GetMixerDeviceInputLineIndex(int Mixer, unsigned int SrcID, unsigned int * Index);
		virtual bool SetMixerDeviceSelectInputLine(int Mixer, int Line);
		virtual bool GetMixerDeviceSelectInputLine(int Mixer, unsigned int * iLine);
		virtual bool MuteSelectedInputLine(int Mixer, unsigned int line, bool mute=true, bool temporary=false);
		virtual int SetSpeakers(int Mixer, bool restore=false, bool mute=true);
		virtual int GetCountMixerDevice(void);
		CAudioInput();
		CAudioInput(bool dummy);
		virtual ~CAudioInput();
		
	protected: // CAudioInput
		CMixerDevice * GetMixerDevice(int Mixer);

	protected: // CAudioInput
		CArray< CMixerDevice *,CMixerDevice *> m_ArrayMixerDevice;
		int m_CurrentMixerDevice;
}; // CAudioInput


#endif // !defined(AFX_AUDIOINPUT_H__9FACD8AD_2945_4282_9FA6_F366CCBA46BD__INCLUDED_)
