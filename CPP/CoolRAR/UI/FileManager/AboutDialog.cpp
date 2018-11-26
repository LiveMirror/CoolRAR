// AboutDialog.cpp

#include "StdAfx.h"

#include "AboutDialog.h"
#include "HelpUtils.h"
#include "LangUtils.h"

static CIDLangPair kIDLangPairs[] =
{
  { IDC_ABOUT_BUTTON_HOMEPAGE               , 0x04000370 },
  { TEXT_BANQUAN                            , 0x04000371 },
  { TEXT_CoolRAR                            , 0x04000372 },
  { IDOK                                    , 0x05000001 },
};

#define MY_HOME_PAGE TEXT("http://www.CoolRAR.com/")

static LPCTSTR kHomePageURL     = MY_HOME_PAGE;
static LPCWSTR kHelpTopic = L"start.htm";

bool CAboutDialog::OnInit()
{
  LangSetWindowText(HWND(*this), 0x04000403);
  LangSetDlgItemsText(HWND(*this), kIDLangPairs, sizeof(kIDLangPairs) / sizeof(kIDLangPairs[0]));
  NormalizePosition();
  return CModalDialog::OnInit();
}

void CAboutDialog::OnHelp()
{
  ShowHelpWindow(NULL, kHelpTopic);
}

bool CAboutDialog::OnButtonClicked(int buttonID, HWND buttonHWND)
{
  LPCTSTR url;
  switch(buttonID)
  {
    case IDC_ABOUT_BUTTON_HOMEPAGE: url = kHomePageURL; break;
    default:
      return CModalDialog::OnButtonClicked(buttonID, buttonHWND);
  }

  #ifdef UNDER_CE
  SHELLEXECUTEINFO s;
  memset(&s, 0, sizeof(s));
  s.cbSize = sizeof(s);
  s.lpFile = url;
  ::ShellExecuteEx(&s);
  #else
  ::ShellExecute(NULL, NULL, url, NULL, NULL, SW_SHOWNORMAL);
  #endif

  return true;
}
