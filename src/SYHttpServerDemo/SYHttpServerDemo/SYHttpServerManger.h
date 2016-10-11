#ifndef _SYHTTPSERVERMANGER_H_
#define _SYHTTPSERVERMANGER_H_




typedef int (WINAPI* SYHttpServer_TestAdd)(int a, int b);
typedef bool (WINAPI* SYHttpServer_InitServerWithPort)(USHORT port, WCHAR *pwszRootPath);
typedef void (WINAPI* SYHttpServer_GetOnlineClient)(void);
typedef void (WINAPI* SYHttpServer_AddShareWithFilePath)(WCHAR *pwszFilePath);
typedef void (WINAPI* SYHttpServer_RemoveShareWithFilePath)(WCHAR *pwszFilePath);


class SYHttpServerManger
{

public:	
	//creat
	static SYHttpServerManger* Instance(void);
	~SYHttpServerManger(void);
	//method
	int TestAdd(int a, int b);
	bool InitServerWithPort(USHORT port, WCHAR *pwszRootPath);
	void GetOnlineClient(void);
	void AddShareWithFilePath(WCHAR *pwszFilePath);
	void RemoveShareWithFilePath(WCHAR *pwszFilePath);
private:
	//method
	SYHttpServerManger(void);	


private:
	//var
	//creat
	static SYHttpServerManger*			m_instance;		
	//dll
	HMODULE  							m_hModule;

	SYHttpServer_TestAdd				m_DLLFuncTestAdd;
	SYHttpServer_InitServerWithPort		m_DLLFuncInitServerWithPort;
	SYHttpServer_GetOnlineClient		m_DLLFuncGetOnlineClient;
	SYHttpServer_AddShareWithFilePath	m_DLLFuncAddShareWithFilePath;
	SYHttpServer_RemoveShareWithFilePath	m_DLLFuncRemoveShareWithFilePath;
	//
	
	
};

#endif