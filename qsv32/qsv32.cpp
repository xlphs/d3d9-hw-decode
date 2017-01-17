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

void Decode();

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
	if (!InitInstance(hInstance, nCmdShow))
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


	return (int)msg.wParam;
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

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_QSV32));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_QSV32);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

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

			// DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);

			// Instead of showing about window, decode
			Decode();

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

	// Ref https://msdn.microsoft.com/en-us/library/windows/desktop/cc307964(v=vs.85).aspx
	// Here we create the video service so FFmpeg can use IDirect3DDeviceManager9::GetVideoService
	// later to retrieve this
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

static const char* getProfileName(AVCodecID id, int profile) {
	AVCodec *c = avcodec_find_decoder(id);
	if (!c)
		return "Unknow";
	return av_get_profile_name(c, profile);
}
static const char* getProfileName(const AVCodecContext* ctx) {
	if (ctx->codec)
		return av_get_profile_name(ctx->codec, ctx->profile);
	return getProfileName(ctx->codec_id, ctx->profile);

}

const d3d_format_t* getFormat(const AVCodecContext *avctx, GUID *selected) {
	unsigned input_count;
	GUID *input_list = NULL;
	DXVA2_VideoDesc videoDesc;
	videoDesc.Format = D3DFMT_UNKNOWN;
	int dxfmt = 0;

	HRESULT rs = d9videoservice->GetDecoderDeviceGuids(&input_count, &input_list);
	if (rs != S_OK) {
		return NULL;
	}

	/* Try all supported mode by our priority */
	const dxva2_mode_t *mode = dxva2_modes;
	for (; mode->name; ++mode) {
		if (!mode->codec || mode->codec != avctx->codec_id) {
			printf("codec does not match to %s: %s\n", avcodec_get_name(avctx->codec_id),
				avcodec_get_name((AVCodecID)mode->codec));
			continue;
		}

		printf("D3D found codec: %s. Check runtime support for the codec.\n", mode->name);
		bool is_supported = false;
		for (unsigned c = 0; c < input_count; c++) {
			GUID g = input_list[c];
			if (IsEqualGUID(*mode->guid, g)) {
				is_supported = true;
				break;
			}
		}
		
		if (is_supported) {
			printf("Check profile support: %s\n", getProfileName(avctx));
			is_supported = checkProfile(mode, avctx->profile);
		}
		if (!is_supported)
			continue;

		dxfmt = fourccFor(mode->guid);
		if (!dxfmt)
			continue;

		if (selected) {
			*selected = *mode->guid;
			break;
		}
	}

	if (input_list) CoTaskMemFree(input_list);

	if (dxfmt) return D3dFindFormat(dxfmt);
	
	printf("No suitable codec or profile found\n");

	return NULL;
}


//
// FFmpeg stuff below
// More global variables:
AVBufferRef *hw_device_ctx;
AVBufferRef *hw_frames_ctx;


// Ref ffmpeg_dxva2.c
bool dxva2_get_decoder_configuration(AVCodecContext *s, const GUID *device_guid,
	const DXVA2_VideoDesc *desc,
	DXVA2_ConfigPictureDecode *config)
{
	unsigned cfg_count = 0, best_score = 0;
	DXVA2_ConfigPictureDecode *cfg_list = NULL;
	DXVA2_ConfigPictureDecode best_cfg = { { 0 } };
	HRESULT hr;

	hr = d9videoservice->GetDecoderConfigurations(*device_guid, desc, NULL, &cfg_count, &cfg_list);
	if (FAILED(hr)) {
		printf("Unable to retrieve decoder configurations\n");
		return false;
	}

	for (unsigned i = 0; i < cfg_count; i++) {
		DXVA2_ConfigPictureDecode *cfg = &cfg_list[i];

		unsigned score;
		if (cfg->ConfigBitstreamRaw == 1)
			score = 1;
		else if (s->codec_id == AV_CODEC_ID_H264 && cfg->ConfigBitstreamRaw == 2)
			score = 2;
		else
			continue;
		if (IsEqualGUID(cfg->guidConfigBitstreamEncryption, DXVA_NoEncrypt))
			score += 16;
		if (score > best_score) {
			best_score = score;
			best_cfg = *cfg;
		}
	}
	CoTaskMemFree(cfg_list);

	if (!best_score) {
		printf("No valid decoder configuration available\n");
		return false;
	}

	*config = best_cfg;
	return true;
}

// Ref ffmpeg_dxva2.c
bool dxva2_create_decoder(AVCodecContext *s) {
	dxva_context *dxva_ctx = (dxva_context *)s->hwaccel_context;
	DXVA2_VideoDesc desc = { 0 };
	DXVA2_ConfigPictureDecode config;
	D3DFORMAT target_format;
	int surface_alignment, num_surfaces;

	AVDXVA2FramesContext *frames_hwctx;
	AVHWFramesContext *frames_ctx;

	// Get the fourcc
	GUID device_guid = GUID_NULL;
	const d3d_format_t *fmt = getFormat(s, &device_guid);
	if (!fmt) return false;
	target_format = (D3DFORMAT)fmt->fourcc;


	desc.SampleWidth = s->coded_width;
	desc.SampleHeight = s->coded_height;
	desc.Format = target_format;

	if (!dxva2_get_decoder_configuration(s, &device_guid, &desc, &config)) {
		return false;
	}

	/* decoding MPEG-2 requires additional alignment on some Intel GPUs,
	but it causes issues for H.264 on certain AMD GPUs..... */
	if (s->codec_id == AV_CODEC_ID_MPEG2VIDEO)
		surface_alignment = 32;
	/* the HEVC DXVA2 spec asks for 128 pixel aligned surfaces to ensure
	all coding features have enough room to work with */
	else if (s->codec_id == AV_CODEC_ID_HEVC)
		surface_alignment = 128;
	else
		surface_alignment = 16;

	/* 4 base work surfaces */
	num_surfaces = 4;

	/* add surfaces based on number of possible refs */
	if (s->codec_id == AV_CODEC_ID_H264 || s->codec_id == AV_CODEC_ID_HEVC)
		num_surfaces += 16;
	else if (s->codec_id == AV_CODEC_ID_VP9)
		num_surfaces += 8;
	else
		num_surfaces += 2;

	/* add extra surfaces for frame threading */
	if (s->active_thread_type & FF_THREAD_FRAME)
		num_surfaces += s->thread_count;

	hw_frames_ctx = av_hwframe_ctx_alloc(hw_device_ctx);
	if (!hw_frames_ctx) return false;

	frames_ctx = (AVHWFramesContext*)hw_frames_ctx->data;
	frames_hwctx = (AVDXVA2FramesContext *)frames_ctx->hwctx;

	frames_ctx->format = AV_PIX_FMT_DXVA2_VLD;
	frames_ctx->sw_format = (target_format == MKTAG('P', '0', '1', '0') ? AV_PIX_FMT_P010 : AV_PIX_FMT_NV12);
	frames_ctx->width = FFALIGN(s->coded_width, surface_alignment);
	frames_ctx->height = FFALIGN(s->coded_height, surface_alignment);
	frames_ctx->initial_pool_size = num_surfaces;

	frames_hwctx->surface_type = DXVA2_VideoDecoderRenderTarget;

	if (av_hwframe_ctx_init(hw_frames_ctx) < 0) {
		printf("Failed to initialize the HW frames context\n");
		av_buffer_unref(&hw_frames_ctx);
		return false;
	}

	HRESULT hr = d9videoservice->CreateVideoDecoder(device_guid,
		&desc, &config, frames_hwctx->surfaces,
		frames_hwctx->nb_surfaces, &frames_hwctx->decoder_to_release);
	if (FAILED(hr)) {
		printf("Failed to create DXVA2 video decoder\n");
		av_buffer_unref(&hw_frames_ctx);
		return false;
	}

	// Do we need to keep a copy of these?
	//ctx->decoder_guid   = device_guid;
	//ctx->decoder_config = config;

	dxva_ctx->cfg = &config;
	dxva_ctx->decoder = frames_hwctx->decoder_to_release;
	dxva_ctx->surface = frames_hwctx->surfaces;
	dxva_ctx->surface_count = frames_hwctx->nb_surfaces;

	if (isIntelClearVideo(&device_guid)) {
		printf("FF_DXVA2_WORKAROUND_INTEL_CLEARVIDEO\n");
		dxva_ctx->workaround |= FF_DXVA2_WORKAROUND_INTEL_CLEARVIDEO;
	}

	return true;
}

// Callback to negotiate the pixel format
// @param  in_fmts  The list of formats which are supported by the codec,
// it is terminated by -1 as 0 is a valid format, the formats are ordered by quality.
// The first is always the native one.
// @return the chosen format
// This callback may be called again immediately if hwaccel init for 
// the chosen pixel format failed.
static enum AVPixelFormat ffmpeg_GetFormat(AVCodecContext *codecCtx,
	const enum AVPixelFormat *in_fmts)
{
	AVPixelFormat fmt = AV_PIX_FMT_DXVA2_VLD;


	AVHWDeviceContext    *device_ctx;
	AVDXVA2DeviceContext *device_hwctx;

	// Notes on device name, if you read the source code of dxva2_device_create,
	// you will see adapter = atoi(device);
	int ret = av_hwdevice_ctx_create(&hw_device_ctx, AV_HWDEVICE_TYPE_DXVA2, NULL, NULL, 0);
	if (ret != 0) {
		printf("av_hwdevice_ctx_create() error\n");
		return AV_PIX_FMT_NONE;
	}

	device_ctx = (AVHWDeviceContext *)hw_device_ctx->data;
	device_hwctx = (AVDXVA2DeviceContext *)device_ctx->hwctx;

	device_hwctx->devmgr = d9devmng;

	dxva_context *dxva_ctx = (dxva_context *)av_mallocz(sizeof(struct dxva_context));
	codecCtx->hwaccel_context = dxva_ctx;


	if (!dxva2_create_decoder(codecCtx)) {
		av_free(dxva_ctx);
		codecCtx->hwaccel_context = NULL;
		av_buffer_unref(&hw_device_ctx);

		printf("Error creating the DXVA2 decoder\n");

		return AV_PIX_FMT_NONE;
	}

	printf("Successfully created DXVA2 decoder.\n");

	const AVPixFmtDescriptor *dsc = av_pix_fmt_desc_get(fmt);
	printf("Trying decoder %s\n", dsc->name);

	return fmt;
}

static int ffmpeg_GetFrameBuf(struct AVCodecContext *ctx, AVFrame *frame, int flags) {
	if (hw_frames_ctx) {
		return av_hwframe_get_buffer(hw_frames_ctx, frame, 0);
	}

	for (unsigned i = 0; i < AV_NUM_DATA_POINTERS; i++) {
		frame->data[i] = NULL;
		frame->linesize[i] = 0;
		frame->buf[i] = NULL;
	}

	return avcodec_default_get_buffer2(ctx, frame, flags);
}

void log_ffmpeg_error(const char *prefix, int i) {
	char err[256] = { 0 };
	av_make_error_string(err, 255, i);
	printf("%s. %s\n", prefix, err);
}

void Decode() {
	// This is in the project directory (not root directory)
	const char *video_file = "snip.mp4";

	av_register_all();
	av_log_set_level(AV_LOG_VERBOSE);

	printf("Input file: %s\n", video_file);

	AVFormatContext *avFormatCtx = NULL;

	if (avformat_open_input(&avFormatCtx, video_file, NULL, NULL) != 0) {
		printf("Could not open input file\n");
		return;
	}

	if (avformat_find_stream_info(avFormatCtx, NULL) < 0) {
		avformat_close_input(&avFormatCtx);
		printf("Could not find stream information\n");
		return;
	}

	int video_index = 0;
	AVCodecContext *codecCtx = NULL;
	AVCodec *codec = NULL;
	AVStream *videoStream = NULL;

	video_index = av_find_best_stream(avFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	if (video_index < 0) {
		avformat_close_input(&avFormatCtx);
		printf("Could not find video stream\n");
		return ;
	}

	videoStream = avFormatCtx->streams[video_index];
	codec = avcodec_find_decoder(videoStream->codecpar->codec_id);
	if (!codec) {
		avformat_close_input(&avFormatCtx);
		printf("Could not find video codec\n");
		return;
	}

	codecCtx = avcodec_alloc_context3(codec);
	avcodec_parameters_to_context(codecCtx, videoStream->codecpar);

	// set callbacks (requires LIBAVCODEC_VERSION_MAJOR >= 55)
	codecCtx->get_format = ffmpeg_GetFormat;
	codecCtx->get_buffer2 = ffmpeg_GetFrameBuf;
	codecCtx->opaque = NULL;

	if (avcodec_open2(codecCtx, codec, NULL) < 0) {
		avcodec_free_context(&codecCtx);
		avformat_close_input(&avFormatCtx);
		printf("avcodec_open2 err\n");
		return;
	}

	av_dump_format(avFormatCtx, 0, video_file, 0);

	printf("Setup complete. Decoding the first frame.\n");

	AVPacket pkt;
	av_init_packet(&pkt);
	pkt.data = NULL;
	pkt.size = 0;

	AVFrame *frame = av_frame_alloc();

	bool again = false; // to deal with EAGAIN from avcodec_receive_frame
	do {
		again = false;

		int read_frame = av_read_frame(avFormatCtx, &pkt);
		if (read_frame >= 0) {

			avcodec_send_packet(codecCtx, &pkt);

			int ret = avcodec_receive_frame(codecCtx, frame);
			if (ret == 0) {

				printf("Successfully decoded the first frame.\n");
				printf("width = %i, height = %i\n", frame->width, frame->height);

				// TODO: download nv12 data from gpu and dump to file, or just render

				break;

			}
			else {
				log_ffmpeg_error("avcodec_receive_frame() error", ret);

				again = (AVERROR(ret) == EAGAIN);
			}
		}
		else {
			log_ffmpeg_error("avcodec_receive_frame() error", read_frame);
		}

		av_packet_unref(&pkt);

	} while (again);

	av_frame_free(&frame);


	if (hw_frames_ctx) av_buffer_unref(&hw_frames_ctx);
	if (hw_device_ctx) av_buffer_unref(&hw_device_ctx);

	if (codecCtx) {
		if (codecCtx->hwaccel_context) av_freep(&codecCtx->hwaccel_context);
		avcodec_free_context(&codecCtx);
	}

	if (avFormatCtx) avformat_close_input(&avFormatCtx);

}