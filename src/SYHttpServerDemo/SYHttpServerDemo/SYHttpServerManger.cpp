#include "stdafx.h"
#include "SYHttpServerManger.h"

SYHttpServerManger* SYHttpServerManger::m_instance = NULL;

SYHttpServerManger* SYHttpServerManger::Instance()
{
	if (m_instance == NULL){
		m_instance = new SYHttpServerManger(); 
	}
	return m_instance;
}

SYHttpServerManger::SYHttpServerManger()
{
	m_hModule					= NULL;
	m_DLLFuncTestAdd			= NULL;
	m_DLLFuncInitServerWithPort = NULL;
	m_DLLFuncGetOnlineClient    = NULL;
	m_DLLFuncAddShareWithFilePath = NULL;
	m_DLLFuncRemoveShareWithFilePath = NULL;
	//-------------------------------------------------------------------------------
	m_hModule = LoadLibrary(L".\\SYHttpServer.dll");
	//-------------------------------------------------------------------------------
	if ( m_hModule != NULL )
	{
		//-------------------------------------------------------------------------------
		m_DLLFuncTestAdd = (SYHttpServer_TestAdd)GetProcAddress( m_hModule, "TestAdd" );
		if (m_DLLFuncTestAdd == NULL)
		{
			OutputDebugString(L"GetProc fault:TestAdd\n");
			::MessageBox(NULL, L"GetProc fault:TestAdd", L"", MB_OK);
			FreeLibrary(m_hModule);
			return;
		}

		//-------------------------------------------------------------------------------		
		m_DLLFuncInitServerWithPort = (SYHttpServer_InitServerWithPort)GetProcAddress( m_hModule, "InitServerWithPort" );
		if (m_DLLFuncInitServerWithPort == NULL)
		{
			OutputDebugString(L"GetProc fault:InitServerWithPort\n");
			::MessageBox(NULL, L"GetProc fault:InitServerWithPort", L"", MB_OK);
			FreeLibrary(m_hModule);
			return;
		}

		//-------------------------------------------------------------------------------
		m_DLLFuncGetOnlineClient = (SYHttpServer_GetOnlineClient)GetProcAddress( m_hModule, "GetOnlineClient" );
		if (m_DLLFuncGetOnlineClient == NULL)
		{
			OutputDebugString(L"GetProc fault:GetOnlineClient\n");
			::MessageBox(NULL, L"GetProc fault:GetOnlineClient", L"", MB_OK);
			FreeLibrary(m_hModule);
			return;
		}

		//-------------------------------------------------------------------------------
		m_DLLFuncAddShareWithFilePath = (SYHttpServer_AddShareWithFilePath)GetProcAddress( m_hModule, "AddShareWithFilePath" );
		if (m_DLLFuncAddShareWithFilePath == NULL)
		{
			OutputDebugString(L"GetProc fault:AddShareWithFilePath\n");
			::MessageBox(NULL, L"GetProc fault:AddShareWithFilePath", L"", MB_OK);
			FreeLibrary(m_hModule);
			return;
		}

		//-------------------------------------------------------------------------------
		m_DLLFuncRemoveShareWithFilePath = (SYHttpServer_RemoveShareWithFilePath)GetProcAddress( m_hModule, "RemoveShareWithFilePath" );
		if (m_DLLFuncRemoveShareWithFilePath == NULL)
		{
			OutputDebugString(L"GetProc fault:RemoveShareWithFilePath\n");
			::MessageBox(NULL, L"GetProc fault:RemoveShareWithFilePath", L"", MB_OK);
			FreeLibrary(m_hModule);
			return;
		}
		
	}else{
		OutputDebugString(L"SYHttpServerManger fault:Can't find SYUSB.dll\n");
	}
}

SYHttpServerManger::~SYHttpServerManger()
{
	FreeLibrary(m_hModule);
}


int SYHttpServerManger::TestAdd(int a, int b)
{
	return m_DLLFuncTestAdd(a, b);
}

bool SYHttpServerManger::InitServerWithPort(USHORT port, WCHAR *pwszRootPath)
{
	return m_DLLFuncInitServerWithPort(port, pwszRootPath);
}

void SYHttpServerManger::GetOnlineClient(void)
{
	m_DLLFuncGetOnlineClient();
}

void SYHttpServerManger::AddShareWithFilePath(WCHAR *pwszFilePath)
{
	m_DLLFuncAddShareWithFilePath(pwszFilePath);
}
void SYHttpServerManger::RemoveShareWithFilePath(WCHAR *pwszFilePath)
{
	m_DLLFuncRemoveShareWithFilePath(pwszFilePath);
}