// SYHttpServer.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "SYSocket.h"
#include <string>
#include <fstream>
#include <stdio.h>
#include "SYDirFileManger.h"
#include "Tool.h"
#include <map>
#include <list>
#include <math.h> 

SYTCPSocket *g_pServerSocket = NULL;
void OnSYTCPSocketEvent(SYTCPSocket *sender, SYTCPEvent e);

SYDirFileManger	*g_pDirMgr = NULL;

std::wstring g_strRootPath;
#define SYHTTPSERVER_UPLOAD_FILE_CONTEXT "<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"><title>Upload Files</title></head><body><form method='POST' action='' enctype='multipart/form-data'><input type='file' name='upfile' size='71'><br><input type='file' name='upfile' size='71'><br><input type='submit' value='Upload'></form></body></html>"
#define SYHTTPSERVER_ERROR_530_CONTEXT	"<HTML><HEAD><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"><TITLE>503</TITLE></HEAD><BODY><BR>This Page Is Not Exist</BR></BODY></HTML>"
#define vbCrLf	"\r\n"//Chr(13) Chr(10)
#define SYSERVER_NAME "SYHttpServer v1.00.00"

#define BUFFER_STREAM	(1024*512) /*(1024*16)*/

class SYUploadData
{
public:
	SYUploadData(std::wstring wstrPath, std::string strBoundary, int Content_Length)
	{
		_pszBuffer = NULL;
		_wstrPath = wstrPath;
		_strBoundary = strBoundary;
		_iContent_Length = Content_Length;
		flag_finish = false;
	}
	~SYUploadData()
	{
		free(_pszBuffer);		
	}

	void AddBuffer(char *pData, int Len)
	{
		if (_pszBuffer == NULL)
		{	
			_pszBuffer = (char*)malloc(sizeof(char)*Len);
			memset(_pszBuffer, 0, sizeof(char)*Len);
			memcpy( _pszBuffer, pData, sizeof(char)*Len);		
			_bufferLen = Len;
		}else{
			int size = _bufferLen;
			_pszBuffer = (char*)realloc(_pszBuffer, sizeof(char) * (size+Len));

			memset(_pszBuffer+size, 0, sizeof(char)*Len);
			memcpy(_pszBuffer+size, pData, sizeof(char)*Len);

			_bufferLen+=Len;
		}			
	}

	void RemoveBuffer(int size)
	{
		if (_bufferLen-size == 0){
			free(_pszBuffer);
			_pszBuffer = NULL;
			_bufferLen = 0;
			return;
		}

		//copy
		char *pszTmp = (char*)malloc(sizeof(char*)*(_bufferLen-size));
		memset(pszTmp, 0, sizeof(char)*(_bufferLen-size));
		memcpy(pszTmp, _pszBuffer+size, sizeof(char)*(_bufferLen-size));

		//resize
		_pszBuffer = (char*)realloc(_pszBuffer, sizeof(char) * (_bufferLen-size));

		//recopy
		memset(_pszBuffer, 0, sizeof(char) * (_bufferLen-size));
		memcpy(_pszBuffer, pszTmp, sizeof(char) * (_bufferLen-size));

		free(pszTmp);

		_bufferLen = (_bufferLen-size);
	}

	void Clear()
	{		
		RemoveBuffer(_bufferLen);	
	}


	void Process()
	{
		int iPos_Cur = 0;
		int iPos_End = 0;
		// check 是否有結尾字串，有代表資料都傳完了
		//42 bytes is Boundary ex:------WebKitFormBoundarycAPu1tyRBXulXiNr--
		//2  bytes is chr(13)=0x0D chr(10)=0x0A
		//1  byte  is end char '\0'
		if (_bufferLen > 44)
		{
			char szBoundaryEnd[45];
			memset(szBoundaryEnd, 0, sizeof(szBoundaryEnd));
			memcpy(szBoundaryEnd, _pszBuffer+(_bufferLen-44), sizeof(szBoundaryEnd)-1);
			//OutputDebugStringA(boundKey);
			//OutputDebugStringA("\n==============================================\n");

			if ( (_strBoundary+"--"+vbCrLf).compare(szBoundaryEnd) != 0 )
			{
				return;
			}
			iPos_End = _bufferLen-44;
		}


		while (true)
		{
			// check 是否有Boundary字串，有代表可能有傳進一筆檔案
			//40 bytes is Boundary ex:------WebKitFormBoundarycAPu1tyRBXulXiNr
			//2  bytes is chr(13)=0x0D chr(10)=0x0A
			//1  byte  is end char '\0'
			char szBoundary[43];
			memset(szBoundary, 0, sizeof(szBoundary));
			memcpy(szBoundary, _pszBuffer+iPos_Cur, sizeof(szBoundary)-1);
			if (iPos_Cur == iPos_End)
			{
				break;
			}

			if ((_strBoundary+vbCrLf).compare(szBoundary) == 0)
			{
				iPos_Cur+=42;
				int iPos_FileHeader = iPos_Cur;

				//---[Header]--------------------------------------------------------------------------
				//取得下面兩行字串，會經歷兩個vbCrLf
				//Content-Disposition: form-data; name="upfile"; filename="upfile.cgi"
				//Content-Type: application/octet-stream
				char szCrlf[3];
				int iCount = 2;
				while(true)
				{
					memset(szCrlf, 0, sizeof(szCrlf));
					memcpy(szCrlf, _pszBuffer+iPos_Cur, sizeof(szCrlf)-1);
					if ( strcmp(szCrlf, vbCrLf) == 0 )
					{
						iPos_Cur++;
						iCount--;
						if (iCount == 0)
						{
							iPos_Cur+=3;
							break;
						}//end if
					}//end if
					iPos_Cur++;
				}//end while

				char *pszHeader = (char*)malloc(sizeof(char)*(iPos_Cur-iPos_FileHeader+1));
				memset(pszHeader, 0, sizeof(char)*(iPos_Cur-iPos_FileHeader+1));
				memcpy(pszHeader, _pszBuffer+iPos_FileHeader, iPos_Cur-iPos_FileHeader);
				
				//UTF8 to Unicode
				wchar_t *pUnicode = NULL;
				UTF8ToUnicode(pszHeader, &pUnicode);
				free(pszHeader);	
				
				//Get File Name
				std::wstring wstrFileHeader = pUnicode;				
				size_t Pos1, Pos2;
				Pos1 = wstrFileHeader.find(L"; filename=\"")+12;
				Pos2 = wstrFileHeader.find(L"\"", Pos1);
				std::wstring wstrFilename = wstrFileHeader.substr(Pos1, Pos2 - Pos1);
				free(pUnicode);
				

							
				
				OutputDebugString(wstrFilename.c_str());

				//---[Header End]--------------------------------------------------------------------------
				if (wstrFilename.length() == 0)
				{
					goto Lable_FileDataEnd;
				}
				//---[File Data]---------------------------------------------------------------------------
				int iPos_FileData = iPos_Cur;
				// check 是否有Boundary字串，有代表檔案結束了
				//40 bytes is Boundary ex:------WebKitFormBoundarycAPu1tyRBXulXiNr				
				//1  byte  is end char '\0'
				while (true)
				{
					char szBoundaryKey[41];
					memset(szBoundaryKey, 0, sizeof(szBoundaryKey));
					memcpy(szBoundaryKey, _pszBuffer+iPos_Cur, sizeof(szBoundaryKey)-1);
					if ((_strBoundary).compare(szBoundaryKey) == 0)
					{
						std::ofstream _outfile;
						_outfile.open(getSerialNumFileNameWithPath(_wstrPath+wstrFilename).c_str(), std::ofstream::binary);
						if(_outfile) {
							_outfile.write (_pszBuffer+iPos_FileData, iPos_Cur-iPos_FileData-2); //sub 2 for 0x0D 0x0A	
							_outfile.close();
						}	
						break;	
					}else{
						iPos_Cur++;
					}
				}
				//---[File Data End]---------------------------------------------------------------------------
Lable_FileDataEnd:;
				
			}else{
				iPos_Cur++;
			}			
		}

		flag_finish = true;		
	}

	bool IsFinish()
	{
		return flag_finish;	
	}
private:
	char *_pszBuffer;
	int _bufferLen;
	std::wstring _wstrPath;
	std::string _strBoundary;
	int _iContent_Length;
	bool flag_finish;
};

std::map<int, SYUploadData*> g_mapUploadData; 
std::list<std::wstring> g_listShareFiles;


#ifdef _MANAGED
#pragma managed(push, off)
#endif

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{

	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		g_pServerSocket = new SYTCPSocket();
		g_pServerSocket->OnEvent  = OnSYTCPSocketEvent;
		//g_pVecWlan = new std::vector<SYWlanInfo*>;	
		g_pDirMgr = SYDirFileManger::Instance();
		break;

	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		delete g_pServerSocket;
		delete g_pDirMgr;
		/*
		if (g_pVecWlan)
		{
		//clear
		while(!g_pVecWlan->empty())
		{
		delete g_pVecWlan->back();
		g_pVecWlan->pop_back();
		}
		delete g_pVecWlan;
		}
		*/
		break;
	}


	return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

//======================================================================================================================
int WINAPI TestAdd(int a, int b)
{
	return a+b;	
}
//======================================================================================================================
void RFC2616SendRawDataWithPort(USHORT port, std::wstring StrPath, std::wstring StrRequestPage, int StartPos = 0)
{
	char *szData= NULL;
	const long size = GetFileSize(StrPath);
	bool bFileExist = (size==0?false:true);
	

	SOCKET socket = g_pServerSocket->GetSocketWithPort(port);		

	std::string download = "HTTP/1.1 200 Ok";
	download+= vbCrLf;
	download+= "Content-type: application/octet-stream";
	download+= vbCrLf;

	if (StartPos == 0)
	{
		download+= "Content-range: bytes";
	}else{		
		//"Content-range: bytes " & .Range & "-" & Val(.BytesLength) - 1 & "/" & .BytesLength & vbCrLf
		download+= "Content-range: bytes ";
		download+= int2str(StartPos);
		download+= "-";
		download+= int2str(size-1);
		download+= "/";
		download+= int2str(size);
	}
	
	download+= vbCrLf;
	download+= "Content-length: ";
	download+= int2str(size-StartPos);
	download+= vbCrLf;
	download+= "Server: ";
	download+= SYSERVER_NAME;
	download+= vbCrLf;
	download+= "Content-Disposition: inline;  filename=\"";
	download+= ws2s(StrRequestPage);
	download+= "\""; 
	download+= vbCrLf;
	download+= vbCrLf;
	

	//OutputDebugStringA(download.c_str());

	int iResult = send(socket, (const char *)download.c_str(), (int)download.size(), 0); 

	if (bFileExist)
	{
		int max = ceil((float)(size-StartPos)/(float)BUFFER_STREAM);

		for (int i=0; i<max; i++)
		{			
			long data_len;
			data_len = LoadFile(StrPath, &szData, StartPos+i*BUFFER_STREAM, BUFFER_STREAM);			
			iResult = send(socket, (const char *)szData, data_len, 0); 
			free(szData);
			szData = NULL;
		}

	}else{
		const char *pData = SYHTTPSERVER_ERROR_530_CONTEXT;
		iResult = send(socket, (const char *)pData, (int)strlen(SYHTTPSERVER_ERROR_530_CONTEXT), 0); 
	}


	

	OutputDebugString(L"Server has Resp data\n");
	//closesocket(socket);
	//int d = ntohs(port);
	g_pServerSocket->CloseSocketWithPort(port);	
}

void RFC2616SendPageWithPort(USHORT port, std::wstring StrPath, std::wstring StrRequestPage)
{
	int size;
	char *szData= NULL;

	bool bResult = LoadFile(StrPath, &szData, &size);
	const char *pData;
	if (bResult)
	{
		pData = szData;
	}else{
		pData = SYHTTPSERVER_ERROR_530_CONTEXT;
		size = (int)strlen(SYHTTPSERVER_ERROR_530_CONTEXT);
	}

	SOCKET socket = g_pServerSocket->GetSocketWithPort(port);		

	//OutputDebugStringA(download.c_str());


	int iResult = send(socket, (const char *)pData, size, 0); 

	free(szData);


	OutputDebugString(L"Server has Resp data\n");
	//closesocket(socket);
	//int d = ntohs(port);
	g_pServerSocket->CloseSocketWithPort(port);	
}

void RFC2616SendPartialContentHeaderWithPort(USHORT port, std::wstring StrPath, std::wstring StrRequestPage)
{
	long size = GetFileSize(StrPath);


	std::string ext = ws2s(extractExtensionW(StrRequestPage));

	std::string header = "HTTP/1.1 206 Partial Content";
	header+= vbCrLf;
	header+= "Content-Length: 2";
	header+= vbCrLf;
	header+= "Content-Type: video/";
	header+= ext;
	header+= vbCrLf;
	header+= "Server: ";
	header+= SYSERVER_NAME;
	header+= vbCrLf;
	header+= "Accept-Ranges: bytes";
	header+= vbCrLf;
	header+= "Content-Range: bytes 0-1/";
	header+= long2str(size);
	header+= vbCrLf;	
	header+= "TransferMode.DLNA.ORG: Streaming";
	header+= vbCrLf;
	header+= vbCrLf;
	header+= "\0";
	header+= "\0";

	SOCKET socket = g_pServerSocket->GetSocketWithPort(port);
	int iResult = send(socket, (const char *)header.c_str(), (int)header.size(), 0); 

	OutputDebugString(L"Server has Resp data\n");

	g_pServerSocket->CloseSocketWithPort(port);	
}
void RFC2616SendPartialContentStreamingWithPort(USHORT port, std::wstring StrPath, std::wstring StrRequestPage, int StartPos = 0)
{
	char *szData= NULL;
	const long size = GetFileSize(StrPath);
	bool bFileExist = (size==0?false:true);

	std::string ext = ws2s(extractExtensionW(StrRequestPage));

	std::string header = "HTTP/1.1 206 Partial Content";
	header+= vbCrLf;
	header+= "Content-Length: ";
	header+= long2str(size-StartPos);
	header+= vbCrLf;
	header+= "Content-Type: video/";
	header+= ext;
	header+= vbCrLf;
	header+= "Server: ";
	header+= SYSERVER_NAME;
	header+= vbCrLf;
	header+= "Accept-Ranges: bytes";
	header+= vbCrLf;
	header+= "Content-Range: bytes ";
	header+= long2str(StartPos);	
	header+= "-";
	header+= long2str(size-1);
	header+= "/";
	header+= long2str(size);
	header+= vbCrLf;	
	header+= "TransferMode.DLNA.ORG: Streaming";
	header+= vbCrLf;
	header+= vbCrLf;
	

	SOCKET socket = g_pServerSocket->GetSocketWithPort(port);
	int iResult = send(socket, (const char *)header.c_str(), (int)header.size(), 0); 
	

	if (bFileExist)
	{
		int max = ceil((float)(size-StartPos)/(float)BUFFER_STREAM);

		for (int i=0; i<max; i++)
		{			
			long data_len;
			data_len = LoadFile(StrPath, &szData, StartPos+i*BUFFER_STREAM, BUFFER_STREAM);			
			iResult = send(socket, (const char *)szData, data_len, 0); 
			free(szData);
			szData = NULL;
		}
		

	}else{
		const char *pData = SYHTTPSERVER_ERROR_530_CONTEXT;
		iResult = send(socket, (const char *)pData, (int)strlen(SYHTTPSERVER_ERROR_530_CONTEXT), 0); 
	}	

	//free(szData);
	OutputDebugString(L"Server has Resp data\n");

	g_pServerSocket->CloseSocketWithPort(port);	
}

void Send_CMD_UploadWithPort(USHORT port)
{
	int size;
	const char *pData;

	pData = SYHTTPSERVER_UPLOAD_FILE_CONTEXT;
	size = (int)strlen(SYHTTPSERVER_UPLOAD_FILE_CONTEXT);

	SOCKET socket = g_pServerSocket->GetSocketWithPort(port);	

	int iResult = send(socket, (const char *)pData, size, 0); 

	g_pServerSocket->CloseSocketWithPort(port);	
}

void Send_CMD_ListWithPort(USHORT port, std::wstring strPath, std::wstring strFile)
{	
	//std::vector<SYDirFileInfo*>* pVecPlusEnum;

	WCHAR wszPath[256];
	ZeroMemory(wszPath, sizeof(wszPath));
	bool flag_IsRoot = false;
	// 根目錄ex:http://127.0.0.1:5000/%3Clist%3E    strPath=%3Clist%3E   strFile=%3Clist%3E
	if (strPath == strFile)
	{
		flag_IsRoot = true;
		wsprintf(wszPath, L"%s*.*", g_strRootPath.c_str());				
	}else{				
		wsprintf(wszPath, L"%s%s\\*.*", g_strRootPath.c_str(), strPath.c_str());							
	}
	//pVecPlusEnum = g_pDirMgr->EnumDirPlusWithDirPath(wszPath);	
	std::vector<SYDirFileInfo*> *pVecPlusEnum = new std::vector<SYDirFileInfo*>;
	g_pDirMgr->EnumDirPlusWithDirPath(wszPath, pVecPlusEnum);	

	std::wstring context = L"<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"><title>list</title></head></body>";
	context.append(L"Local Path:<Font Color='#FF0000'>");
	context.append(wszPath);
	context.append(L"</F><BR>");

	if (flag_IsRoot)
	{
		context.append(L"<A href='");
		context.append(L"<share>'>[Share]</A><BR>");
	}
	else
	{
		//[Back]
		context.append(L"<A href='../");
		context.append(L"<list>'>Back</A><BR>");
	}//end if
	

	for (unsigned int i=0; i<pVecPlusEnum->size(); i++)
	{
		SYDirFileInfo *pFileInfo = pVecPlusEnum->at(i);				
		if (pFileInfo->strName == L".")
		{
			continue;
		}

		if (pFileInfo->IsDirectory) 
		{
			context.append(L"<A href='");
			context.append(pFileInfo->strName);
			context.append(L"/<list>");//if is folder
			context.append(L"'>");
			context.append(L"<Font Color='#0000FF'>[");
			context.append(pFileInfo->strName);
			context.append(L"]</F></A><BR>");
		}else{
			context.append(L"<A href='");
			context.append(pFileInfo->strName);					
			context.append(L"'>");
			context.append(L"<Font Color='#0000FF'>");
			context.append(pFileInfo->strName);
			context.append(L"</F></A><BR>");
		}			
	}
	context.append(L"</body></html>");

	//clear	
	while(!pVecPlusEnum->empty())
	{
		delete pVecPlusEnum->back();
		pVecPlusEnum->pop_back();
	}
	delete pVecPlusEnum;


	char *pAniFilename = NULL;
	UnicodeToUTF8(context.c_str(), &pAniFilename);


	std::string header = "HTTP/1.1 200 Ok";
	header+= vbCrLf;
	header+= "Content-type: text/html; charset=utf-8";
	header+= vbCrLf;
	header+= "Content-length: ";
	header+= (int)strlen(pAniFilename);
	header+= vbCrLf;
	header+= "Server: ";
	header+= SYSERVER_NAME;
	header+= vbCrLf;
	header+= "Connection: close";
	header+= vbCrLf;
	header+= vbCrLf;

	

	SOCKET socket = g_pServerSocket->GetSocketWithPort(port);	
	int iResult = send(socket, (const char *)header.c_str(), (int)header.size(), 0); 
	iResult = send(socket, (const char *)pAniFilename, (int)strlen(pAniFilename), 0); 
	g_pServerSocket->CloseSocketWithPort(port);	

	delete pAniFilename;
}


void Send_CMD_ListXMLWithPort(USHORT port, std::wstring strPath, std::wstring strFile, std::wstring strHost)
{	
	
	WCHAR wszPath[256];
	ZeroMemory(wszPath, sizeof(wszPath));
	bool flag_IsRoot = false;

	// 根目錄ex:http://127.0.0.1:5000/%3Clist%3E    strPath=%3ClistXML%3E   strFile=%3ClistXML%3E
	if (strPath == strFile)
	{
		flag_IsRoot = true;
		wsprintf(wszPath, L"%s*.*", g_strRootPath.c_str());				
	}else{				
		wsprintf(wszPath, L"%s%s\\*.*", g_strRootPath.c_str(), strPath.c_str());							
	}
	std::vector<SYDirFileInfo*> *pVecPlusEnum = new std::vector<SYDirFileInfo*>;
	g_pDirMgr->EnumDirPlusWithDirPath(wszPath, pVecPlusEnum);	

	/*
	<list>	 
	  <local_path>C:\\....\...\</local_path>
      <file>
	     <isDirectory>true</isDirectory>
		 <name></name>
		 <url></url>
	  </file>
	</list>
	*/
	std::wstring context = L"<list>\r\n";
	context.append(L"<local_path>");
	context.append(wszPath);
	context.append(L"</local_path>\r\n");

	if (flag_IsRoot)
	{
		context.append(L"<file>\r\n");
		context.append(L"<isDirectory>");
		context.append(L"true");
		context.append(L"</isDirectory>\r\n");
		context.append(L"<name>");
		context.append(L"[Share]");
		context.append(L"</name>\r\n");
		context.append(L"<url>http://");
		context.append(strHost);
		context.append(L"<sharexml>");	
		context.append(L"</url>\r\n");		
		context.append(L"</file>\r\n");
	}

	for (unsigned int i=0; i<pVecPlusEnum->size(); i++)
	{
		SYDirFileInfo *pFileInfo = pVecPlusEnum->at(i);				
		if (pFileInfo->strName == L".")
		{
			continue;
		}

		context.append(L"<file>\r\n");
		context.append(L"<isDirectory>");
		context.append(pFileInfo->IsDirectory?L"true":L"false");
		context.append(L"</isDirectory>\r\n");
		context.append(L"<name>");
		context.append(pFileInfo->strName);
		context.append(L"</name>\r\n");
		context.append(L"<url>http://");
		context.append(strHost);
		context.append(pFileInfo->strName);	
		context.append(L"</url>\r\n");		
		context.append(L"</file>\r\n");
	}
	context.append(L"</list>");

	//clear	
	while(!pVecPlusEnum->empty())
	{
		delete pVecPlusEnum->back();
		pVecPlusEnum->pop_back();
	}
	delete pVecPlusEnum;


	char *pAniFilename = NULL;
	UnicodeToUTF8(context.c_str(), &pAniFilename);


	std::string header = "HTTP/1.1 200 Ok";
	header+= vbCrLf;
	header+= "Content-type: text/html; charset=utf-8";
	header+= vbCrLf;
	header+= "Content-length: ";
	header+= int2str((int)strlen(pAniFilename));
	header+= vbCrLf;
	header+= "Server: ";
	header+= SYSERVER_NAME;
	header+= vbCrLf;
	header+= "Connection: close";
	header+= vbCrLf;
	header+= vbCrLf;



	SOCKET socket = g_pServerSocket->GetSocketWithPort(port);	
	int iResult = send(socket, (const char *)header.c_str(), (int)header.size(), 0); 
	iResult = send(socket, (const char *)pAniFilename, (int)strlen(pAniFilename), 0); 
	g_pServerSocket->CloseSocketWithPort(port);	

	delete pAniFilename;
}

void Send_CMD_ShareWithPort(USHORT port, std::wstring strPath, std::wstring strFile)
{	
	
	std::wstring context = L"<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"><title>list</title></head></body>";
	context.append(L"Local Path:<Font Color='#FF0000'>");
	context.append(L"Share");
	context.append(L"</F><BR>");

	
	//[Back]
	context.append(L"<A href='../");
	context.append(L"<list>'>Back</A><BR>");
	
	std::list<std::wstring>::iterator it;
	for (it=g_listShareFiles.begin(); it!=g_listShareFiles.end(); ++it)
	{
		std::wstring path = (*it);
		std::size_t found = path.find(L":");

		context.append(L"<A href='");
		context.append(path.replace(found, 1, L"") );					
		context.append(L"'>");
		context.append(L"<Font Color='#0000FF'>");
		context.append(path);
		context.append(L"</F></A><BR>");
	}
	context.append(L"</body></html>");


	char *pAniFilename = NULL;
	UnicodeToUTF8(context.c_str(), &pAniFilename);


	std::string header = "HTTP/1.1 200 Ok";
	header+= vbCrLf;
	header+= "Content-type: text/html; charset=utf-8";
	header+= vbCrLf;
	header+= "Content-length: ";
	header+= (int)strlen(pAniFilename);
	header+= vbCrLf;
	header+= "Server: ";
	header+= SYSERVER_NAME;
	header+= vbCrLf;
	header+= "Connection: close";
	header+= vbCrLf;
	header+= vbCrLf;



	SOCKET socket = g_pServerSocket->GetSocketWithPort(port);	
	int iResult = send(socket, (const char *)header.c_str(), (int)header.size(), 0); 
	iResult = send(socket, (const char *)pAniFilename, (int)strlen(pAniFilename), 0); 
	g_pServerSocket->CloseSocketWithPort(port);	

	delete pAniFilename;
}



void Send_CMD_ShareXMLWithPort(USHORT port, std::wstring strHost)
{	

	std::wstring context = L"<list>\r\n";
	context.append(L"<local_path>");
	context.append(L"Share");
	context.append(L"</local_path>\r\n");\


	std::list<std::wstring>::iterator it;
	for (it=g_listShareFiles.begin(); it!=g_listShareFiles.end(); ++it)
	{
		std::wstring path = (*it);
		std::size_t found = path.find(L":");

		context.append(L"<file>\r\n");
		context.append(L"<isDirectory>");
		context.append(L"false");
		context.append(L"</isDirectory>\r\n");
		context.append(L"<name>");
		context.append(path);
		context.append(L"</name>\r\n");
		context.append(L"<url>http://");
		context.append(strHost);
		context.append(path.replace(found, 1, L""));	
		context.append(L"</url>\r\n");		
		context.append(L"</file>\r\n");

	}
	context.append(L"</list>");


	char *pAniFilename = NULL;
	UnicodeToUTF8(context.c_str(), &pAniFilename);


	std::string header = "HTTP/1.1 200 Ok";
	header+= vbCrLf;
	header+= "Content-type: text/html; charset=utf-8";
	header+= vbCrLf;
	header+= "Content-length: ";
	header+= (int)strlen(pAniFilename);
	header+= vbCrLf;
	header+= "Server: ";
	header+= SYSERVER_NAME;
	header+= vbCrLf;
	header+= "Connection: close";
	header+= vbCrLf;
	header+= vbCrLf;



	SOCKET socket = g_pServerSocket->GetSocketWithPort(port);	
	int iResult = send(socket, (const char *)header.c_str(), (int)header.size(), 0); 
	iResult = send(socket, (const char *)pAniFilename, (int)strlen(pAniFilename), 0); 
	g_pServerSocket->CloseSocketWithPort(port);	

	delete pAniFilename;
}
void PreParser(SYTCPSocket *sender, SYTCPEvent e)
{

	std::string StrData = e.szData;
	std::wstring StrRequestPage = L"";
	size_t  Pos1;
	size_t  Pos2;
	if (StrData.substr(0, 3) == "GET")
	{
		//取出要取得的頁面(檔名)
		Pos1 = StrData.find("GET /")+5;
		Pos2 = 0;
		for (unsigned int i=(int)Pos1; i<StrData.length(); i++)
		{			
			if ( StrData.substr(i, 1) == " ")
			{
				Pos2 = i;
				break;
			}
			if ( StrData.substr(i, 1) == "?")
			{
				Pos2 = i;
				break;
			}
		}
	
		StrRequestPage = URLDecodeW( StrData.substr(Pos1, Pos2-Pos1) );
		OutputDebugString(L"FileName:");
		OutputDebugString(StrRequestPage.c_str());
		OutputDebugString(L"\n");

		
		Pos1 = StrData.find("?");	
		if (Pos1 != std::string::npos)
		{
			OutputDebugString(L"Had Param\n");
		}

		//如果沒有填寫讀取首頁
		if (StrRequestPage.length() == 0)
		{	
			if (FileIsExist(g_strRootPath+L"index.htm")) StrRequestPage = L"index.htm";
			if (FileIsExist(g_strRootPath+L"index.html")) StrRequestPage = L"index.html";
		}

		//分離出路徑和檔名
		unsigned int found = (int)StrRequestPage.find_last_of(L"/\\");
		std::wstring strPath = StrRequestPage.substr(0, found);
		std::wstring strFile = StrRequestPage.substr(found+1);

		//取出Host:
		Pos1 = StrData.find("Host: ")+6;
		Pos2 = StrData.find(vbCrLf, Pos1);
		std::wstring StrHost = URLDecodeW( StrData.substr(Pos1, Pos2-Pos1) );
		OutputDebugString(L"StrHost:");
		OutputDebugString(StrHost.c_str());
		OutputDebugString(L"\n");

		if (strPath == strFile)
		{
			StrHost.append(L"/");
		}else{
			StrHost.append(L"/");
			StrHost.append(strPath);
			StrHost.append(L"/");
		}


		//Command <list>
		if (strFile == L"<list>")
		{	
			//Ex:http://127.0.0.1:5000/<list>
			Send_CMD_ListWithPort(e.Port, strPath, strFile);
			return;
		}//end if <list>				
		
		
		//Command <listxml>
		if (strFile == L"<listxml>")
		{	
			//Ex:http://127.0.0.1:5000/<listxml>
			Send_CMD_ListXMLWithPort(e.Port, strPath, strFile, StrHost);
			return;
		}//end if <listxml>


		//Command <share>
		if (strFile == L"<share>")
		{	
			//Ex:http://127.0.0.1:5000/<share>
			Send_CMD_ShareWithPort(e.Port, strPath, strFile);
			return;
		}//end if <share>

		
		//Command <sharexml>
		if (strFile == L"<sharexml>")
		{	
			//Ex:http://127.0.0.1:5000/<sharexml>
			Send_CMD_ShareXMLWithPort(e.Port, StrHost);
			return;
		}//end if <sharexml>


		
		std::wstring fullPath = g_strRootPath+StrRequestPage;

		//for get share files
		if (strPath.find(L"/") == 1) 
		{
			fullPath = strPath.insert(1, L":")+L"/"+strFile;
			StrRequestPage = getPathLastComponentW(fullPath);
		}

		
		//Command <upload>
		if (strFile == L"<upload>")
		{	
			Send_CMD_UploadWithPort(e.Port);
			return;
		}//end if <upload>

		if ( StrRequestPage.find(L"html") != std::string::npos || StrRequestPage.find(L"htm") != std::string::npos)
		{
			RFC2616SendPageWithPort(e.Port, fullPath, StrRequestPage);
			return;
		}


		if (StrData.find("X-Playback-Session-Id:") != std::string::npos)
		{
			if (StrData.find("Range: bytes=0-1\r\n") != std::string::npos)
			{
				RFC2616SendPartialContentHeaderWithPort(e.Port, fullPath, StrRequestPage);
				return;				
			}

			Pos1 = StrData.find("Range: bytes=") + 13;
			Pos2 = StrData.find("-", Pos1);
			int StartPos = atoi(StrData.substr(Pos1, Pos2-Pos1).c_str());

			RFC2616SendPartialContentStreamingWithPort(e.Port, fullPath, StrRequestPage, StartPos);
			return;
		}

		

		


		//[續傳&多Seed]
		if (StrData.find("Range: bytes=") != std::string::npos)
		{
			Pos1 = StrData.find("Range: bytes=") + 13;
			Pos2 = StrData.find("-", Pos1);
			int StartPos = atoi(StrData.substr(Pos1, Pos2-Pos1).c_str());

			RFC2616SendRawDataWithPort(e.Port, fullPath, StrRequestPage, StartPos);
			return;
		}//end if ([續傳&多Seed])


		//單線下載
		RFC2616SendRawDataWithPort(e.Port, fullPath, StrRequestPage);
	}//end if (GET)

	if (StrData.substr(0, 4) == "POST")
	{
		//取出要取得的頁面(檔名)
		Pos1 = StrData.find("POST /")+6;
		Pos2 = 0;
		for (unsigned int i=(int)Pos1; i<StrData.length(); i++)
		{			
			if ( StrData.substr(i, 1) == " ")
			{
				Pos2 = i;
				break;
			}			
		}
		StrRequestPage = URLDecodeW( StrData.substr(Pos1, Pos2-Pos1) );

		OutputDebugString(L"FileName:");
		OutputDebugString(StrRequestPage.c_str());
		OutputDebugString(L"\n");
		//分離出路徑和檔名
		unsigned int found = (int)StrRequestPage.find_last_of(L"/\\");
		std::wstring strPath = StrRequestPage.substr(0, found);
		std::wstring strFile = StrRequestPage.substr(found+1);

		if (strPath.compare(strFile) == 0)
		{
			//根目錄和執行檔同一層
			strPath=L"";
		}else{
			//不和執行檔同一層
			strPath.append(L"\\");
		}

		//[上傳檔案]
		if (StrData.find("boundary=") != std::string::npos)
		{
			
			Pos1 = StrData.find("boundary=") + 9;
			Pos2 = StrData.find(vbCrLf, Pos1);
			std::string strBoundary = std::string("--")+StrData.substr(Pos1, Pos2-Pos1);

			Pos1 = StrData.find("Content-Length: ") + 16;
			Pos2 = StrData.find(vbCrLf, Pos1);
			std::string strContentLen = StrData.substr(Pos1, Pos2-Pos1);
			 

			SYUploadData *data = new SYUploadData(std::wstring(g_strRootPath)+strPath, strBoundary,  atoi(strContentLen.c_str()));
			data->AddBuffer(e.szData, e.iLen);
			data->Process();
			if (data->IsFinish())
			{
				Send_CMD_UploadWithPort(e.Port);				
			}else{

				std::map<int, SYUploadData*>::iterator iter = g_mapUploadData.find(e.Port);
				if(iter != g_mapUploadData.end()) {
					delete iter->second;
					g_mapUploadData.erase(iter);					
				}
				//insert will overwrite , but may be need release befor data
				g_mapUploadData.insert(std::map<int, SYUploadData*>::value_type(e.Port, data));
			}			
			return;
		}//end if ([上傳檔案])
	}//end if (POST)

	std::map<int, SYUploadData*>::iterator iter = g_mapUploadData.find(e.Port);
	if(iter != g_mapUploadData.end())
	{
		iter->second->AddBuffer(e.szData, e.iLen);
		iter->second->Process();
		if (iter->second->IsFinish())
		{
			delete iter->second;
			g_mapUploadData.erase(iter);			
			Send_CMD_UploadWithPort(e.Port);
		}
	}

	
}

//////////////////////////////////////////////////////////////////////////
void OnSYTCPSocketEvent(SYTCPSocket *sender, SYTCPEvent e)
{
	switch (e.Status)
	{
	case SYTCPSOCKET_CLOSE:
		OutputDebugString(L"SYTCPSOCKET_CLOSE\n");
		break;

	case SYTCPSOCKET_RECVDATA:
		OutputDebugString(L"SYTCPSOCKET_RECVDATA\n");
		OutputDebugStringA(e.szData);
		PreParser(sender, e);		
		break;

	case SYTCPSOCKET_CONNECTED:
		OutputDebugString(L"SYTCPSOCKET_CONNECTED\n");
		break;

	case SYTCPSOCKET_CONNECTFAULT:
		OutputDebugString(L"SYTCPSOCKET_CONNECTFAULT\n");
		break;

	case SYTCPSOCKET_DISCONNECT:
		OutputDebugString(L"SYTCPSOCKET_DISCONNECT\n");
		break;

	case SYTCPSOCKET_LISTENED:
		OutputDebugString(L"SYTCPSOCKET_LISTENED\n");
		break;
	}
}
//======================================================================================================================
bool WINAPI InitServerWithPort(USHORT port, WCHAR *pwszRootPath)
{
	g_strRootPath = L"";
	g_strRootPath.append(pwszRootPath);

	g_pServerSocket->LocalPort = port;
	g_pServerSocket->Listen();
	return true;
}
//======================================================================================================================
void WINAPI GetOnlineClient()
{
	g_pServerSocket->GetNowOnlineClient();
}

//======================================================================================================================
void WINAPI AddShareWithFilePath(WCHAR *pwszFilePath) 
{
	bool bExist = false;
	std::wstring filePath = pwszFilePath;

	while (true)
	{
		size_t last = filePath.find_last_of(L"\\");
		if (last == std::string::npos) break;
		filePath.replace(last, 1, L"/");
	}
	

	std::list<std::wstring>::iterator it;
	for (it=g_listShareFiles.begin(); it!=g_listShareFiles.end(); ++it)
	{
		if (_wcsicmp(filePath.c_str(), (*it).c_str()) == 0)
		{
			bExist = true;
			break;
		}
	}

	if (!bExist)
	{
		g_listShareFiles.push_back(filePath);
	}
}

//======================================================================================================================
void WINAPI RemoveShareWithFilePath(WCHAR *pwszFilePath) 
{
	std::list<std::wstring>::iterator it;
	for (it=g_listShareFiles.begin(); it!=g_listShareFiles.end(); ++it)
	{
		if (_wcsicmp(pwszFilePath, (*it).c_str())==0)
		{
			g_listShareFiles.erase(it);
			break;
		}
	}
}
//======================================================================================================================
