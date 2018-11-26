#include "StdAfx.h"
#include <stdio.h>
#include "IsArchiveSFX.h"
#include "../../Archive/IArchive.h"
#include "../../Common/FileStreams.h"
#include "../../../Common/MyCom.h"
#include "../../../Windows/PropVariant.h"

using namespace NWindows;

const GUID CLSID_CArchiveHandler2   = { 0x23170F69, 0x40C1, 0x278A, { 0x10, 0x00,  0x00,  0x01,  0x10,  0xdd,  0x00,  0x00 } };
const GUID IID_IInArchive2			= { 0x23170F69, 0x40C1, 0x278A, { 0x00, 0x00,  0x00,  0x06,  0x00,  0x60,  0x00,  0x00 } };

BOOL IsValidPEFile( UString strPathName )
{

	HANDLE hFile = CreateFileW( strPathName, 
		GENERIC_READ, 
		FILE_SHARE_READ, 
		NULL, 
		OPEN_EXISTING, 
		FILE_ATTRIBUTE_NORMAL, 
		NULL );
	if ( hFile == INVALID_HANDLE_VALUE ) {
		//TRACE1( "Failed To Open File %s !\n", strPathName );
		return FALSE;
	}

	HANDLE hMMFile = CreateFileMapping( hFile, NULL, PAGE_READONLY, 0, 0, NULL );
	if ( hMMFile == INVALID_HANDLE_VALUE ) {
		CloseHandle( hFile );
		return FALSE;
	}

	LPVOID pvMem = MapViewOfFile( hMMFile, FILE_MAP_READ, 0, 0, 0 );
	if ( ! pvMem ) {
		CloseHandle( hMMFile );
		CloseHandle( hFile );
		return FALSE;
	}

	if ( *( USHORT* ) pvMem != IMAGE_DOS_SIGNATURE ) {
		UnmapViewOfFile( pvMem );
		CloseHandle( hMMFile );
		CloseHandle( hFile );
		return FALSE;
	}

	if ( *( ( DWORD* ) ( ( PBYTE ) pvMem + ( ( PIMAGE_DOS_HEADER ) pvMem )->e_lfanew ) ) != IMAGE_NT_SIGNATURE ) {
		UnmapViewOfFile( pvMem );
		CloseHandle( hMMFile );
		CloseHandle( hFile );
		return FALSE;
	}

	UnmapViewOfFile( pvMem );
	CloseHandle( hMMFile );
	CloseHandle( hFile );

	return TRUE;
}
BOOL isWinZip(UString strPathName)
{
	HANDLE hFile = CreateFileW( strPathName, 
		GENERIC_READ, 
		FILE_SHARE_READ, 
		NULL, 
		OPEN_EXISTING, 
		FILE_ATTRIBUTE_NORMAL, 
		NULL );
	if ( hFile == INVALID_HANDLE_VALUE ) {
		return FALSE;
	}

	HANDLE hMMFile = CreateFileMapping( hFile, NULL, PAGE_READONLY, 0, 0, NULL );
	if ( hMMFile == INVALID_HANDLE_VALUE ) {
		CloseHandle( hFile );
		return FALSE;
	}

	LPVOID pvMem = MapViewOfFile( hMMFile, FILE_MAP_READ, 0, 0, 0 );
	if ( ! pvMem ) {
		CloseHandle( hMMFile );
		CloseHandle( hFile );
		return FALSE;
	}
	//查找NT头
	IMAGE_DOS_HEADER *pDosHdr = (IMAGE_DOS_HEADER *)pvMem;
	IMAGE_NT_HEADERS *pNTHdr = (IMAGE_NT_HEADERS *)((char *)pvMem + pDosHdr->e_lfanew);

	//段的数目
	int iSecNum = pNTHdr->FileHeader.NumberOfSections;
	//找到段表的开始,NT头指针 + NT头大小
	IMAGE_SECTION_HEADER *pSecHdr = (IMAGE_SECTION_HEADER *)((char *)pNTHdr + sizeof(IMAGE_NT_HEADERS));
	for(int i = 0; i < iSecNum; i++)
	{
		const char* _winzip="_winzip_";
		//sprintf(_winzip,sizeof(_winzip),26,"_winzip_");

		AString str=(AString)(char *)pSecHdr->Name;

		int a =str.Compare(_winzip);
		if (str.Compare(_winzip)>=0)
		{
			
			UnmapViewOfFile( pvMem );
			CloseHandle( hMMFile );
			CloseHandle( hFile );
			return TRUE;
		}
		pSecHdr++;
	}

	UnmapViewOfFile( pvMem );
	CloseHandle( hMMFile );
	CloseHandle( hFile );
	return FALSE;
}

HRESULT IsArchiveSFX(const UString& filePath,UString dllpath)
{
	IInArchive* archive;
	HRESULT hr = E_FAIL;

	if (!IsValidPEFile(filePath))
	{
		return E_FAIL;
	}
	if (isWinZip(filePath))
	{
		return S_OK;
	}

	const UInt64 kMaxCheckStartPos = 1 << 22;
	typedef UINT32 (WINAPI * CreateObjectPointer)(const GUID *clsID, const GUID *interfaceID, void **outObject);

	HINSTANCE hCoolRAR = LoadLibraryW(dllpath + L"CoolRAR.dll");
	if ( !hCoolRAR ) return E_FAIL;

	CreateObjectPointer createObjectPointer = (CreateObjectPointer)::GetProcAddress(hCoolRAR, "CreateObject");

	if ( createObjectPointer )
	{
		hr = createObjectPointer(&CLSID_CArchiveHandler2, &IID_IInArchive2, (void**)&archive);

		

		if ( S_OK != hr ) 
		{
			FreeLibrary(hCoolRAR);
			return E_FAIL;		
		}

		CInFileStream file;
		if ( !file.Open(filePath))
		{
			FreeLibrary(hCoolRAR);
			return E_FAIL;		
		}


		hr =  archive->Open(&file, &kMaxCheckStartPos, NULL);
		if ( S_OK != hr)
		{	
			FreeLibrary(hCoolRAR);
			return S_OK;
		}
		FreeLibrary(hCoolRAR);
		return E_FAIL;
	}
	FreeLibrary(hCoolRAR);
	return E_FAIL;

}