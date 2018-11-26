#ifndef _IS_ARCHIVE_SFX_H_
#define _IS_ARCHIVE_SFX_H_

#include "../../../Common/MyString.h"

/*
* S_OK 自解压
* E_FAIL 不能解压
*/
HRESULT IsArchiveSFX(const UString& file,UString dllpath);

#endif