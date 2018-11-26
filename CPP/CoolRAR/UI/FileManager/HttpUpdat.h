#ifndef __HTTP_UPDAT_H
#define __HTTP_UPDAT_H
#include "StdAfx.h"
#include "Windows/Control/Dialog.h"
#include "Windows/DLL.h"

#include "ProgramLocation.h"
#include "resource.h"


using namespace NWindows;
#ifndef UNDER_CE
typedef bool (WINAPI * DllRegisterServerPtr)();
typedef bool (WINAPI * DllRunUpdate)(bool mesbox);
typedef bool (WINAPI * DllGetUpdatPtr)(const wchar_t *m);
typedef bool (WINAPI * DllGetUpdatPP)(const wchar_t *m,const wchar_t * p);
class HttpUpdat
{
public:
    HttpUpdat(void);
	~HttpUpdat(void);
	NWindows::NDLL::CLibrary lib;
	bool DllUpdatExe(bool mesbox);
	bool DestroyDll();
	bool DllGetUpdat();
	bool CreatShortcut();
	bool GetMD5Info(UString filepath);

};

#endif
#endif
