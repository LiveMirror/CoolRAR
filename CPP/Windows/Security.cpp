// Windows/Security.cpp

#include "StdAfx.h"

#include "Security.h"

namespace NWindows {
namespace NSecurity {
  
static void SetLsaString(LPWSTR src, PLSA_UNICODE_STRING dest)
{
  int len = (int)wcslen(src);
  dest->Length = (USHORT)(len * sizeof(WCHAR));
  dest->MaximumLength = (USHORT)((len + 1) * sizeof(WCHAR));
  dest->Buffer = src;
}


#ifndef _UNICODE
typedef BOOL (WINAPI * LookupAccountNameWP)(
    LPCWSTR lpSystemName,
    LPCWSTR lpAccountName,
    PSID Sid,
    LPDWORD cbSid,
    LPWSTR ReferencedDomainName,
    LPDWORD cchReferencedDomainName,
    PSID_NAME_USE peUse
    );
#endif

static PSID GetSid(LPWSTR accountName)
{
  #ifndef _UNICODE
  HMODULE hModule = GetModuleHandle(TEXT("Advapi32.dll"));
  if (hModule == NULL)
    return NULL;
  LookupAccountNameWP lookupAccountNameW = (LookupAccountNameWP)GetProcAddress(hModule, "LookupAccountNameW");
  if (lookupAccountNameW == NULL)
    return NULL;
  #endif

  DWORD sidLen = 0, domainLen = 0;
  SID_NAME_USE sidNameUse;
  if (!
    #ifdef _UNICODE
    ::LookupAccountNameW
    #else
    lookupAccountNameW
    #endif
    (NULL, accountName, NULL, &sidLen, NULL, &domainLen, &sidNameUse))
  {
    if (::GetLastError() == ERROR_INSUFFICIENT_BUFFER)
    {
      PSID pSid = ::HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sidLen);
      LPWSTR domainName = (LPWSTR)::HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (domainLen + 1) * sizeof(WCHAR));
      BOOL res =
        #ifdef _UNICODE
        ::LookupAccountNameW
        #else
        lookupAccountNameW
        #endif
        (NULL, accountName, pSid, &sidLen, domainName, &domainLen, &sidNameUse);
      ::HeapFree(GetProcessHeap(), 0, domainName);
      if (res)
        return pSid;
    }
  }
  return NULL;
}

#define MY__SE_LOCK_MEMORY_NAME L"SeLockMemoryPrivilege"

bool AddLockMemoryPrivilege()
{
  CPolicy policy;
  LSA_OBJECT_ATTRIBUTES attr;
  attr.Length = sizeof(attr);
  attr.RootDirectory = NULL;
  attr.ObjectName  = NULL;
  attr.Attributes = 0;
  attr.SecurityDescriptor = NULL;
  attr.SecurityQualityOfService  = NULL;
  if (policy.Open(NULL, &attr,
      POLICY_ALL_ACCESS)
      != 0)
    return false;
  LSA_UNICODE_STRING userRights;
  wchar_t s[128] = MY__SE_LOCK_MEMORY_NAME;
  SetLsaString(s, &userRights);
  WCHAR userName[256 + 2];
  DWORD size = 256;
  if (!GetUserNameW(userName, &size))
    return false;
  PSID psid = GetSid(userName);
  if (psid == NULL)
    return false;
  bool res = false;

  {
    NTSTATUS status = policy.AddAccountRights(psid, &userRights);
    if (status == 0)
      res = true;
  }
  HeapFree(GetProcessHeap(), 0, psid);
  return res;
}

}}

