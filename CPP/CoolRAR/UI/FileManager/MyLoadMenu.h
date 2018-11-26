// MyLoadMenu.h

#ifndef __MY_LOAD_MENU_H
#define __MY_LOAD_MENU_H

void OnMenuActivating(HWND hWnd, HMENU hMenu, int position);


bool OnMenuCommand(HWND hWnd, int id);
void MyLoadMenu();
void LoadDriveMenu(HMENU hMenu);
void LoadFileMenu(HMENU hMenu, int startPos, bool programMenu,
    bool isFsFolder, int numItems, bool allAreFiles);
bool ExecuteFileCommand(int id);
bool ContextMenuCommand(int id);
//获取驱动器盘符
void GetDriverName(UString &s,int number );
 //获取驱动器数量
int GetDriverNumber();

#endif
