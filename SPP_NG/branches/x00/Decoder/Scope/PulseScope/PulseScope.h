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
#define	HI	 300.0f
#define	LO	 100.0f
#define DEFAULT_RATE 192000
#define PULSE_BUF_SIZE	DEFAULT_RATE/10
#define PULSE_MAX_SIZE	DEFAULT_RATE/10
#define MID_BUF			DEFAULT_RATE/20


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

	// Get pulse data - Prepare it for scope to render
	void DisplayPulseData(UINT nPulses, float *Length, float *Value);

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

	// Start/Stop measuring distances
	void StopMeasure(int x, int y);
	void Measure(int x, int y);
	void StartMeasure(int x, int y);

	// Display Play/Pause button
	void DisplayPausePlayButton(bool Play,D2D1_RECT_F rect1);


	HRESULT CPulseScope::LoadResourceBitmap(
		ID2D1RenderTarget *pRenderTarget,
		IWICImagingFactory *pIWICFactory,
		PCWSTR resourceName,
		PCWSTR resourceType,
		UINT destinationWidth,
		UINT destinationHeight,
		ID2D1Bitmap **ppBitmap
		);


private:
    HWND m_hwnd;
    ID2D1Factory* m_pDirect2dFactory;
    ID2D1HwndRenderTarget* m_pRenderTarget;

    ID2D1SolidColorBrush* m_pLightSlateGrayBrush;
    ID2D1SolidColorBrush* m_pCornflowerBlueBrush;
    ID2D1SolidColorBrush* m_pDarkViolet;
    ID2D1SolidColorBrush* m_pArrowColor;
    ID2D1SolidColorBrush* m_pButtonColor;
    ID2D1SolidColorBrush* m_pMeasureBrush;

	ID2D1PathGeometry* m_pWaveGeometry;

    IDWriteTextFormat *m_pTextFormat;
    IDWriteTextFormat *m_pBtnTextFormat;
	IDWriteTextFormat *m_pMsrTextFormat;

	IDWriteFactory *m_pDWriteFactory;
	HANDLE m_hWinThread;
	D2D1_POINT_2F *m_points;
	UINT m_npoints;
	float m_offset;
	bool m_isMeasuring;
	bool m_isPlaying;
	D2D1_POINT_2F m_measureStartPoint,  m_measureEndPoint;
	D2D1_RECT_F m_PlayPauseRect;

};

PULSESCOPE_API void Pulse2Scope(int length, bool low, LPVOID Param);
PULSESCOPE_API CPulseScope * InitPulseScope(void);
void		WINAPI WinThread(void);
bool inRect(int x, int y, D2D1_RECT_F rect1);

