
#ifndef _SYDIRFILE_H_
#define _SYDIRFILE_H_ 
#include <Windows.h>
#include <vector>

#ifdef BCB
typedef unsigned char  	BYTE;
typedef wchar_t 		WCHAR;
#else
#endif


class SYDirInfo
{
public:
	SYDirInfo()
	{
		memset(wszDrive, 0, sizeof(wszDrive));
		lTotal = 0;
		lFree  = 0;
		iType  = DRIVE_UNKNOWN;
	}

	void Print()
	{
		WCHAR buf[512];
		wsprintf(buf, L"Drive:%s Total:%d MByte Free:%d MByte\n", wszDrive, lTotal, lFree);
		OutputDebugString(buf);

		switch (iType)
		{
		case DRIVE_UNKNOWN:
			OutputDebugString(L"無法偵測的儲存設備\n");
			break;

		case DRIVE_NO_ROOT_DIR:
			OutputDebugString(L"此儲存設備不存在\n");
			break;

		case DRIVE_REMOVABLE:
			OutputDebugString(L"移動式儲存設備(軟碟)\n");
			break;

		case DRIVE_FIXED:
			OutputDebugString(L"固定式儲存設備(硬碟)\n");
			break;

		case DRIVE_REMOTE:
			OutputDebugString(L"遠端的儲存設備(網路磁碟)\n");
			break;

		case DRIVE_CDROM:
			OutputDebugString(L"CDROM光碟機\n");
			break;

		case DRIVE_RAMDISK:
			OutputDebugString(L"RAM DRIVE\n");
			break;

		default: 
			OutputDebugString(L"偵測儲存設備失敗\n");
		}
	}
public:
	WCHAR wszDrive[80];	
	int   iType;	 
	int  lTotal;
	int  lFree;
};

class SYDirFileInfo
{
public:
	SYDirFileInfo()
	{
		strName = L"";
		IsDirectory = false;
	}
public:
	std::wstring strName;
	bool		 IsDirectory;
};


class SYDirFileManger
{
public:	
	//creat
	static SYDirFileManger* Instance(void);
	~SYDirFileManger(void);
	//method
	std::vector<SYDirInfo*>* GetDrives(void);
	bool IsExistWithFileName(const WCHAR *pPath);
	WCHAR *GetWindowsPath(void);
	WCHAR *GetProgramFilePath(void);
	void CopyFrom(WCHAR *pFromPath, WCHAR *pToPath);
	std::vector<std::wstring>* EnumDirWithDirPath(const WCHAR *Path);
	std::vector<SYDirFileInfo*>* EnumDirPlusWithDirPath(const WCHAR *pPath);
	bool EnumDirPlusWithDirPath(const WCHAR *pPath, std::vector<SYDirFileInfo*>* pVecPlusEnum);
private:
	//method
	SYDirFileManger(void);	
	

private:
	//var
	//creat
	static SYDirFileManger*		m_instance;		
	//dir
	std::vector<SYDirInfo*>*	m_pVecDirInfo;
	//enum dir
	std::vector<std::wstring>*  m_pVecEnumDir;
	std::vector<SYDirFileInfo*>*   m_pVecEnumPlusDir;
	//win folder
	WCHAR m_szWindowsFolder[256];
};

#endif
