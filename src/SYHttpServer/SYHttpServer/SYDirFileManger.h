
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
			OutputDebugString(L"�L�k�������x�s�]��\n");
			break;

		case DRIVE_NO_ROOT_DIR:
			OutputDebugString(L"���x�s�]�Ƥ��s�b\n");
			break;

		case DRIVE_REMOVABLE:
			OutputDebugString(L"���ʦ��x�s�]��(�n��)\n");
			break;

		case DRIVE_FIXED:
			OutputDebugString(L"�T�w���x�s�]��(�w��)\n");
			break;

		case DRIVE_REMOTE:
			OutputDebugString(L"���ݪ��x�s�]��(�����Ϻ�)\n");
			break;

		case DRIVE_CDROM:
			OutputDebugString(L"CDROM���о�\n");
			break;

		case DRIVE_RAMDISK:
			OutputDebugString(L"RAM DRIVE\n");
			break;

		default: 
			OutputDebugString(L"�����x�s�]�ƥ���\n");
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
