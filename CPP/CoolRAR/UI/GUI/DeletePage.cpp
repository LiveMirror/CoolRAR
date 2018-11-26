#include "StdAfx.h"
#include "DeletePage.h"
#include "DeletePageRes.h"

#include "Windows/FileDir.h"
#include "Windows/FileName.h"
#include "Windows/System.h"

#include "Common/IntToString.h"
#include "Common/StringConvert.h"

#include "../FileManager/BrowseDialog.h"
#include "../FileManager/FormatUtils.h"
#include "../FileManager/HelpUtils.h"
#include "../FileManager/SplitUtils.h"

#include "../Explorer/MyMessages.h"

#include "../Common/ZipRegistry.h"

#include "../FileManager/LangUtils.h"
#include "../FileManager/HttpUpdat.h"


static CIDLangPair kIDLangPairs[] =
{
	{ IDD_PAGE_DELETE, 0x04000514 },
	{ IDC_DET_PAGE_GROUP_YSSF, 0x04000515 },
	{ IDC_DET_PAGE_TEXT_ONE, 0x04000516 },
	{ IDC_DET_PAGE_TEXT_SIX, 0x04000517 },
	{ IDC_DET_PAGE_TEXT_THREE, 0x04000518 },
	{ IDC_DET_PAGE_TEXT_FOUR, 0x04000519 },
	{ IDC_DET_PAGE_TEXT_FIVE, 0x04000520 },
	{ IDC_DET_PAGE_TEXT_SEVEN, 0x04000521 },
	{ IDC_DET_PAGE_TEXT_NINE, 0x04000522 },
	{ IDC_DET_PAGE_GROUP_PRESSSTYLE, 0x04000523 },
	{ IDC_DET_PAGE_BOX_PRESSSTYLE_ONE, 0x04000524 },
	{ IDC_DET_PAGE_BOX_PRESSSTYLE_TWO, 0x04000525 },
	{ IDC_DET_PAGE_BOX_PRESSSTYLE_THREE, 0x04000526 }
};

extern HttpUpdat httpworkdat;
static LPCWSTR kExeExt = L".exe";
static LPCWSTR k7zFormat = L"7z";
#define UM_METHOD_TIMER WM_USER+10
#define UM_NUMBERT_TIMER WM_USER+11
#define UM_DICTIONARY_TIMER WM_USER+12
#define UM_WORD_TIMER WM_USER+13
#define UM_STRONE_TIMER WM_USER+14
struct CLevelInfo
{
	UInt32 ResourceID;
	UInt32 LangID;
};

static const CLevelInfo g_Levels[] =
{
	{ IDS_METHOD_STORE, 0x02000D81 },
	{ IDS_METHOD_FASTEST, 0x02000D85 },
	{ 0, 0 },
	{ IDS_METHOD_FAST, 0x02000D84 },
	{ 0, 0 },
	{ IDS_METHOD_NORMAL, 0x02000D82 },
	{ 0, 0 },
	{ IDS_METHOD_MAXIMUM, 0x02000D83 },
	{ 0, 0 },
	{ IDS_METHOD_ULTRA, 0x02000D86 }
};

#define MY_SIZE_OF_ARRAY(x) (sizeof(x) / sizeof(x[0]))
using namespace NWindows;
using namespace NFile;
using namespace NName;
using namespace NDirectory;

enum EMethodID
{
	kCopy,
	kLZMA,
	kLZMA2,
	kPPMd,
	kBZip2,
	kDeflate,
	kDeflate64
};

enum ELevel
{
	kStore = 0,
	kFastest = 1,
	kFast = 3,
	kNormal = 5,
	kMaximum = 7,
	kUltra = 9
};


static const LPCWSTR kMethodsNames[] =
{
	L"Copy",
	L"LZMA",
	L"LZMA2",
	L"PPMd",
	L"BZip2",
	L"Deflate",
	L"Deflate64"
};
static const EMethodID g_7zMethods[] =
{
	kLZMA,
	kLZMA2,
	kPPMd,
	kBZip2
};
static const EMethodID g_7zSfxMethods[] =
{
	kCopy,
	kLZMA,
	kLZMA2,
	kPPMd
};

static EMethodID g_ZipMethods[] =
{
	kDeflate,
	kDeflate64,
	kBZip2,
	kLZMA
};

static EMethodID g_GZipMethods[] =
{
	kDeflate
};

static EMethodID g_BZip2Methods[] =
{
	kBZip2
};

static EMethodID g_XzMethods[] =
{
	kLZMA2
};

struct CFormatInfo
{
	LPCWSTR Name;
	UInt32 LevelsMask;
	const EMethodID *MathodIDs;
	int NumMethods;
	bool Filter;
	bool Solid;
	bool MultiThread;
	bool SFX;
	bool Encrypt;
	bool EncryptFileNames;
};
#define METHODS_PAIR(x) x, MY_SIZE_OF_ARRAY(x)

static const CFormatInfo g_Formats[] =
{
	{
		L"",
			(1 << 0) | (1 << 1) | (1 << 3) | (1 << 5) | (1 << 7) | (1 << 9),
			0, 0,
			false, false, false, false, false, false
	},
	{
		k7zFormat,
			(1 << 0) | (1 << 1) | (1 << 3) | (1 << 5) | (1 << 7) | (1 << 9),
			METHODS_PAIR(g_7zMethods),
			true, true, true, true, true, true
		},
		{
			L"Zip",
				(1 << 0) | (1 << 1) | (1 << 3) | (1 << 5) | (1 << 7) | (1 << 9),
				METHODS_PAIR(g_ZipMethods),
				false, false, true, false, true, false
		},
		{
			L"GZip",
				(1 << 1) | (1 << 5) | (1 << 7) | (1 << 9),
				METHODS_PAIR(g_GZipMethods),
				false, false, false, false, false, false
			},
			{
				L"BZip2",
					(1 << 1) | (1 << 3) | (1 << 5) | (1 << 7) | (1 << 9),
					METHODS_PAIR(g_BZip2Methods),
					false, false, true, false, false, false
			},
			{
				L"xz",
					(1 << 1) | (1 << 3) | (1 << 5) | (1 << 7) | (1 << 9),
					METHODS_PAIR(g_XzMethods),
					false, false, true, false, false, false
				},
				{
					L"Tar",
						(1 << 0),
						0, 0,
						false, false, false, false, false, false
				}
};

static bool IsMethodSupportedBySfx(int methodID)
{
	for (int i = 0; i < MY_SIZE_OF_ARRAY(g_7zSfxMethods); i++)
		if (methodID == g_7zSfxMethods[i])
			return true;
	return false;
};


static UInt64 GetMaxRamSizeForProgram()
{
	UInt64 physSize = NSystem::GetRamSize();
	const UInt64 kMinSysSize = (1 << 24);
	if (physSize <= kMinSysSize)
		physSize = 0;
	else
		physSize -= kMinSysSize;
	const UInt64 kMinUseSize = (1 << 24);
	if (physSize < kMinUseSize)
		physSize = kMinUseSize;
	return physSize;
}
bool CDeletePage::OnInit()
{
	LangSetWindowText(HWND(*this), 04000514);
	LangSetDlgItemsText(HWND(*this), kIDLangPairs, sizeof(kIDLangPairs) / sizeof(kIDLangPairs[0]));

	DeleteVisible = true;
	m_Method.Attach(GetItem(IDC_DET_PAGE_COMBO_ONE));
	//m_Level.Attach(GetItem(IDC_DET_PAGE_COMBO_TWO));
	m_Dictionary.Attach(GetItem(IDC_DET_PAGE_COMBO_THREE));
	m_Order.Attach(GetItem(IDC_DET_PAGE_COMBO_FOUR));
	m_Solid.Attach(GetItem(IDC_DET_PAGE_COMBO_FIVE));
	m_NumThreads.Attach(GetItem(IDC_DET_PAGE_COMBO_SIX));
    
	SetMethod();

	 SetSolidBlockSize();

	SetNumThreads();


	 SetMemoryUsage();

	 CheckControlsEnable();
	

	 EnableItem(IDC_DET_PAGE_BOX_PRESSSTYLE_ONE,false);
	 EnableItem(IDC_DET_PAGE_BOX_PRESSSTYLE_TWO,false);
	 EnableItem(IDC_DET_PAGE_BOX_PRESSSTYLE_THREE,false);


	return COptionPage::OnInit();
}

void CDeletePage::SetNumThreads()
{
	m_NumThreads.ResetContent();

	const CFormatInfo &fi = g_Formats[GetStaticFormatIndex()];
	if (!fi.MultiThread)
		return;

	UInt32 numHardwareThreads = NSystem::GetNumberOfProcessors();
	UInt32 defaultValue = numHardwareThreads;

	const CArcInfoEx &ai = (*ArcFormats)[FormatIndex];
	int index = FindRegistryFormat(ai.Name);
	if (index >= 0)
	{
		const NCompression::CFormatOptions &fo = m_RegistryInfo.Formats[index];
		if (fo.Method.CompareNoCase(GetMethodSpec()) == 0 && fo.NumThreads != (UInt32)-1)
			defaultValue = fo.NumThreads;
	}

	UInt32 numAlgoThreadsMax = 1;
	int methodID = GetMethodID();
	switch (methodID)
	{
	case kLZMA: numAlgoThreadsMax = 2; break;
	case kLZMA2: numAlgoThreadsMax = 32; break;
	case kBZip2: numAlgoThreadsMax = 32; break;
	}
	if (IsZipFormat())
		numAlgoThreadsMax = 128;
	for (UInt32 i = 1; i <= numHardwareThreads * 2 && i <= numAlgoThreadsMax; i++)
	{
		TCHAR s[40];
		ConvertUInt32ToString(i, s);
		int index = (int)m_NumThreads.AddString(s);
		m_NumThreads.SetItemData(index, (UInt32)i);
	}
	SetNearestSelectComboBox(m_NumThreads, defaultValue);
}


void CDeletePage::CheckControlsEnable()
{
	const CFormatInfo &fi = g_Formats[GetStaticFormatIndex()];
	Info->SolidIsSpecified = fi.Solid;
	bool multiThreadEnable = fi.MultiThread;
	Info->MultiThreadIsAllowed = multiThreadEnable;
	Info->EncryptHeadersIsAllowed = fi.EncryptFileNames;

	EnableItem(IDC_DET_PAGE_COMBO_FIVE, fi.Solid);
	EnableItem(IDC_DET_PAGE_COMBO_SIX, multiThreadEnable);

}
static const UInt32 kNoSolidBlockSize = 0;
static const UInt32 kSolidBlockSize = 64;
void CDeletePage::SetSolidBlockSize()
{
	m_Solid.ResetContent();
	const CFormatInfo &fi = g_Formats[GetStaticFormatIndex()];
	if (!fi.Solid)
		return;

	UInt32 level = GetLevel2();
	if (level == 0)
		return;

	UInt32 dictionary = GetDictionarySpec();
	if (dictionary == UInt32(-1))
		dictionary = 1;

	UInt32 defaultBlockSize = (UInt32)-1;

	const CArcInfoEx &ai = (*ArcFormats)[FormatIndex];
	int index = FindRegistryFormat(ai.Name);
	if (index >= 0)
	{
		const NCompression::CFormatOptions &fo = m_RegistryInfo.Formats[index];
		if (fo.Method.CompareNoCase(GetMethodSpec()) == 0)
			defaultBlockSize = fo.BlockLogSize;
	}
	
	index = (int)m_Solid.AddString(LangString(IDS_COMPRESS_NON_SOLID, 0x02000D14));
	m_Solid.SetItemData(index, (UInt32)kNoSolidBlockSize);
	m_Solid.SetCurSel(0);
	bool needSet = defaultBlockSize == (UInt32)-1;
	for (int i = 20; i <= 36; i++)
	{
		if (needSet && dictionary >= (((UInt64)1 << (i - 7))) && i <= 32)
			defaultBlockSize = i;
		TCHAR s[40];
		ConvertUInt32ToString(1 << (i % 10), s);
		if (i < 30) lstrcat(s, TEXT(" M"));
		else        lstrcat(s, TEXT(" G"));
		lstrcat(s, TEXT("B"));
		int index = (int)m_Solid.AddString(s);
		m_Solid.SetItemData(index, (UInt32)i);
	}
	index = (int)m_Solid.AddString(LangString(IDS_COMPRESS_SOLID, 0x02000D15));
	m_Solid.SetItemData(index, kSolidBlockSize);
	if (defaultBlockSize == (UInt32)-1)
		defaultBlockSize = kSolidBlockSize;
	if (defaultBlockSize != kNoSolidBlockSize)
		SetNearestSelectComboBox(m_Solid, defaultBlockSize);
}


void CDeletePage::SetLevel()
{
	m_Level.ResetContent();
	const CFormatInfo &fi = g_Formats[GetStaticFormatIndex()];
	const CArcInfoEx &ai = (*ArcFormats)[FormatIndex];
	int index = FindRegistryFormat(ai.Name);
	UInt32 level = kNormal;
	if (index >= 0)
	{
		const NCompression::CFormatOptions &fo = m_RegistryInfo.Formats[index];
		if (fo.Level <= kUltra)
			level = fo.Level;
		else
			level = kUltra;
	}
	int i;
	for (i = 0; i <= kUltra; i++)
	{
		if ((fi.LevelsMask & (1 << i)) != 0)
		{
			const CLevelInfo &levelInfo = g_Levels[i];
			int index = (int)m_Level.AddString(LangString(levelInfo.ResourceID, levelInfo.LangID));
			m_Level.SetItemData(index, i);
		}
	}
	UString s;
	m_Level.GetText(s);
	SetNearestSelectComboBox(m_Level, level);
	m_Level.GetText(s);

	
}

UInt32 CDeletePage::GetComboValue(NWindows::NControl::CComboBox &c, int defMax)
{
	if (c.GetCount() <= defMax)
		return (UInt32)-1;
	return (UInt32)c.GetItemData(c.GetCurSel());
}

void CDeletePage::SetMethod(int keepMethodId)
{
	m_Method.ResetContent();
	UInt32 level = GetLevel();
	if (level == 0)
	{
		SetDictionary();
		SetOrder();
		return;
	}
	const CFormatInfo &fi = g_Formats[GetStaticFormatIndex()];
	const CArcInfoEx &ai = (*ArcFormats)[FormatIndex];
	int index = FindRegistryFormat(ai.Name);
	UString defaultMethod;
	if (index >= 0)
	{
		const NCompression::CFormatOptions &fo = m_RegistryInfo.Formats[index];
		defaultMethod = fo.Method;
	}
	bool isSfx = Issfx;
	bool weUseSameMethod = false;
	for (int m = 0; m < fi.NumMethods; m++)
	{
		EMethodID methodID = fi.MathodIDs[m];
		if (isSfx)
			if (!IsMethodSupportedBySfx(methodID))
				continue;
		const LPCWSTR method = kMethodsNames[methodID];
		int itemIndex = (int)m_Method.AddString(GetSystemString(method));

		if (keepMethodId == methodID)
		{
			m_Method.SetCurSel(itemIndex);
			weUseSameMethod = true;
			continue;
		}
		if ((defaultMethod.CompareNoCase(method) == 0 || m == 0) && !weUseSameMethod)
			m_Method.SetCurSel(itemIndex);
	}
	if (!weUseSameMethod)
	{
		SetDictionary();
		SetOrder();
	}
}


void CDeletePage::SetNearestSelectComboBox(NControl::CComboBox &comboBox, UInt32 value)
{
	for (int i = comboBox.GetCount() - 1; i >= 0; i--)
		if ((UInt32)comboBox.GetItemData(i) <= value)
		{
			comboBox.SetCurSel(i);
			return;
		}
		if (comboBox.GetCount() > 0)
			comboBox.SetCurSel(0);
}



int CDeletePage::FindRegistryFormatAlways(const UString &name)
{
	int index = FindRegistryFormat(name);
	if (index < 0)
	{
		NCompression::CFormatOptions fo;
		fo.FormatID = GetSystemString(name);
		index = m_RegistryInfo.Formats.Add(fo);
	}
	return index;
}

int CDeletePage::FindRegistryFormat(const UString &name)
{
	for (int i = 0; i < m_RegistryInfo.Formats.Size(); i++)
	{
		const NCompression::CFormatOptions &fo = m_RegistryInfo.Formats[i];
		if (name.CompareNoCase(GetUnicodeString(fo.FormatID)) == 0)
			return i;
	}
	return -1;
}


int CDeletePage::GetStaticFormatIndex()
{
	 ;
	const CArcInfoEx &ai = (*ArcFormats)[FormatIndex];
	for (int i = 0; i < MY_SIZE_OF_ARRAY(g_Formats); i++)
		if (ai.Name.CompareNoCase(g_Formats[i].Name) == 0)
			return i;
	return 0; // -1;
}

UString CDeletePage::GetMethodSpec()
{
	if (m_Method.GetCount() <= 1)
	{
		return UString();
	}
	UString result;
	m_Method.GetText(result);

	return result;
}

int CDeletePage::GetMethodID()
{
	UString methodName;
	m_Method.GetText(methodName);
	for (int i = 0; i < MY_SIZE_OF_ARRAY(kMethodsNames); i++)
		if (methodName.CompareNoCase(kMethodsNames[i]) == 0)
			return i;
	return -1;
}
UInt32 CDeletePage::GetLevel2()
{
	UInt32 level = GetLevel();
	if (level == (UInt32)-1)
		level = 5;
	return level;
}

void CDeletePage::SetDictionary()
{
	m_Dictionary.ResetContent();
	int Index = FormatIndex;
	const CArcInfoEx &ai = (*ArcFormats)[Index];
	int index = FindRegistryFormat(ai.Name);
	UInt32 defaultDictionary = UInt32(-1);
	if (index >= 0)
	{
		const NCompression::CFormatOptions &fo = m_RegistryInfo.Formats[index];
		if (fo.Method.CompareNoCase(GetMethodSpec()) == 0)
			defaultDictionary = fo.Dictionary;
	}
	int methodID = GetMethodID();
	UInt32 level = GetLevel2();
	if (methodID < 0)
		return;
	const UInt64 maxRamSize = GetMaxRamSizeForProgram();
	switch (methodID)
	{
	case kLZMA:
	case kLZMA2:
		{
			static const UInt32 kMinDicSize = (1 << 16);
			if (defaultDictionary == UInt32(-1))
			{
				if (level >= 9)      defaultDictionary = (1 << 26);
				else if (level >= 7) defaultDictionary = (1 << 25);
				else if (level >= 5) defaultDictionary = (1 << 24);
				else if (level >= 3) defaultDictionary = (1 << 20);
				else                 defaultDictionary = (kMinDicSize);
			}
			int i;
			AddDictionarySize(kMinDicSize);
			m_Dictionary.SetCurSel(0);
			for (i = 20; i <= 30; i++)
				for (int j = 0; j < 2; j++)
				{
					if (i == 20 && j > 0)
						continue;
					UInt32 dictionary = (1 << i) + (j << (i - 1));
					if (dictionary >
#ifdef _WIN64
						(1 << 30)
#else
						(1 << 26)
#endif
						)
						continue;
					AddDictionarySize(dictionary);
					UInt64 decomprSize;
					UInt64 requiredComprSize = GetMemoryUsage(dictionary, decomprSize);
					if (dictionary <= defaultDictionary && requiredComprSize <= maxRamSize)
						m_Dictionary.SetCurSel(m_Dictionary.GetCount() - 1);
				}

				// SetNearestSelectComboBox(m_Dictionary, defaultDictionary);
				break;
		}
	case kPPMd:
		{
			if (defaultDictionary == UInt32(-1))
			{
				if (level >= 9)      defaultDictionary = (192 << 20);
				else if (level >= 7) defaultDictionary = ( 64 << 20);
				else if (level >= 5) defaultDictionary = ( 16 << 20);
				else                 defaultDictionary = (  4 << 20);
			}
			int i;
			for (i = 20; i < 31; i++)
				for (int j = 0; j < 2; j++)
				{
					if (i == 20 && j > 0)
						continue;
					UInt32 dictionary = (1 << i) + (j << (i - 1));
					if (dictionary >= (1 << 31))
						continue;
					AddDictionarySize(dictionary);
					UInt64 decomprSize;
					UInt64 requiredComprSize = GetMemoryUsage(dictionary, decomprSize);
					if (dictionary <= defaultDictionary && requiredComprSize <= maxRamSize || m_Dictionary.GetCount() == 0)
						m_Dictionary.SetCurSel(m_Dictionary.GetCount() - 1);
				}
				SetNearestSelectComboBox(m_Dictionary, defaultDictionary);
				break;
		}
	case kDeflate:
		{
			AddDictionarySize(32 << 10);
			m_Dictionary.SetCurSel(0);
			break;
		}
	case kDeflate64:
		{
			AddDictionarySize(64 << 10);
			m_Dictionary.SetCurSel(0);
			break;
		}
	case kBZip2:
		{
			// UInt32 defaultDictionary;
			if (defaultDictionary == UInt32(-1))
			{
				if (level >= 5)
					defaultDictionary = (900 << 10);
				else if (level >= 3)
					defaultDictionary = (500 << 10);
				else
					defaultDictionary = (100 << 10);
			}
			for (int i = 1; i <= 9; i++)
			{
				UInt32 dictionary = (i * 100) << 10;
				AddDictionarySize(dictionary);
				if (dictionary <= defaultDictionary || m_Dictionary.GetCount() == 0)
					m_Dictionary.SetCurSel(m_Dictionary.GetCount() - 1);
			}
			break;
		}
	}
}

int CDeletePage::AddDictionarySize(UInt32 size)
{
	if (size > 0)
	{
		if ((size & 0xFFFFF) == 0)
			return AddDictionarySize(size, false, true);
		if ((size & 0x3FF) == 0)
			return AddDictionarySize(size, true, false);
	}
	return AddDictionarySize(size, false, false);
}

int CDeletePage::AddDictionarySize(UInt32 size, bool kilo, bool maga)
{
	UInt32 sizePrint = size;
	if (kilo)
		sizePrint >>= 10;
	else if (maga)
		sizePrint >>= 20;
	TCHAR s[40];
	ConvertUInt32ToString(sizePrint, s);
	if (kilo)
		lstrcat(s, TEXT(" K"));
	else if (maga)
		lstrcat(s, TEXT(" M"));
	else
		lstrcat(s, TEXT(" "));
	lstrcat(s, TEXT("B"));
	int index = (int)m_Dictionary.AddString(s);
	m_Dictionary.SetItemData(index, size);
	return index;
}
UInt64 CDeletePage::GetMemoryUsage(UInt32 dictionary, UInt64 &decompressMemory)
{
	decompressMemory = UInt64(Int64(-1));
	UInt32 level = GetLevel2();
	if (level == 0)
	{
		decompressMemory = (1 << 20);
		return decompressMemory;
	}
	UInt64 size = 0;

	const CFormatInfo &fi = g_Formats[GetStaticFormatIndex()];
	if (fi.Filter && level >= 9)
		size += (12 << 20) * 2 + (5 << 20);
	UInt32 numThreads = GetNumThreads2();
	if (IsZipFormat())
	{
		UInt32 numSubThreads = 1;
		if (GetMethodID() == kLZMA && numThreads > 1 && level >= 5)
			numSubThreads = 2;
		UInt32 numMainThreads = numThreads / numSubThreads;
		if (numMainThreads > 1)
			size += (UInt64)numMainThreads << 25;
	}
	int methidId = GetMethodID();
	switch (methidId)
	{
	case kLZMA:
	case kLZMA2:
		{
			UInt32 hs = dictionary - 1;
			hs |= (hs >> 1);
			hs |= (hs >> 2);
			hs |= (hs >> 4);
			hs |= (hs >> 8);
			hs >>= 1;
			hs |= 0xFFFF;
			if (hs > (1 << 24))
				hs >>= 1;
			hs++;
			UInt64 size1 = (UInt64)hs * 4;
			size1 += (UInt64)dictionary * 4;
			if (level >= 5)
				size1 += (UInt64)dictionary * 4;
			size1 += (2 << 20);

			UInt32 numThreads1 = 1;
			if (numThreads > 1 && level >= 5)
			{
				size1 += (2 << 20) + (4 << 20);
				numThreads1 = 2;
			}
			UInt32 numBlockThreads = numThreads / numThreads1;
			if (methidId == kLZMA || numBlockThreads == 1)
				size1 += (UInt64)dictionary * 3 / 2;
			else
			{
				UInt64 chunkSize = (UInt64)dictionary << 2;
				chunkSize = MyMax(chunkSize, (UInt64)(1 << 20));
				chunkSize = MyMin(chunkSize, (UInt64)(1 << 28));
				chunkSize = MyMax(chunkSize, (UInt64)dictionary);
				size1 += chunkSize * 2;
			}
			size += size1 * numBlockThreads;

			decompressMemory = dictionary + (2 << 20);
			return size;
		}
	case kPPMd:
		{
			decompressMemory = dictionary + (2 << 20);
			return size + decompressMemory;
		}
	case kDeflate:
	case kDeflate64:
		{
			UInt32 order = GetOrder();
			if (order == UInt32(-1))
				order = 32;
			if (level >= 7)
				size += (1 << 20);
			size += 3 << 20;
			decompressMemory = (2 << 20);
			return size;
		}
	case kBZip2:
		{
			decompressMemory = (7 << 20);
			UInt64 memForOneThread = (10 << 20);
			return size + memForOneThread * numThreads;
		}
	}
	return UInt64(Int64(-1));
}

bool CDeletePage::IsZipFormat()
{
	const CArcInfoEx &ai = (*ArcFormats)[FormatIndex];
	return (ai.Name.CompareNoCase(L"zip") == 0);
}

void CDeletePage::SetOrder()
{
	m_Order.ResetContent();
	const CArcInfoEx &ai = (*ArcFormats)[FormatIndex];
	int index = FindRegistryFormat(ai.Name);
	UInt32 defaultOrder = UInt32(-1);
	if (index >= 0)
	{
		const NCompression::CFormatOptions &fo = m_RegistryInfo.Formats[index];
		if (fo.Method.CompareNoCase(GetMethodSpec()) == 0)
			defaultOrder = fo.Order;
	}
	int methodID = GetMethodID();
	UInt32 level = GetLevel2();
	if (methodID < 0)
		return;
	switch (methodID)
	{
	case kLZMA:
	case kLZMA2:
		{
			if (defaultOrder == UInt32(-1))
				defaultOrder = (level >= 7) ? 64 : 32;
			for (int i = 3; i <= 8; i++)
				for (int j = 0; j < 2; j++)
				{
					UInt32 order = (1 << i) + (j << (i - 1));
					if (order <= 256)
						AddOrder(order);
				}
				AddOrder(273);
				SetNearestSelectComboBox(m_Order, defaultOrder);
				break;
		}
	case kPPMd:
		{
			if (defaultOrder == UInt32(-1))
			{
				if (level >= 9)
					defaultOrder = 32;
				else if (level >= 7)
					defaultOrder = 16;
				else if (level >= 5)
					defaultOrder = 6;
				else
					defaultOrder = 4;
			}
			int i;
			AddOrder(2);
			AddOrder(3);
			for (i = 2; i < 8; i++)
				for (int j = 0; j < 4; j++)
				{
					UInt32 order = (1 << i) + (j << (i - 2));
					if (order < 32)
						AddOrder(order);
				}
				AddOrder(32);
				SetNearestSelectComboBox(m_Order, defaultOrder);
				break;
		}
	case kDeflate:
	case kDeflate64:
		{
			if (defaultOrder == UInt32(-1))
			{
				if (level >= 9)
					defaultOrder = 128;
				else if (level >= 7)
					defaultOrder = 64;
				else
					defaultOrder = 32;
			}
			int i;
			for (i = 3; i <= 8; i++)
				for (int j = 0; j < 2; j++)
				{
					UInt32 order = (1 << i) + (j << (i - 1));
					if (order <= 256)
						AddOrder(order);
				}
				AddOrder(methodID == kDeflate64 ? 257 : 258);
				SetNearestSelectComboBox(m_Order, defaultOrder);
				break;
		}
	case kBZip2:
		{
			break;
		}
	}
}


int CDeletePage::AddOrder(UInt32 size)
{
	TCHAR s[40];
	ConvertUInt32ToString(size, s);
	int index = (int)m_Order.AddString(s);
	m_Order.SetItemData(index, size);
	return index;
}

void CDeletePage::SetMemoryUsage()
{
	UInt64 decompressMem;
	UInt64 memUsage = GetMemoryUsage(decompressMem);
	PrintMemUsage(IDC_DET_PAGE_TEXT_EIGHT, memUsage);
	PrintMemUsage(IDC_DET_PAGE_TEXT_TEN, decompressMem);
}

UInt64 CDeletePage::GetMemoryUsage(UInt64 &decompressMemory)
{
	return GetMemoryUsage(GetDictionary(), decompressMemory);
}
void CDeletePage::PrintMemUsage(UINT res, UInt64 value)
{
	if (value == (UInt64)(Int64)-1)
	{
		SetItemText(res, TEXT("?"));
		return;
	}
	value = (value + (1 << 20) - 1) >> 20;
	TCHAR s[40];
	ConvertUInt64ToString(value, s);
	lstrcat(s, TEXT(" MB"));
	SetItemText(res, s);
}

void CDeletePage::GetHwndFromGeneral(HWND newwindos)
{
	m_Level.Attach(newwindos);
}
void CDeletePage::SettingPressLevel()
{
	const CArcInfoEx &ai = (*ArcFormats)[FormatIndex];
	int index = FindRegistryFormatAlways(ai.Name);
	NCompression::CFormatOptions &fo = m_RegistryInfo.Formats[index];
	fo.ResetForLevelChange();
	SetMethod();
	SetSolidBlockSize();
	SetNumThreads();

	SetMemoryUsage();
	SaveInfo();
}
bool CDeletePage::OnCommand(int code, int itemID, LPARAM lParam)
{

	if (code == CBN_SELCHANGE)
	{
		switch(itemID)
		{
		case IDC_DET_PAGE_COMBO_ONE://算法
			{
				SetTimer(UM_METHOD_TIMER,10);
			  return true;
			}
		case IDC_DET_PAGE_COMBO_TWO://压缩等级（May be wo don't need this?）
			{
				
				const CArcInfoEx &ai = (*ArcFormats)[FormatIndex];
				int index = FindRegistryFormatAlways(ai.Name);
				NCompression::CFormatOptions &fo = m_RegistryInfo.Formats[index];
				fo.ResetForLevelChange();
				SetMethod();
				SetSolidBlockSize();
				SetNumThreads();
				SetMemoryUsage();
				SaveInfo();
				
				return true;
			}
		case IDC_DET_PAGE_COMBO_THREE:
		case IDC_DET_PAGE_COMBO_FOUR://单词字典
			{
				SetTimer(UM_DICTIONARY_TIMER,10);
				return true;
			}
		case IDC_DET_PAGE_COMBO_SIX://线程
			{
				SetTimer(UM_NUMBERT_TIMER,10);
				return true;
			}
		case IDC_DET_PAGE_COMBO_FIVE:
			{
				SaveInfo();
				return true;
			}

		}
	}
	return COptionPage::OnCommand(code, itemID, lParam);
}

void CDeletePage::SaveOptionsInMem()
{
	const CArcInfoEx &ai = (*ArcFormats)[Info->FormatIndex];
	int index = FindRegistryFormatAlways(ai.Name);
	Info->Options.Trim();
	NCompression::CFormatOptions &fo = m_RegistryInfo.Formats[index];
	UString method = fo.Method;
	fo.Options = Info->Options;
    
	if(fo.NumThreads > 2)//当线程数大于2时说明丢失默认压缩数据
	{
		fo.NumThreads = 2;
	}
	if (fo.Level > 9)
	{
		fo.Level = 5;
	}
	if (Info->Level > 9)
	{
		Info->Level = fo.Level;
	}

	if (!method.IsEmpty()
		&&GetMethodSpec().IsEmpty())
	{
		if(method ==L"PPMd")
		{
			fo.NumThreads =1;
			Info->NumThreads = 1;
			UInt32 solidLogSize = fo.BlockLogSize;
			if (solidLogSize > 0 && solidLogSize != (UInt32)-1)
			Info->SolidBlockSize = (solidLogSize>= 64) ? (UInt64)(Int64)-1 : ((UInt64)1 << solidLogSize);
			Info->Order = fo.Order;
			Info->Dictionary = fo.Dictionary;
		   return; 
		}
	
	}
	
	fo.Level = GetLevelSpec();

	if ((int)GetDictionarySpec() !=-1 )
	fo.Dictionary =GetDictionarySpec();

	if ((int)GetOrderSpec() !=-1 )
	fo.Order = GetOrderSpec();

	if(! GetMethodSpec().IsEmpty())//如果deletepage没有初始化控件则跳过使用默认值
	fo.Method = GetMethodSpec();

	if(GetNumThreadsSpec() < 3)
	fo.NumThreads = GetNumThreadsSpec();
	
	if ((int)GetBlockSizeSpec() !=-1 )
	fo.BlockLogSize = GetBlockSizeSpec();

	if ((int)fo.Dictionary !=-1 && (int)Info->Dictionary == -1)//如果未丢失默认压缩数据
	{
		Info->Dictionary = fo.Dictionary;//字典
		Info->NumThreads = fo.NumThreads;//线程个数

		int sodblock = fo.BlockLogSize;//设置固实大小
		Info->SolidBlockSize =(sodblock >= 64) ? (UInt64)(Int64)-1 : ((UInt64)1 << sodblock);

		if(fo.Method !=L"BZip2")//假如算法为BZIP2则不需要对单词进行初始化
		Info->Order = fo.Order;

		Info->Method = fo.Method;//开始设置算法
		if(Info->Method == L"LZMA2")
			Info->Method = L"LZMA";
	}

	if (ai.Name == L"7z"
		&& (int)fo.Dictionary == -1)//如果使用7Z压缩格式而且丢失默认压缩数据
	{
		static const UInt32 kMinDicSize = (1 << 16);
		if (fo.Dictionary == UInt32(-1))
		{
			if (fo.Level >= 9)      fo.Dictionary = (1 << 26);
			else if (fo.Level >= 7) fo.Dictionary = (1 << 25);
			else if (fo.Level >= 5) fo.Dictionary = (1 << 24);
			else if (fo.Level >= 3) fo.Dictionary = (1 << 20);
			else                 fo.Dictionary = (kMinDicSize);

			if (fo.Level >= 9)      Info->Dictionary = (1 << 26);
			else if (fo.Level >= 7) Info->Dictionary = (1 << 25);
			else if (fo.Level >= 5) Info->Dictionary = (1 << 24);
			else if (fo.Level >= 3) Info->Dictionary = (1 << 20);
			else                 Info->Dictionary = (kMinDicSize);
		}
		if (fo.BlockLogSize == UInt32(-1) &&fo.Method !=L"BZip2")
		{
			
			    fo.BlockLogSize = 31;
				Info->SolidBlockSize = 2147483648;
		}
		if(fo.Order == UINT(-1) )
		{
			if (fo.Level >= 9)      fo.Order = 64;
			else if (fo.Level >= 7) fo.Order = 32;
			else if (fo.Level >= 5) fo.Order = 32;
			else if (fo.Level >= 3) fo.Order = 32;

			if (fo.Level >= 9)      Info->Order = 64;
			else if (fo.Level >= 7) Info->Order = 32;
			else if (fo.Level >= 5) Info->Order = 32;
			else if (fo.Level >= 3) Info->Order = 32;
			
		}
		if (fo.Method.IsEmpty())
		{
			fo.Method =L"LZMA";
			Info->Method =L"LZMA";
		}
		if (Info->NumThreads > 2)
		{
			Info->NumThreads = 2;
		}
		
	}



	if (ai.Name == L"zip")
	{
	
		Info->SolidBlockSize = 0;
		if(fo.Order == UInt32(-1) )
		{
			
			if (fo.Level >= 9)
				fo.Order = 128;
			else if (fo.Level >= 7)
				fo.Order = 64;
			else
				fo.Order = 32;


			if (fo.Level >= 9)      Info->Order = 128;
			else if (fo.Level >= 7) Info->Order = 64;
			else Info->Order = 32;

		}
		if (fo.Method.IsEmpty())
		{
			fo.Method =L"Deflate";
			Info->Method =L"Deflate";
		}
		if(Info->NumThreads > 4 || Info->NumThreads < 1)
		Info->NumThreads = 2;

	}

}


bool CDeletePage::GetOrderMode()
{
	switch (GetMethodID())
	{
	case kPPMd:
		return true;
	}
	return false;
}
void CDeletePage ::SaveInfo()
{
	SaveOptionsInMem();
	const CArcInfoEx &ai = (*ArcFormats)[Info->FormatIndex];
	Info->Level = GetLevelSpec();
	Info->Dictionary = GetDictionarySpec();
	Info->Order = GetOrderSpec();
	Info->OrderMode = GetOrderMode();
	Info->NumThreads = GetNumThreadsSpec();
	if (Info->Method ==L"PPMd")
	{
		Info->NumThreads = 1;
	}
	
	UInt32 solidLogSize = GetBlockSizeSpec();
	Info->SolidBlockSize = 0;
	if (solidLogSize > 0 && solidLogSize != (UInt32)-1)
		Info->SolidBlockSize = (solidLogSize >= 64) ? (UInt64)(Int64)-1 : ((UInt64)1 << solidLogSize);

	Info->Method = GetMethodSpec();
	if (ai.Name == L"7z"
		&& (int)Info->Dictionary == -1)
	{
		static const UInt32 kMinDicSize = (1 << 16);
		if (Info->Dictionary == UInt32(-1))
		{


			if (Info->Level >= 9)      Info->Dictionary = (1 << 26);
			else if (Info->Level >= 7) Info->Dictionary = (1 << 25);
			else if (Info->Level >= 5) Info->Dictionary = (1 << 24);
			else if (Info->Level >= 3) Info->Dictionary = (1 << 20);
			else                 Info->Dictionary = (kMinDicSize);
		}
		if (solidLogSize == (UInt32)-1)
		{
			Info->SolidBlockSize = 2147483648;
		}
		if(Info->Order == UINT(-1) )
		{

			if (Info->Level >= 9)      Info->Order = 64;
			else if (Info->Level >= 7) Info->Order = 32;
			else if (Info->Level >= 5) Info->Order = 32;
			else if (Info->Level >= 3) Info->Order = 32;

		}
		if(Info->NumThreads > 2)
			Info->NumThreads = 2;
		if (Info->Method.IsEmpty())
		{
			Info->Method =L"LZMA";
		}

	}
	if (ai.Name == L"zip"
		&& Info->Method.IsEmpty())
	{

		if(Info->Order == UInt32(-1) )
		{

			if (Info->Level >= 9)
				Info->Order = 128;
			else if (Info->Level >= 7)
				Info->Order = 64;
			else
				Info->Order = 32;


			if (Info->Level >= 9)      Info->Order = 128;
			else if (Info->Level >= 7) Info->Order = 64;
			else Info->Order = 32;

		}
		if(Info->NumThreads > 2)
		Info->NumThreads = 2;
		if (Info->Method.IsEmpty())
		{
			
			Info->Method =L"Deflate";
		}

	}
	
	

}
void CDeletePage::OnItInfo()
{
	const CFormatInfo &fi = g_Formats[GetStaticFormatIndex()];
	const CArcInfoEx &ai = (*ArcFormats)[FormatIndex];
	int index = FindRegistryFormat(ai.Name);
	UInt32 level = kNormal;
	if (index >= 0)
	{
		const NCompression::CFormatOptions &fo = m_RegistryInfo.Formats[index];
		if (fo.Level <= kUltra)
			level = fo.Level;
		else
			level = kUltra;

		Info->Level = level;
		if (level != 0)
		{
			UString defaultMethod;
			Info->Method = fo.Method;
			
		}
		
		
		
		UInt32 solidLogSize = 0;
		Info->SolidBlockSize = 0;
		if (fi.Solid && level != 0)
		{
			UInt32 defaultBlockSize = (UInt32)-1;
			if (fo.Method.CompareNoCase(GetMethodSpec()) == 0)
				solidLogSize = fo.BlockLogSize;

			if (solidLogSize > 0 && solidLogSize != (UInt32)-1)
				Info->SolidBlockSize = (solidLogSize >= 64) ? (UInt64)(Int64)-1 : ((UInt64)1 << solidLogSize);
		}
		Info->MultiThreadIsAllowed = true;
		if(fi.MultiThread)
		{
			UInt32 numHardwareThreads = NSystem::GetNumberOfProcessors();
			UInt32 defaultValue = numHardwareThreads;
			if (fo.Method.CompareNoCase(GetMethodSpec()) == 0 && fo.NumThreads != (UInt32)-1)
				defaultValue = fo.NumThreads;
			Info->NumThreads = defaultValue;
		}
		else 
		{
			Info->MultiThreadIsAllowed = false;
		}

		Info->Dictionary = fo.Dictionary;
		Info->Order = fo.Order;

		Info->EncryptionMethod = fo.EncryptionMethod;
			Info->EncryptHeadersIsAllowed = fi.EncryptFileNames;
			Info->EncryptHeaders = m_RegistryInfo.EncryptHeaders;

	}

}
bool CDeletePage::OnTimer(WPARAM wp, LPARAM lp )
{
	if ( wp == UM_METHOD_TIMER)
	{
		KillTimer(UM_METHOD_TIMER);
		SetDictionary();
		SetOrder();
		SetSolidBlockSize();
		SetNumThreads();
		SetMemoryUsage();
		SaveInfo();
		return true;
	}

	if ( wp == UM_DICTIONARY_TIMER)
	{
		KillTimer(UM_DICTIONARY_TIMER);
		SetSolidBlockSize();
		SetMemoryUsage();
		SaveInfo();
		return true;
	}
	if( wp ==UM_NUMBERT_TIMER)
	{
		KillTimer(UM_NUMBERT_TIMER);
		SetMemoryUsage();
		SaveInfo();
		return true;
	}

	return true;

}