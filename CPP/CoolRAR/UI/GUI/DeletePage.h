// DeletePage.h

#ifndef __DELETE_PAGE_H
#define __DELETE_PAGE_H


#include "Windows/Control/OptionPage.h"
#include "../Common/ExtractMode.h"
#include "OptionsDlg.h"
#include "Windows/Control/ComboBox.h"

class CDeletePage: public NWindows::NControl::COptionPage
{

public:
	NOptinsAddDlg::CInfo* Info;
	CObjectVector<CArcInfoEx> *ArcFormats;
	CRecordVector<int> ArcIndices;
	UString* OriginalFileName; 
	NCompression::CInfo m_RegistryInfo;	
	int FormatIndex;//压缩格式序号
	UString FormatName;
	bool Issfx;//是不是自解压格式
	UString ArchivePath;
	bool DeleteVisible;
	bool GetOrderMode();
	int FindRegistryFormat(const UString &name);
	void SetMethod(int keepMethodId = -1);
	int GetStaticFormatIndex();
	int AddOrder(UInt32 size);
	void SetOrder();
	int AddDictionarySize(UInt32 size, bool kilo, bool maga);
	void SetNearestSelectComboBox(NWindows::NControl::CComboBox &comboBox, UInt32 value);

	int FindRegistryFormatAlways(const UString &name);

	NWindows::NControl::CComboBox m_Level;
	NWindows::NControl::CComboBox m_Method;
	NWindows::NControl::CComboBox m_Dictionary;
	NWindows::NControl::CComboBox m_NumThreads;
	NWindows::NControl::CComboBox m_Order;//单词大小
	NWindows::NControl::CComboBox m_Solid;//固实数据大小
	void SetLevel();
	UInt64 GetMemoryUsage(UInt64 &decompressMemory);
	void SetMemoryUsage();
	void PrintMemUsage(UINT res, UInt64 value);
	UInt32 GetLevelSpec(){return GetComboValue(m_Level, 1); }
	UInt32 GetLevel() 	{ 		return GetComboValue(m_Level);}
	UInt32 GetOrderSpec() 	{ 	return GetComboValue(m_Order, 1);}
	UInt32 GetBlockSizeSpec()	{ return  GetComboValue(m_Solid, 1);}
	void SetNumThreads();
	UInt32 GetDictionarySpec()	{ return GetComboValue(m_Dictionary, 1);}
	UInt32 GetComboValue(NWindows::NControl::CComboBox &c, int defMax = 0);
	UInt32 GetDictionary() { return GetComboValue(m_Dictionary); }
	void CheckControlsEnable();
	void SetSolidBlockSize();
	UString GetMethodSpec();
	int GetMethodID();
	UInt32 GetLevel2();
	void SetDictionary();
	int AddDictionarySize(UInt32 size);
	UInt64 GetMemoryUsage(UInt32 dictionary, UInt64 &decompressMemory);
	UInt32 GetOrder() { return GetComboValue(m_Order); }
	UInt32 GetNumThreads2() 
	{ 
		UInt32 num = GetNumThreadsSpec(); 
		if (num == UInt32(-1)) num = 1;
		return num; 
	}
	UInt32 GetNumThreadsSpec(){ return GetComboValue(m_NumThreads, 1);}
	bool IsZipFormat();

	void SaveInfo();
	virtual bool OnInit();
	virtual bool OnCommand(int code, int itemID, LPARAM lParam);
	virtual bool OnTimer(WPARAM , LPARAM );
	void SaveOptionsInMem();
	void OnItInfo();
	void GetHwndFromGeneral(HWND newwindos);
	void SettingPressLevel();
	
};
#endif
