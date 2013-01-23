// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the PULSESCOPE_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// PULSESCOPE_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef PULSESCOPE_EXPORTS
#define PULSESCOPE_API __declspec(dllexport)
#else
#define PULSESCOPE_API __declspec(dllimport)
#endif

#define	NDOTS 10000
#define	HI	 0.8f
#define	LO	 0.2f
#define DEFAULT_RATE 192000
#define PULSE_BUF_SIZE DEFAULT_RATE/10
#define PULSE_MAX_SIZE DEFAULT_RATE/10
#define WM_BUFF_READY WM_USER+112

struct PULSE_DATA {
	UINT	nPulses;
	float	*pXBuff;
	float	*pYBuff;
};

// This class is exported from the PulseScope.dll
class PULSESCOPE_API CPulseScope {
public:
	CPulseScope(void);
	HRESULT Initialize(void);
	~CPulseScope();

private:
    // Initialize device-independent resources.
    HRESULT CreateDeviceIndependentResources();

    // Resize the render target.
    void OnResize( UINT width, UINT height);

    // Draw content.
    HRESULT OnRender();

    // Initialize device-dependent resources.
    HRESULT CreateDeviceResources();

    // Release device-dependent resource.
    void DiscardDeviceResources();

	// The windows procedure.
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam,LPARAM lParam);

private:
    HWND m_hwnd;
    ID2D1Factory* m_pDirect2dFactory;
    ID2D1HwndRenderTarget* m_pRenderTarget;
    ID2D1SolidColorBrush* m_pLightSlateGrayBrush;
    ID2D1SolidColorBrush* m_pCornflowerBlueBrush;
    ID2D1SolidColorBrush* m_pDarkViolet;
	ID2D1PathGeometry* m_pWaveGeometry;
    IDWriteTextFormat *m_pTextFormat;
	IDWriteFactory *m_pDWriteFactory;
	HANDLE m_hWinThread;

};

PULSESCOPE_API void Pulse2Scope(int length, bool low, LPVOID Param);
PULSESCOPE_API CPulseScope * InitPulseScope(void);
void		WINAPI WinThread(void);
