// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             //  �� Windows ͷ�ļ����ų�����ʹ�õ���Ϣ
//#include <windows.h>
#include <strmif.h>
#include <uuids.h>

// TODO:  �ڴ˴����ó�����Ҫ������ͷ�ļ�
#include "libutils.h"
#include "streams.h"
#include "libcamera.h"
#include "libface.h"
#include "UIlib.h"
using namespace e;
using namespace DuiLib;

#ifdef _DEBUG
#pragma comment(lib, "libutilsd.lib")
#pragma comment(lib, "libstreamd.lib")
#pragma comment(lib, "libcamerad.lib")
#pragma comment(lib, "libfaced.lib")
#pragma comment(lib, "libduid.lib")
#else
#pragma comment(lib, "libutils.lib")
#pragma comment(lib, "libstream.lib")
#pragma comment(lib, "libcamera.lib")
#pragma comment(lib, "libface.lib")
#pragma comment(lib, "libdui.lib")
#endif
