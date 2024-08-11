#ifndef _GRAPH_H_INCLUDE
#define _GRAPH_H_INCLUDE

#define WIN32_LEAN_AND_MEAN
#define OEMRESOURCE 
#include <windows.h>

// The following section allows us to compile using Visual Studio 2008 and
// beyond without requiring the installation of DX9.
#if _MSC_VER < 1500
#include <qedit.h>
#else
#pragma include_alias("dxtrans.h","qedit.h")
#define __IDxtCompositor_INTERFACE_DEFINED__
#define __IDxtAlphaSetter_INTERFACE_DEFINED__
#define __IDxtJpeg_INTERFACE_DEFINED__
#define __IDxtKey_INTERFACE_DEFINED__
#include "qedit.h"
#endif

#ifdef HAVE_WMF_SDK
#include <dshowasf.h>
#endif // HAVE_WMF_SDK

#include "dshow_utils.h"

typedef enum EFilterIndices {
    CaptureFilterIndex,
    AudioFilterIndex,
    SampleGrabberIndex,
    MuxFilterIndex,
    FileSinkIndex,
    RendererFilterIndex,
    StillGrabberIndex,
    StillRendererIndex,
    CustomFilterIndex,
} EFilterIndices;

typedef struct GraphSpecification {
    int nDeviceIndex;
    int nAudioIndex;
    BOOL bAudioRequired;
    IBaseFilter* aFilters[9];
    WCHAR wszSourcePath[MAX_PATH];
    WCHAR wszOutputPath[MAX_PATH];
} GraphSpecification;

// Function declarations
HRESULT ConstructCaptureGraph(GraphSpecification* pSpec, IGraphBuilder** ppGraphBuilder);
HRESULT ConnectFilterGraph(GraphSpecification* pSpec, IGraphBuilder* pGraphBuilder);
HRESULT ReconnectFilterGraph(GraphSpecification* pSpec, IGraphBuilder* pGraphBuilder);
HRESULT RemoveFiltersFromGraph(IFilterGraph* pFilterGraph);
HRESULT DisconnectFilterGraph(IGraphBuilder* pGraphBuilder);
HRESULT DisconnectPins(IBaseFilter* pFilter);
HRESULT GetCaptureMediaFormat(IGraphBuilder* pGraph, int index, AM_MEDIA_TYPE** ppmt);
void FreeMediaType(AM_MEDIA_TYPE* pmt);

// Additional functions that were missing
HRESULT CreateCompatibleSampleGrabber(IBaseFilter** ppFilter);
HRESULT MediaType(LPCWSTR sPath, LPCGUID* ppMediaType, LPCGUID* ppMediaSubType);
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
HRESULT ShowPropertyPages(LPUNKNOWN Object, LPCOLESTR Caption, HWND hwnd);
BOOL GetRGBBitsPerPixel(HDC hdc, PINT pRed, PINT pGreen, PINT pBlue);

#endif /* _GRAPH_H_INCLUDE */
