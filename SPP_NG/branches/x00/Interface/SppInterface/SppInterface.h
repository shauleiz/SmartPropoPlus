// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the SPPINTERFACE_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// SPPINTERFACE_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef SPPINTERFACE_EXPORTS
#define SPPINTERFACE_API __declspec(dllexport)
#else
#define SPPINTERFACE_API __declspec(dllimport)
#endif

#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>

// Globals
void		WINAPI WinThread(void);

// This class is exported from the SppInterface.dll
class SPPINTERFACE_API CSppInterface {
public:
	CSppInterface(void);
	HRESULT Initialize(HWND hWndParent );

public:
	HWND m_hwnd;
	D2D1_ROUNDED_RECT m_roundedRect;
	class CBaseUnit *bu;

protected:

	// The windows procedure.
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam,LPARAM lParam);

    // Resize the render target.
    void OnResize( UINT width, UINT height);

    // Initialize device-independent resources.
    HRESULT CreateDeviceIndependentResources();

    // Draw content.
    HRESULT OnRender();

    // Initialize device-dependent resources.
    HRESULT CreateDeviceResources();

    // Release device-dependent resource.
    void DiscardDeviceResources();

	// Mouse inside rectangle?
	static bool CSppInterface::inRect(int x, int y, D2D1_RECT_F rect);


protected:

    ID2D1Factory* m_pDirect2dFactory;
	IDWriteFactory *m_pDWriteFactory;
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

    IWICImagingFactory	*m_pWICFactory;
	ID2D1Bitmap			*m_pButtonPauseBitmap;
	ID2D1Bitmap			*m_pButtonPlayBitmap;

	HANDLE m_hWinThread;

	class CSppInterfaceChildWin *m_ChildWin;
	D2D1_RECT_F m_audio_rect;
};

extern SPPINTERFACE_API int nSppInterface;

SPPINTERFACE_API int fnSppInterface(void);
