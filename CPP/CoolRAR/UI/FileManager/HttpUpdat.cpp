#include "StdAfx.h"
#include "HttpUpdat.h"

HttpUpdat::HttpUpdat(void)
{

	UString prefix;
	GetProgramFolderPath(prefix);
	
	if (!lib.Load(prefix + L"CoolZip.dll"))
	{
	}

}

HttpUpdat::~HttpUpdat(void)
{

}

bool HttpUpdat::DllUpdatExe(bool mesbox)
{

	char *name="UpdatExe";
	DllRunUpdate f = (DllRunUpdate)lib.GetProc(name);
	if (f == NULL)
	{
		return false;
	}
	bool res = f(mesbox);
	return res;

}

bool HttpUpdat::DllGetUpdat()
{
	
	char *name="GetUpdat";
	DllRegisterServerPtr f = (DllRegisterServerPtr)lib.GetProc(name);
	if (f == NULL)
	{
		return false;
	}
	bool res = f();
	return res;
}




bool HttpUpdat::CreatShortcut()
{
	char *name="CreatOrDeleteShortcut";
	DllRegisterServerPtr f=(DllRegisterServerPtr)lib.GetProc(name);
	if(f == NULL)
	{
		return false;
	}
	bool res = f();
	return res;
}

bool HttpUpdat::GetMD5Info(UString filepath)
{
	char *name="GetMD5ForFileManager";
	DllGetUpdatPtr f=(DllGetUpdatPtr)lib.GetProc(name);
	if(f == NULL)
	{
		return false;
	}
	bool res = f(filepath);
	return res;
}

bool HttpUpdat::DestroyDll()
{
	return lib.Free();
}
