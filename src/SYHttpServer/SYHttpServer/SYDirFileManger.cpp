#include "stdafx.h"

#include <shlobj.h>
#include <Shellapi.h> //for CopyFrom

#ifdef BCB
#include <errno.h>
#include <stdio.h>
int __cdecl _wfopen_s( FILE **pFile, const wchar_t *filename, const wchar_t *mode )
{
	*pFile = _wfopen(filename, mode);
	return errno;
}
typedef wchar_t 		WCHAR;
#else
#endif

#include "SYDirFileManger.h"
BOOL flag_Debug = false;
//=================================================================================================================================
SYDirFileManger* SYDirFileManger::m_instance = NULL;
//=================================================================================================================================
SYDirFileManger* SYDirFileManger::Instance()
{
	if (m_instance == NULL){
		m_instance = new SYDirFileManger(); 
	}
	return m_instance;
}

SYDirFileManger::SYDirFileManger()
{
	m_pVecDirInfo = new std::vector<SYDirInfo*>;
	m_pVecEnumDir = new std::vector<std::wstring>;
	m_pVecEnumPlusDir = new std::vector<SYDirFileInfo*>;
}

SYDirFileManger::~SYDirFileManger()
{
	if (m_pVecDirInfo)
	{
		//clear
		while(!m_pVecDirInfo->empty())
		{
			delete m_pVecDirInfo->back();
			m_pVecDirInfo->pop_back();
		}
		delete m_pVecDirInfo;
	}

	if (m_pVecEnumDir)
	{
		//clear
		while(!m_pVecEnumDir->empty())
		{
			//delete m_pVecEnumDir->back();
			m_pVecEnumDir->pop_back();
		}
		delete m_pVecEnumDir;
	}

	if (m_pVecEnumPlusDir)
	{
		//clear
		while(!m_pVecEnumPlusDir->empty())
		{
			delete m_pVecEnumPlusDir->back();
			m_pVecEnumPlusDir->pop_back();
		}
		delete m_pVecEnumPlusDir;
	}

	
}

//=================================================================================================================================
std::vector<SYDirInfo*> *SYDirFileManger::GetDrives(void)
{
	/*
	EX:
	SYDirFileManger *g_syDirFileMgr = SYDirFileManger::Instance();
	std::vector<SYDirInfo*> *vecDirInfo = g_syDirFileMgr->GetDrives();
	for (unsigned int i=0; i<vecDirInfo->size(); i++)
	{
	SYDirInfo *info = vecDirInfo->at(i);
	info->Print();
	}
	*/
	WCHAR szBuffer [80];
	WCHAR szDname [80];
	WCHAR szQueryName [80];
	UINT  iResl_type;
	UINT  iResl;
	//UINT  iFreeSpace; 
	ULARGE_INTEGER ulAvailable, ulTotal, ulFree;

	DWORDLONG	m_iCurrent; //64位元長整數

	//clear
	while(!m_pVecDirInfo->empty())
	{
		delete m_pVecDirInfo->back();
		m_pVecDirInfo->pop_back();
	}

	//取得裝置名稱
	iResl = GetLogicalDriveStrings(sizeof(szDname),szDname);

	for(unsigned int u=0;u<iResl;u=u+4)
	{
		//取出陣列中個別的裝置名稱
		szQueryName[0] = szDname[u];
		szQueryName[1] = szDname[u+1];
		szQueryName[2] = szDname[u+2];
		szQueryName[3] = szDname[u+3];

		lstrcpy(szBuffer,szQueryName);
		//檢查裝置型態   
		iResl_type = GetDriveType(szQueryName);
		
		SYDirInfo *info = new SYDirInfo();
		m_pVecDirInfo->push_back(info);
		info->iType = iResl_type;
		wsprintf(info->wszDrive, L"%s", szQueryName);

		switch(iResl_type) {
				  case DRIVE_UNKNOWN:
					  lstrcat(szBuffer,L" 無法偵測的儲存設備");
					 

					  break;
				  case DRIVE_NO_ROOT_DIR:
					  lstrcat(szBuffer,L" 此儲存設備不存在");
					 
					  break;
				  case DRIVE_REMOVABLE:
					  lstrcat(szBuffer,L" 移動式儲存設備(軟碟)");
					 
					  break;
				  case DRIVE_FIXED:

					  lstrcat(szBuffer,L" 固定式儲存設備(硬碟)");
					  
					  break;
				  case DRIVE_REMOTE:
					  lstrcat(szBuffer,L" 遠端的儲存設備(網路磁碟)");

					 
					  break;
				  case DRIVE_CDROM:
					  lstrcat(szBuffer,L" CDROM光碟機");
					  
					  break;
				  case DRIVE_RAMDISK:
					  lstrcat(szBuffer,L" RAM DRIVE");
					 
					  break;
				  default: 
					  lstrcat(szBuffer,L" 偵測儲存設備失敗");

					  break;
		}

		if (flag_Debug)
		{
			OutputDebugString(szBuffer);
			OutputDebugString(L"\n");
		}
		

		if(GetDiskFreeSpaceEx(&szDname[u], &ulAvailable,
			&ulTotal, &ulFree) !=0) {
				//顯示總容量
				m_iCurrent=ulTotal.QuadPart;
				wsprintf(szBuffer,L" 容量為:%dMByte",
					(m_iCurrent /(1024 *1024 )));
				if (flag_Debug) OutputDebugString(szBuffer);
				info->lTotal = (int)(m_iCurrent / (1024 *1024));
				
				//顯示可用容量
				m_iCurrent=ulFree.QuadPart; 
				wsprintf(szBuffer,L" 可用為:%dMByte",
					(m_iCurrent /(1024 *1024 )));			
				if (flag_Debug) OutputDebugString(szBuffer);	
				

				info->lFree = (int)(m_iCurrent / (1024 *1024));

				if (flag_Debug) OutputDebugString(L"\n");
		}
	}

	return m_pVecDirInfo; 
}


//=================================================================================================================================
bool SYDirFileManger::IsExistWithFileName(const WCHAR *pPath)
{
	
	FILE *fp = NULL;
	_wfopen_s(&fp, pPath, L"r");
	if( fp ) {
		// exists
		fclose(fp);
		return true;
	} else {
		// doesnt exist
		return false;
	}
	return false;
}
//=================================================================================================================================
WCHAR *SYDirFileManger::GetWindowsPath(void)
{
	memset(m_szWindowsFolder, 0 , sizeof(m_szWindowsFolder));

	SHGetSpecialFolderPath(NULL, m_szWindowsFolder, CSIDL_WINDOWS, FALSE);
	//OutputDebugString(szWindowsFolder);
	return m_szWindowsFolder;
}
//=================================================================================================================================
WCHAR *SYDirFileManger::GetProgramFilePath(void)
{
	memset(m_szWindowsFolder, 0 , sizeof(m_szWindowsFolder));

	SHGetSpecialFolderPath(NULL, m_szWindowsFolder, CSIDL_PROGRAM_FILES, FALSE);
	//OutputDebugString(szWindowsFolder);
	return m_szWindowsFolder;
}
//=================================================================================================================================
void SYDirFileManger::CopyFrom(WCHAR *pFromPath, WCHAR *pToPath)
{
	CreateDirectory(pToPath, NULL); //for winXP

	SHFILEOPSTRUCT   sfo;
	ZeroMemory(&sfo, sizeof(SHFILEOPSTRUCT));
	sfo.hwnd	= NULL; 
	sfo.wFunc	= FO_COPY;
	sfo.pFrom	= pFromPath;
	sfo.pTo		= pToPath;
	sfo.fFlags=FOF_SILENT|FOF_NOCONFIRMATION|FOF_NOCONFIRMMKDIR; 
	sfo.fAnyOperationsAborted=false; 
	int msg = SHFileOperation(&sfo);
	if (msg)
	{
		OutputDebugString(L"Copy file fault\n");
	}
}


//=================================================================================================================================
std::vector<std::wstring>* SYDirFileManger::EnumDirWithDirPath(const WCHAR *pPath)
{
	/*
	[2012.10.08 create]
	EX
	std::vector<std::wstring>* pVecEnum = g_syDirFileMgr->EnumDirWithDirPath(L"D:\\*.txt");
	for (unsigned int i=0; i<pVecEnum->size(); i++)
	{
	OutputDebugString(pVecEnum->at(i).c_str());
	OutputDebugString(L"\n");
	}
	*/
	//clear
	while(!m_pVecEnumDir->empty())
	{
		//delete m_pVecEnumDir->back();
		m_pVecEnumDir->pop_back();
	}


	WIN32_FIND_DATA wfd;

	HANDLE hFile = FindFirstFile( pPath, &wfd );

	if( INVALID_HANDLE_VALUE == hFile )
	{
		OutputDebugString( L"SYDirFileManger::EnumDirWithDirPathFind First File Failed..." );
		return m_pVecEnumDir;
	}

	//First file
	if( FILE_ATTRIBUTE_HIDDEN != wfd.dwFileAttributes &&
		FILE_ATTRIBUTE_SYSTEM != wfd.dwFileAttributes )
	{
		std::wstring tmp = wfd.cFileName;
		m_pVecEnumDir->push_back(tmp);
	}

	//Next file
	while( 0 != FindNextFile( hFile, &wfd ) )
	{
		if( 0 == _wcsicmp( L".", wfd.cFileName ) || 0 == _wcsicmp( L"..", wfd.cFileName ) )
			continue;

		if( FILE_ATTRIBUTE_HIDDEN != wfd.dwFileAttributes &&
			FILE_ATTRIBUTE_SYSTEM != wfd.dwFileAttributes )
		{
			std::wstring tmp = wfd.cFileName;
			m_pVecEnumDir->push_back(tmp);
		}
	}

	FindClose (hFile);
	
	return m_pVecEnumDir;
}

//=================================================================================================================================
std::vector<SYDirFileInfo*>* SYDirFileManger::EnumDirPlusWithDirPath(const WCHAR *pPath)
{
	/*
	[2013.01.24 create]
	EX
	std::vector<SYDirFileInfo*>* pVecPlusEnum = g_syDirFileMgr->EnumDirPlusWithDirPath(L"E:\\Sample\\SYHttpServer\\src\\all\\debug\\*.*");
	for (unsigned int i=0; i<pVecPlusEnum->size(); i++)
	{
	SYDirFileInfo *pFileInfo = pVecPlusEnum->at(i);
	if (pFileInfo->IsDirectory)
	{
	OutputDebugString(L"[Directory] ");
	}else{
	OutputDebugString(L"[file] ");
	}
	OutputDebugString(pFileInfo->strName.c_str());
	OutputDebugString(L"\n");
	}
	*/
	//clear
	while(!m_pVecEnumPlusDir->empty())
	{
		delete m_pVecEnumPlusDir->back();
		m_pVecEnumPlusDir->pop_back();
	}
	m_pVecEnumDir->empty();

	WIN32_FIND_DATA wfd;

	HANDLE hFile = FindFirstFile( pPath, &wfd );

	if( INVALID_HANDLE_VALUE == hFile )
	{
		OutputDebugString( L"SYDirFileManger::EnumDirWithDirPathFind First File Failed..." );
		return m_pVecEnumPlusDir;
	}

	
	//First file
	if( FILE_ATTRIBUTE_HIDDEN != wfd.dwFileAttributes &&
		FILE_ATTRIBUTE_SYSTEM != wfd.dwFileAttributes )
	{
		std::wstring tmp = wfd.cFileName;

		SYDirFileInfo *pFileInfo = new SYDirFileInfo();
		m_pVecEnumPlusDir->push_back(pFileInfo);
		
		pFileInfo->strName = tmp;
		pFileInfo->IsDirectory = (FILE_ATTRIBUTE_DIRECTORY == wfd.dwFileAttributes);
		
	}

	//Next file
	while( 0 != FindNextFile( hFile, &wfd ) )
	{
		if( 0 == _wcsicmp( L".", wfd.cFileName ) || 0 == _wcsicmp( L"..", wfd.cFileName ) )
			continue;

		if( FILE_ATTRIBUTE_HIDDEN != wfd.dwFileAttributes &&
			FILE_ATTRIBUTE_SYSTEM != wfd.dwFileAttributes )
		{
			std::wstring tmp = wfd.cFileName;

			SYDirFileInfo *pFileInfo = new SYDirFileInfo();
			m_pVecEnumPlusDir->push_back(pFileInfo);

			pFileInfo->strName = tmp;
			pFileInfo->IsDirectory = (FILE_ATTRIBUTE_DIRECTORY == wfd.dwFileAttributes);
		}
	}

	FindClose (hFile);

	return m_pVecEnumPlusDir;
}


//=================================================================================================================================
bool SYDirFileManger::EnumDirPlusWithDirPath(const WCHAR *pPath, std::vector<SYDirFileInfo*>* pVecPlusEnum)
{
	/*
	[2013.02.21 create]
	EX
	std::vector<SYDirFileInfo*> VecPlusEnum;
	bool bResult = g_syDirFileMgr->EnumDirPlusWithDirPath(L"E:\\Sample\\SYHttpServer\\src\\all\\debug\\*.*", &VecPlusEnum);
	for (unsigned int i=0; i<VecPlusEnum.size(); i++)
	{
	SYDirFileInfo *pFileInfo = VecPlusEnum.at(i);
	if (pFileInfo->IsDirectory)
	{
	OutputDebugString(L"[Directory] ");
	}else{
	OutputDebugString(L"[file] ");
	}
	OutputDebugString(pFileInfo->strName.c_str());
	OutputDebugString(L"\n");
	}
	*/
	//clear	
	pVecPlusEnum->empty();

	WIN32_FIND_DATA wfd;

	HANDLE hFile = FindFirstFile( pPath, &wfd );

	if( INVALID_HANDLE_VALUE == hFile )
	{
		OutputDebugString( L"SYDirFileManger::EnumDirWithDirPathFind First File Failed..." );
		return false;
	}


	//First file
	if( FILE_ATTRIBUTE_HIDDEN != wfd.dwFileAttributes &&
		FILE_ATTRIBUTE_SYSTEM != wfd.dwFileAttributes )
	{
		std::wstring tmp = wfd.cFileName;

		SYDirFileInfo *pFileInfo = new SYDirFileInfo();
		pVecPlusEnum->push_back(pFileInfo);

		pFileInfo->strName = tmp;
		pFileInfo->IsDirectory = (FILE_ATTRIBUTE_DIRECTORY == wfd.dwFileAttributes);

	}

	//Next file
	while( 0 != FindNextFile( hFile, &wfd ) )
	{
		if( 0 == _wcsicmp( L".", wfd.cFileName ) || 0 == _wcsicmp( L"..", wfd.cFileName ) )
			continue;

		if( FILE_ATTRIBUTE_HIDDEN != wfd.dwFileAttributes &&
			FILE_ATTRIBUTE_SYSTEM != wfd.dwFileAttributes )
		{
			std::wstring tmp = wfd.cFileName;

			SYDirFileInfo *pFileInfo = new SYDirFileInfo();
			pVecPlusEnum->push_back(pFileInfo);

			pFileInfo->strName = tmp;
			pFileInfo->IsDirectory = (FILE_ATTRIBUTE_DIRECTORY == wfd.dwFileAttributes);
		}
	}

	FindClose (hFile);

	return true;
}