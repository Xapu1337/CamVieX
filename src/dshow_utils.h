/* dshow_utils.h - Enhanced version
 *
 * --------------------------------------------------------------------------
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 * --------------------------------------------------------------------------
 */

#ifndef _DSHOW_UTILS_H_INCLUDE
#define _DSHOW_UTILS_H_INCLUDE

#include <atlbase.h>
#include <dshow.h>
#include <wchar.h>
#include <shlwapi.h>

 // Filter names
#define CAPTURE_FILTER_NAME   L"Capture Filter"
#define AUDIO_FILTER_NAME     L"Audio Filter"
#define SAMPLE_GRABBER_NAME   L"Sample Grabber"
#define RENDERER_FILTER_NAME  L"Renderer"
#define STILL_GRABBER_NAME    L"Still Grabber"
#define STILL_RENDERER_NAME   L"Still Renderer"
#define CUSTOM_FILTER_NAME    L"Custom Filter"

// Function declarations

HRESULT ShowPropertyPages(LPUNKNOWN Object, LPCOLESTR Caption = NULL, HWND hwnd = NULL);

HRESULT GetDeviceMoniker(CLSID Category, int DeviceIndex, IMoniker** ppMoniker);
HRESULT GetDeviceName(CLSID Category, int DeviceIndex, BSTR* pstrName);
HRESULT GetDeviceID(CLSID Category, int DeviceIndex, BSTR* pstrName);

HRESULT FindPinByName(IBaseFilter* pFilter, LPCWSTR sID, LPCWSTR sName, IPin** ppPin);
HRESULT FindPinByCategory(IBaseFilter* pFilter, REFGUID Category, IPin** ppPin);
HRESULT FindPinByDirection(IBaseFilter* pFilter, PIN_DIRECTION eDirection, IPin** ppPin);

HRESULT FindGraphInterface(IGraphBuilder* pGraphBuilder, LPCWSTR pstrFilter, REFIID riid, void** ppif);

HRESULT RegisterFilterGraph(IFilterGraph* pFilterGraph, DWORD* lpdwCookie);
HRESULT UnregisterFilterGraph(DWORD dwCookie);

HRESULT SaveFilterGraph(IFilterGraph* pFilterGraph, BSTR sFilename);
HRESULT LoadFilterGraph(BSTR sFilename, IFilterGraph** ppFilterGraph);

BOOL    GetRGBBitsPerPixel(HDC hdc, PINT pRed, PINT pGreen, PINT pBlue);

HRESULT CreateCompatibleSampleGrabber(IBaseFilter** ppFilter);
HRESULT GetCaptureMediaFormat(IGraphBuilder* pGraph, int index, AM_MEDIA_TYPE** ppmt);
void FreeMediaType(AM_MEDIA_TYPE* pmt);

HRESULT RemoveFiltersFromGraph(IFilterGraph* pFilterGraph);
HRESULT DisconnectFilterGraph(IGraphBuilder* pGraphBuilder);
HRESULT DisconnectPins(IBaseFilter* pFilter);

HRESULT MediaType(LPCWSTR sPath, LPCGUID* ppMediaType, LPCGUID* ppMediaSubType);

// Bitmap format retrieval
static BOOL GetOptimalDIBFormat(HDC hdc, BITMAPINFOHEADER* pbi);
static BYTE CountBits(DWORD dw);

#endif // _DSHOW_UTILS_H_INCLUDE
