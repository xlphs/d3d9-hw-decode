// qsv32.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "qsv32.h"

#include "d3dGUID.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HWND videoWindow;																// le D3D video window

// The Direct3D stuff
IDirect3D9 *pD3D = NULL;
IDirect3DDevice9 *d3d9dev = NULL;
UINT d9token; // reset token
IDirect3DDeviceManager9 *d9devmng = NULL;
IDirectXVideoDecoderService *d9videoservice = NULL;
HANDLE dxvadev = NULL;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void AddConsole();

void setup_d3d();
void teardown_d3d();
void ClearD3DBackground();
void CheckSupportedCodecs();

// https://msdn.microsoft.com/en-us/library/windows/desktop/bb970490(v=vs.85).aspx
HRESULT CreateD3DDeviceManager(
	IDirect3DDevice9 *pDevice,
	UINT *pReset,
	IDirect3DDeviceManager9 **ppManager
);


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_QSV32, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_QSV32));

		AddConsole();
		setup_d3d();

		if (dxvadev != NULL) {

			ClearD3DBackground();
			CheckSupportedCodecs();

		}

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }


    return (int) msg.wParam;
}

void AddConsole() {
	AllocConsole();
	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);
}

void ClearD3DBackground() {
	// no sanity checks here, just draw

	HRESULT hr;

	//Colour to clear background
	DWORD colour = 0xFF00BB33;

	//Clear the buffer to our new colour.
	hr = d3d9dev->Clear(0,  //Number of rectangles to clear, we're clearing everything so set it to 0
		NULL, //Pointer to the rectangles to clear, NULL to clear whole display
		D3DCLEAR_TARGET,   //What to clear.  We don't have a Z Buffer or Stencil Buffer
		colour, //Colour to clear to (AARRGGBB)
		1.0f,  //Value to clear ZBuffer to, doesn't matter since we don't have one
		0);   //Stencil clear value, again, we don't have one, this value doesn't matter

	if (FAILED(hr)) {
		printf("Clear failed");
		return;
	}

	hr = d3d9dev->Present(NULL,  //Source rectangle to display, NULL for all of it
		NULL,  //Destination rectangle, NULL to fill whole display
		NULL,  //Target window, if NULL uses device window set in CreateDevice
		NULL);//Unused parameter, set it to NULL

	if (FAILED(hr)) {
		printf("Present failed");
	}

	printf("Cleared background\n");
}


//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_QSV32));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_QSV32);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

	 videoWindow = hWnd;

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
		
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}



void setup_d3d() {

	pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (!pD3D) {
		printf("Direct3DCreate9 failed\n");
		return;
	}

	D3DADAPTER_IDENTIFIER9 d3dai;
	if (pD3D->GetAdapterIdentifier(D3DADAPTER_DEFAULT, 0, &d3dai) != D3D_OK) {
		printf("IDirect3D9_GetAdapterIdentifier failed\n");
	}

	/* Parameters copied from VLC */
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(D3DPRESENT_PARAMETERS));
	d3dpp.Flags = D3DPRESENTFLAG_VIDEO;
	d3dpp.Windowed = TRUE;
	d3dpp.hDeviceWindow = NULL;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
	d3dpp.BackBufferCount = 0;                  /* FIXME what to put here */
	d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;    /* FIXME what to put here */
	d3dpp.BackBufferWidth = 0;
	d3dpp.BackBufferHeight = 0;
	d3dpp.EnableAutoDepthStencil = FALSE;

	DWORD flags = D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED;

	if (pD3D->CreateDevice(D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		videoWindow, // NOTE this window
		flags,
		&d3dpp,
		&d3d9dev) != D3D_OK)
	{
		printf("IDirect3D9_CreateDevice failed\n");
		return;
	}

	printf("Successfully created D3D9 device\n");



	/* Now we setup DXVA2 */

	if (CreateD3DDeviceManager(d3d9dev, &d9token, &d9devmng) != D3D_OK) {
		printf("CreateD3DDeviceManager failed\n");
		return;
	}

	// Get a handle to the Direct3D device.
	if (d9devmng->OpenDeviceHandle(&dxvadev) != S_OK) {
		printf("Failed to get Direct3D device handle\n");
		return;
	}

	if (DXVA2CreateVideoService(d3d9dev, IID_IDirectXVideoDecoderService, (void **)&d9videoservice) != S_OK) {
		printf("DXVA2CreateVideoService failed\n");
		return;
	}

	printf("Successfully setup D3D contexts for decoding acceleration\n");
}

HRESULT CreateD3DDeviceManager(
	IDirect3DDevice9 *pDevice,
	UINT *pReset,
	IDirect3DDeviceManager9 **ppManager
)
{
	UINT resetToken = 0;

	IDirect3DDeviceManager9 *pD3DManager = NULL;

	HRESULT hr = DXVA2CreateDirect3DDeviceManager9(&resetToken, &pD3DManager);

	if (FAILED(hr))
	{
		return hr;
	}

	hr = pD3DManager->ResetDevice(pDevice, resetToken);

	if (FAILED(hr))
	{
		return hr;
	}

	*ppManager = pD3DManager;
	*pReset = resetToken;

	return hr;
}

void teardown_d3d()
{
	if (dxvadev) {
		d9devmng->CloseDeviceHandle(dxvadev);
		dxvadev = NULL;
	}

	if (d9devmng) {
		d9devmng->Release();
		d9devmng = NULL;
	}

	if (d9videoservice) {
		d9videoservice->Release();
		d9videoservice = NULL;
	}

	if (d3d9dev) {
		d3d9dev->Release();
		d3d9dev = NULL;
	}

	if (pD3D) {
		pD3D->Release();
		pD3D = NULL;
	}
}

static const dxva2_mode_t *Dxva2FindMode(const GUID *guid)
{
	for (unsigned i = 0; dxva2_modes[i].name; i++) {
		if (IsEqualGUID(*dxva2_modes[i].guid, *guid))
			return &dxva2_modes[i];
	}
	return NULL;
}

bool isIntelClearVideo(const GUID *guid)
{
	return IsEqualGUID(*guid, DXVA_Intel_H264_NoFGT_ClearVideo);
}

bool isNoEncrypt(const GUID *guid)
{
	return IsEqualGUID(*guid, DXVA_NoEncrypt);
}

bool checkProfile(const dxva2_mode_t *mode, int profile)
{
	if (!mode->profiles || !mode->profiles[0] || profile <= 0)
		return true;
	for (const int *p = &mode->profiles[0]; *p; ++p) {
		if (*p == profile)
			return true;
	}
	return false;
}

static const d3d_format_t *D3dFindFormat(int fourcc)
{
	for (unsigned i = 0; d3d_formats[i].name; i++) {
		if (d3d_formats[i].fourcc == fourcc)
			return &d3d_formats[i];
	}
	return NULL;
}

int getSupportedFourcc(int *formats, UINT nb_formats)
{
	for (const int *f = formats; f < &formats[nb_formats]; ++f) {
		const d3d_format_t *format = D3dFindFormat(*f);
		if (format) {
			printf("%s is supported for output\n", format->name);
		}
		else {
			printf("%d is supported for output (%4.4s)\n", *f, (const char*)f);
		}
	}
	for (const d3d_format_t *format = d3d_formats; format->name; ++format) {
		bool is_supported = false;
		for (unsigned k = 0; !is_supported && k < nb_formats; k++) {
			if (format->fourcc == formats[k])
				return format->fourcc;
		}
	}
	return 0;
}

int fourccFor(const GUID *guid)
{
	UINT output_count = 0;
	D3DFORMAT *output_list = NULL;
	if (FAILED(d9videoservice->GetDecoderRenderTargets(*guid, &output_count, &output_list))) {
		printf("IDirectXVideoDecoderService_GetDecoderRenderTargets failed\n");
		return 0;
	}
	int fmt = getSupportedFourcc((int*)output_list, output_count);
	CoTaskMemFree(output_list);
	return fmt;
}

void CheckSupportedCodecs() {
	if (d9videoservice == NULL) return;

	/* Retreive supported modes from the decoder service */

	unsigned input_count;
	GUID *input_list = NULL;
	DXVA2_VideoDesc videoDesc;
	videoDesc.Format = D3DFMT_UNKNOWN;

	HRESULT rs = d9videoservice->GetDecoderDeviceGuids(&input_count, &input_list);
	switch (rs) {
	case S_OK:
		printf("Got decoder service GUIDs.\n");
		break;
	case D3DERR_INVALIDCALL:
		printf("Error from the Direct3D device.\n");
		break;
	case E_FAIL:
		printf("Device does not support video decoders.\n");
		break;
	default: break;
	}

	for (unsigned c = 0; c < input_count; c++) {
		GUID g = input_list[c];
		const dxva2_mode_t *mode = Dxva2FindMode(&g);
		if (mode) {
			printf("Codec '%s' is supported\n", mode->name);
		}
		else {
			printf("Unknown GUID = %08X-%04x-%04x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x\n",
				(unsigned)g.Data1, g.Data2, g.Data3
				, g.Data4[0], g.Data4[1]
				, g.Data4[2], g.Data4[3], g.Data4[4], g.Data4[5], g.Data4[6], g.Data4[7]);
		}
	}

	if (input_list) CoTaskMemFree(input_list);
}
