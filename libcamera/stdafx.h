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
using namespace e;

#ifdef _DEBUG
#pragma comment(lib, "libutilsd.lib")
#pragma comment(lib, "libstreamd.lib")
#else
#pragma comment(lib, "libutils.lib")
#pragma comment(lib, "libstream.lib")
#endif

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "Quartz.lib")