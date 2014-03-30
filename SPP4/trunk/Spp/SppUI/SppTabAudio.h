#pragma once
#include "spptab.h"
class SppTabAudio :
	public SppTab
{
public:
	SppTabAudio(void);
	SppTabAudio(HINSTANCE hInstance, HWND TopDlgWnd);
	virtual ~SppTabAudio(void);

	void DisplayAudioLevels(PVOID Id, UINT Left, UINT Right);
	void AutoParams(WORD ctrl);
	void AudioChannelParams(void);
	void AudioChannelParams(UINT Bitrate, WCHAR Channel);
protected:
public:
	void AudioAutoParams(WORD Mask, WORD Flags);
};

