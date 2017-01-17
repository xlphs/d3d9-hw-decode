// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>


// TODO: reference additional headers your program requires here

#include <stdlib.h>
#include <string>
#include <iostream>

extern "C" {
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/avstring.h>
#include <libavutil/pixdesc.h>
#include <libavutil/mem.h>

// for dxva_context, which ffmpeg says:
// The application must make it available as AVCodecContext.hwaccel_context.
#include <libavcodec/dxva2.h>

#include <libavutil/hwcontext.h>
#include <libavutil/hwcontext_dxva2.h>

}
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")

#include <d3d9.h>
#include <dxva2api.h>
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "dxva2.lib")
