#include "StdAfx.h"
#include "FavClearUpDialog.h"
#include "resource.h"
#include "App.h"
#include "Panel.h"
#include "FavoritesDialog.h"
#include "HelpUtils.h"
#include "LangUtils.h"

using namespace NWindows;
static LPCWSTR kFMHelpTopic = L"dialog/gong_ju.htm";
static CIDLangPair kIDLangPairs[] =
{
	{ IDC_FAVCLEARUP_BUTTON_ADD                   , 0x04000431 },
	{ IDC_FAVCLEARUP_BUTTON_DELETE                , 0x04000432 },
	{ IDC_FAVCLEARUP_BUTTON_EDIT                  , 0x04000433 },
	{ IDC_FAVCLEARUP_BUTTON_MOVEUP                , 0x04000434 },
	{ IDC_FAVCLEARUP_BUTTON_MOVEDOWN              , 0x04000435 },
	{ IDC_FAVCLEARUP_BUTTON_OK                    , 0x05000001 },
	{ IDC_FAVCLEARUP_BUTTON_CHANEL                , 0x05000002 },
	{ IDC_FAVCLEARUP_BUTTON_HELP                  , 0x05000003 },


};
extern CApp g_App;
bool CFavClearUpDialog::OnInit()
{
	LangSetWindowText(HWND(*this), 0x04000436);
	LangSetDlgItemsText(HWND(*this), kIDLangPairs, sizeof(kIDLangPairs) / sizeof(kIDLangPairs[0]));
	g_App.GetBookmark(cStrings);
	g_App.GetFavoritesSign(cSign);
	cStringsKeep = cStrings;
	cSignKeep = cSign;
	ListviewOnInit();
	
	return CModalDialog::OnInit();
}



bool CFavClearUpDialog::OnButtonClicked(int buttonID, HWND buttonHWND)
{
	switch(buttonID)
	{
	case IDC_FAVCLEARUP_BUTTON_ADD:
		g_App.Favorites();
		g_App.GetBookmark(cStrings);
		g_App.GetFavoritesSign(cSign);
		ListviewOnInit();
		break;

	case IDC_FAVCLEARUP_BUTTON_DELETE:
		ItemDelete();
		
		break;

	case IDC_FAVCLEARUP_BUTTON_EDIT:
		FavoritesEdit();
		
		break;
	case IDC_FAVCLEARUP_BUTTON_MOVEUP:
		MoveUp();
		break;
	case IDC_FAVCLEARUP_BUTTON_MOVEDOWN:
		MoveDown();
		break;
	case IDC_FAVCLEARUP_BUTTON_OK:
		OnOK();
		break;
	case IDC_FAVCLEARUP_BUTTON_CHANEL:
		g_App.SetBookmark(cStringsKeep);
		g_App.SetFavoritesSign(cSignKeep);
		OnOK();
		break;
	case IDC_FAVCLEARUP_BUTTON_HELP:
		ShowHelpWindow(NULL,kFMHelpTopic);
		break;

	default:
		return  CModalDialog::OnButtonClicked(buttonID,buttonHWND);
	}
	return false;
}
void CFavClearUpDialog::ListviewOnInit()
{
	favlistview.Attach(GetItem(IDC_FAVCLEARUP_LISTVIEW));
	LV_COLUMNW column;
	column.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	column.cx = 85;
	column.pszText = L"名称";
	favlistview.InsertColumn(1, &column);
	column.pszText = L"路径";
	favlistview.InsertColumn(2, &column);
	column.pszText = L"描述";
	favlistview.InsertColumn(3, &column);
	if (cStrings.Size() == 0)
	return;
	favlistview.DeleteAllItems();
	for(int i =0; i <cStrings.Size(); i++)
	{
		favlistview.InsertItem(i,cStrings.operator [](i));
		favlistview.SetSubItem(i,2,cSign.operator [](i));
	}

}
void CFavClearUpDialog::ItemDelete()
{
	int deleteItem =favlistview.GetFocusedItem();
	if (deleteItem == -1)
	{
		return;
	}
	cStrings.Delete(deleteItem,1);
	cSign.Delete(deleteItem,1);
	g_App.SetBookmark(cStrings);
	g_App.SetFavoritesSign(cSign);
	favlistview.DeleteAllItems();
	for(int i =0; i <cStrings.Size(); i++)
	{
		favlistview.InsertItem(i,cStrings.operator [](i));
		favlistview.SetSubItem(i,2,cSign.operator [](i));
	}
}

void CFavClearUpDialog::FavoritesEdit()
{
	
	int editnumber =favlistview.GetFocusedItem();
	if(editnumber <0)
	{
		::MessageBoxW(HWND(*this),LangString(0x07000011),LangString(0x07000012),MB_ICONWARNING);
		return;
	}
	CFavoritesDialog favorites;
	favorites.GetFloderPath(cStrings.operator [](editnumber));
	favorites.GetSign(cSign.operator [](editnumber));
	favorites.Create(HWND(*this));
	g_App.GetBookmark(cStrings);
	g_App.GetFavoritesSign(cSign);
	favlistview.DeleteAllItems();
	for(int i =0; i <cStrings.Size(); i++)
	{
		favlistview.InsertItem(i,cStrings.operator [](i));
		favlistview.SetSubItem(i,2,cSign.operator [](i));
	}
}
void CFavClearUpDialog::MoveUp()
{
	int moveUp =favlistview.GetFocusedItem();
	if (moveUp == -1)
	{
		return;
	}
	if(moveUp ==0)
	{
		::MessageBoxW(NULL,LangString(0x07000013),LangString(0x07000014),MB_ICONINFORMATION);
		return;
	}
	//向上移动项目
	UString movesign;
	movesign =cSign.operator [](moveUp - 1);
	cSign.operator [](moveUp - 1) =cSign.operator [](moveUp);
	cSign.operator [](moveUp) =movesign;
	UString movestring;
	movestring =cStrings.operator [](moveUp - 1);
	cStrings.operator [](moveUp - 1) =cStrings.operator [](moveUp);
	cStrings.operator [](moveUp) =movestring;

	g_App.SetBookmark(cStrings);
	g_App.SetFavoritesSign(cSign);
	favlistview.DeleteAllItems();
	for(int i =0; i <cStrings.Size(); i++)
	{
		favlistview.InsertItem(i,cStrings.operator [](i));
		favlistview.SetSubItem(i,2,cSign.operator [](i));
	}
	favlistview.SetItemState_FocusedSelected(moveUp - 1);
}
void CFavClearUpDialog::MoveDown()
{
	int moveDown =favlistview.GetFocusedItem();
	int a =favlistview.GetItemCount();
	if(moveDown == favlistview.GetItemCount() - 1)
	{
		::MessageBoxW(NULL,LangString(0x07000015),LangString(0x07000014),MB_ICONINFORMATION);
		return;
	}
	//向下移动项目
	UString movesign;
	movesign =cSign.operator [](moveDown + 1);
	cSign.operator [](moveDown + 1) =cSign.operator [](moveDown);
	cSign.operator [](moveDown) =movesign;
	UString movestring;
	movestring =cStrings.operator [](moveDown + 1);
		cStrings.operator [](moveDown + 1) =cStrings.operator [](moveDown);
	cStrings.operator [](moveDown) =movestring;

	g_App.SetBookmark(cStrings);
	g_App.SetFavoritesSign(cSign);
	favlistview.DeleteAllItems();
	for(int i =0; i <cStrings.Size(); i++)
	{
		favlistview.InsertItem(i,cStrings.operator [](i));
		favlistview.SetSubItem(i,2,cSign.operator [](i));
	}
	favlistview.SetItemState_FocusedSelected(moveDown + 1);
}