// GeneralPage.h

#ifndef __GENERAL_PAGE_H
#define __GENERAL_PAGE_H


#include "Windows/Control/OptionPage.h"
#include "../Common/ExtractMode.h"

#include "Windows/Control/ComboBox.h"
#include "../FileManager/SplitUtils.h"
#include "../Common/ZipRegistry.h"
#include "OptionsDlgFM.h"
#include "../Common/LoadCodecs.h"
#include "DeletePage.h"


struct FormatIdex
{
	int index;
	int arcIndex;
};
typedef CObjectVector<FormatIdex> FormatIdexVector;

class CGeneralPage: public NWindows::NControl::COptionPage
{
public:

	CDeletePage* Delete;
	NWindows::NControl::CComboBox m_ArchivePath;//文件名
	NWindows::NControl::CComboBox m_UpdateMode;//更新
	NWindows::NControl::CComboBox m_Volume;//分卷大小
	FormatIdexVector formatIdexVer;
	int m_PrevFormat;
	void SetVolumeSize();
	bool SetFormat(int i);
	void SetArchiveName(const UString &name);
	int GetFormatSur();
	int GetFormatIndex(int iformat);
	bool IsSFX();
	void CheckControlsEnable();
	void CheckSOLIDControlsEnable();
	int GetStaticFormatIndex();
	void OnButtonSetArchive();
	void CheckSFXControlsEnable();
	int FindRegistryFormat(const UString &name);
	void SaveOptionsInMem();
	void OnButtonSFX();
	void CheckVolumeEnable();
	int FindRegistryFormatAlways(const UString &name);
	void  SetArchiveName2(bool prevWasSFX);
	virtual bool OnInit();
	virtual bool OnCommand(int code, int itemID, LPARAM lParam);
	virtual bool OnButtonClicked(int buttonID, HWND buttonHWND);
	void SaveInfo();
	
};
#endif


