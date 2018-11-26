// AppState.h

#ifndef __APP_STATE_H
#define __APP_STATE_H

#include "Windows/Synchronization.h"

#include "ViewSettings.h"
#include "LangUtils.h"




void inline AddUniqueStringToHead(UStringVector &list,
								  const UString &string)
{
	for(int i = 0; i < list.Size();)
		if (string.CompareNoCase(list[i]) == 0)
			list.Delete(i);
		else
			i++;
	list.Insert(0, string);
}

class CFastFolders
{
	NWindows::NSynchronization::CCriticalSection _criticalSection;
public:
	UStringVector Strings;
	UStringVector Sign;

	int GetStringSize()
	{
		return Strings.Size();
	}
	void SetString(int index, const UString &string)
	{
		NWindows::NSynchronization::CCriticalSectionLock lock(_criticalSection);
		while(Strings.Size() <= index)
			Strings.Add(UString());
		Strings[index] = string;
	}
	void SetString(const UString &string)
	{
		NWindows::NSynchronization::CCriticalSectionLock lock(_criticalSection);
		if(Strings.Find(string) != -1)
		{
			::MessageBoxW(NULL,LangString(0x07000030),LangString(0x07000002),MB_ICONWARNING);
			return;
		}
		else
		{
			Strings.Add(string);	
		}	   
	}
	void SetString(const UStringVector &setString)
	{
		NWindows::NSynchronization::CCriticalSectionLock lock(_criticalSection);
		Strings =setString;
	}
	void GetString(UStringVector &getString)
	{
		NWindows::NSynchronization::CCriticalSectionLock lock(_criticalSection);
		getString =Strings;
	}
	UString GetString(int index)
	{
		NWindows::NSynchronization::CCriticalSectionLock lock(_criticalSection);
		if (index >= Strings.Size())
			return UString();
		return Strings[index];
	}


	void SetSign(const UString &string,const UString &sign)
	{
		NWindows::NSynchronization::CCriticalSectionLock lock(_criticalSection);
		int count =Strings.Find(string);
		while(Sign.Size()<= count)
		{
			Sign.Add(UString());
		}
		Sign.Insert(count ,sign);
	}
	void SetSign(const UStringVector &setSign)
	{
		NWindows::NSynchronization::CCriticalSectionLock lock(_criticalSection);
		Sign = setSign;
	}
	void GetSign(const UString &string,UString &sign)
	{
		NWindows::NSynchronization::CCriticalSectionLock lock(_criticalSection);
		int count =Strings.Find(string);
		if(Sign.Size() == 0)
			return ;
		sign =Sign.operator [](count);
	}
	void GetSign(UStringVector &getSign)
	{
		NWindows::NSynchronization::CCriticalSectionLock lock(_criticalSection);
		getSign = Sign;
	}
	void Save()
	{
		NWindows::NSynchronization::CCriticalSectionLock lock(_criticalSection);
		SaveFastFolders(Strings);
		SaveFastFoldersSign(Sign);
	}
	void Read()
	{
		NWindows::NSynchronization::CCriticalSectionLock lock(_criticalSection);
		ReadFastFolders(Strings);
		ReadFastFoldersSign(Sign);
	}
};
class CThemeTitle
{
	NWindows::NSynchronization::CCriticalSection _criticalSection;
public:
	UStringVector ThemeStr;
	UStringVector ThemeFolderName;

	int GetStringSize()
	{
		return ThemeStr.Size();
	}
	void SetString(int index, const UString &string)
	{
		NWindows::NSynchronization::CCriticalSectionLock lock(_criticalSection);
		while(ThemeStr.Size() <= index)
			ThemeStr.Add(UString());
		ThemeStr[index] = string;
	}
	void SetString(const UString &string)
	{
		NWindows::NSynchronization::CCriticalSectionLock lock(_criticalSection);
		if(ThemeStr.Find(string)==-1)
		{
			ThemeStr.Add(string);

		}
		else
		{
			::MessageBoxW(NULL,LangString(0x07000031),LangString(0x07000032),MB_ICONINFORMATION);
			return;
		}	   
	}
	void SetString(const UStringVector &setString)
	{
		NWindows::NSynchronization::CCriticalSectionLock lock(_criticalSection);
		ThemeStr =setString;
	}
	void GetString(UStringVector &getString)
	{
		NWindows::NSynchronization::CCriticalSectionLock lock(_criticalSection);
		getString =ThemeStr;
	}
	UString GetString(int index)
	{
		NWindows::NSynchronization::CCriticalSectionLock lock(_criticalSection);
		if (index >= ThemeStr.Size())
			return UString();
		return ThemeStr[index];
	}

	void SetFolderName(const UString &Name)
	{
		NWindows::NSynchronization::CCriticalSectionLock lock(_criticalSection);
		if(ThemeFolderName.Find(Name)==-1)
		{
			ThemeFolderName.Add(Name);

		}
		else
		{
			return;
		}					
	}
	void SetFolderName(const UStringVector &FolderName)
	{
		NWindows::NSynchronization::CCriticalSectionLock lock(_criticalSection);
		ThemeFolderName = FolderName;
	}
	void GetFolderName(const UString &string,UString &Name)
	{
		NWindows::NSynchronization::CCriticalSectionLock lock(_criticalSection);
		int count =ThemeStr.Find(string);
		if(ThemeFolderName.Size() == 0)
			return ;
		Name =ThemeFolderName.operator [](count);
	}
	void GetFolderName(UStringVector &FolderName)
	{
		NWindows::NSynchronization::CCriticalSectionLock lock(_criticalSection);
		FolderName = ThemeFolderName;
	}
	void Save()
	{
		NWindows::NSynchronization::CCriticalSectionLock lock(_criticalSection);
		SaveThemeTitle(ThemeStr);
		SaveThemeFolderName(ThemeFolderName);
	}
	void Read()
	{
		NWindows::NSynchronization::CCriticalSectionLock lock(_criticalSection);
		ReadThemeTitle(ThemeStr);
		ReadThemeFolderName(ThemeFolderName);
	}

};

class CFolderHistory
{
	NWindows::NSynchronization::CCriticalSection _criticalSection;
	UStringVector Strings;
	void Normalize()
	{
		const int kMaxSize = 100;
		if (Strings.Size() > kMaxSize)
			Strings.Delete(kMaxSize, Strings.Size() - kMaxSize);
	}

public:
	int GetSize()
	{
		return Strings.Size();
	}
	void GetList(UStringVector &foldersHistory)
	{
		NWindows::NSynchronization::CCriticalSectionLock lock(_criticalSection);
		foldersHistory = Strings;
	}

	void AddString(const UString &string)
	{


		NWindows::NSynchronization::CCriticalSectionLock lock(_criticalSection);
		AddUniqueStringToHead(Strings, string);
		Normalize();


	}


	void RemoveAll()
	{
		NWindows::NSynchronization::CCriticalSectionLock lock(_criticalSection);
		Strings.Clear();
	}

	void Save()
	{
		NWindows::NSynchronization::CCriticalSectionLock lock(_criticalSection);
		SaveFolderHistory(Strings);
	}

	void Read()
	{
		NWindows::NSynchronization::CCriticalSectionLock lock(_criticalSection);
		ReadFolderHistory(Strings);
		Normalize();
	}
};

struct CAppState
{
	CFastFolders FastFolders;
	CFolderHistory FolderHistory;
	CThemeTitle    ThemeTitle;
	void Save()
	{
		FastFolders.Save();
		FolderHistory.Save();
		ThemeTitle.Save();
	}
	void Read()
	{
		FastFolders.Read();
		FolderHistory.Read();
		ThemeTitle.Read();
	}
};


#endif