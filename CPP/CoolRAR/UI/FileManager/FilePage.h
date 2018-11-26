// FilePage.h

#ifndef __FILE_PAGE_H
#define __FILE_PAGE_H


#include "Windows/Control/OptionPage.h"
#include "../Common/ExtractMode.h"

class CFilePage: public NWindows::NControl::COptionPage
{
public: 
	virtual bool OnInit();
};
#endif