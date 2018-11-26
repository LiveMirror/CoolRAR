// AdvancedPage.h

#ifndef __ADVANCED_PAGE_H
#define __ADVANCED_PAGE_H

#include "Windows/Control/OptionPage.h"
#include "../Common/ExtractMode.h"

class CAdvancedPage: public NWindows::NControl::COptionPage
{
   
public:
	int deletemode;
	NExtract::NPathMode::EEnum PathMode;
	virtual bool OnInit();

	virtual bool OnButtonClicked(int buttonID, HWND buttonHWND);
	
};
#endif
