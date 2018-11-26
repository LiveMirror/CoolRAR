
#ifndef __OPTIONS_DLG_H
#define __OPTIONS_DLG_H


#include "../Common/ZipRegistry.h"

#include "../Common/LoadCodecs.h"

namespace NOptinsAddDlg
{
	namespace NUpdateMode
	{
		enum EEnum
		{
			kAdd,
			kUpdate,
			kFresh,
			kSynchronize,
		};
	}
	struct CInfo
	{
		NUpdateMode::EEnum UpdateMode;//更新模式
		bool SolidIsSpecified;//指定了固实压缩
		bool MultiThreadIsAllowed;//多线程
		UInt64 SolidBlockSize;//固实大小
		UInt32 NumThreads;//线程数
		bool deleteSourceFile; //是否删除源文件 
		CRecordVector<UInt64> VolumeSizes;//分卷大小

		UInt32 Level;//压缩方式
		UString Method;//压缩算法
		UInt32 Dictionary;//字典大小
		bool OrderMode;
		UInt32 Order;
		UString Options;

		UString EncryptionMethod;//加密算法

		bool SFXMode;
		bool OpenShareForWrite;


		UString ArchiveName; // in: Relative for ; out: abs//文件名
		UString CurrentDirPrefix;
		bool KeepName;
		UString CommentsValue;

		bool GetFullPathName(UString &result) const;

		int FormatIndex;//格式序号
		UString Password;//密码
		bool EncryptHeadersIsAllowed;
		bool EncryptHeaders;
		void Init()
		{
			Level = Dictionary = Order = UInt32(-1);
			OrderMode = false;
			Method.Empty();
			Options.Empty();
			EncryptionMethod.Empty();
		}
		CInfo()
		{
			Init();
		}
	};

}
class OptionsDlg
{
public :
	INT_PTR OptionsAddDialog(HWND hwndOwner, HINSTANCE /* hInstane */);
	NOptinsAddDlg::CInfo Info;
	CObjectVector<CArcInfoEx> *ArcFormats;
	CRecordVector<int> ArcIndices;
	UString OriginalFileName; 
};

#endif