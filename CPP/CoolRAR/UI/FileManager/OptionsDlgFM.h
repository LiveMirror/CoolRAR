
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
		NUpdateMode::EEnum UpdateMode;//����ģʽ
		bool SolidIsSpecified;//ָ���˹�ʵѹ��
		bool MultiThreadIsAllowed;//���߳�
		UInt64 SolidBlockSize;//��ʵ��С
		UInt32 NumThreads;//�߳���
		bool deleteSourceFile; //�Ƿ�ɾ��Դ�ļ� 
		CRecordVector<UInt64> VolumeSizes;//�־��С

		UInt32 Level;//ѹ����ʽ
		UString Method;//ѹ���㷨
		UInt32 Dictionary;//�ֵ��С
		bool OrderMode;
		UInt32 Order;
		UString Options;

		UString EncryptionMethod;//�����㷨

		bool SFXMode;
		bool OpenShareForWrite;


		UString ArchiveName; // in: Relative for ; out: abs//�ļ���
		UString CurrentDirPrefix;
		bool KeepName;
		UString CommentsValue;

		bool GetFullPathName(UString &result) const;

		int FormatIndex;//��ʽ���
		UString Password;//����
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