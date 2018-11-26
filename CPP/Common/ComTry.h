// ComTry.h

#ifndef __COM_TRY_H
#define __COM_TRY_H

#include "MyWindows.h"


#define COM_TRY_BEGIN try {
#define COM_TRY_END } catch(...) { return E_OUTOFMEMORY; }
  


#endif
