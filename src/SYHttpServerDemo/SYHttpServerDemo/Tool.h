#include <string>
#include <fstream>

//======================================================================================================================
bool FileIsExist(std::wstring filePath)
{
	std::ifstream infile (filePath.c_str(), std::ifstream::binary);
	if(!infile) {
		return false;
	}
	infile.close();
	return true;
}
//======================================================================================================================
std::string LoadFile(std::wstring filePath, int *len)
{
	if (FileIsExist(filePath))
	{
		std::string context = "";
		//open file
		std::fstream infile(filePath.c_str(), std::ios::in | std::ios::binary | std::ios::ate );
		if (infile)
		{
			std::ifstream::pos_type eof = infile.tellg();
			(*len) = eof;
			char *szData = (char*)malloc(sizeof(char)*(*len));
			ZeroMemory(szData, sizeof(char)*(*len));
			infile.seekg (0); 
			infile.read (szData, (*len));			
			infile.close();
			context = szData;
			free(szData);
		}
		return context;
	}
	return "<HTML><HEAD><TITLE>503</TITLE></HEAD><BODY><BR>This Page Is Not Exist</BR></BODY></HTML>";
}

bool LoadFile(std::wstring filePath, char **szData, int *len)
{
	if (FileIsExist(filePath))
	{		
		//open file
		std::fstream infile(filePath.c_str(), std::ios::in | std::ios::binary | std::ios::ate );
		if (infile)
		{
			std::ifstream::pos_type eof = infile.tellg();
			(*len) = eof;
			(*szData) = (char*)malloc(sizeof(char)*(*len));
			ZeroMemory((*szData), sizeof(char)*(*len));
			infile.seekg (0); 
			infile.read ((*szData), (*len));			
			infile.close();
		
			//free(szData);
		}
		return true;
	}
	return false;
}
//======================================================================================================================
BYTE toHex(const BYTE &x)
{
	return x > 9 ? x -10 + 'A': x + '0';
}

BYTE fromHex(const BYTE &x)
{
	return isdigit(x) ? x-'0' : x-'A'+10;
}


char Char2Int(char ch)
{
	if(ch>='0' && ch<='9')return (char)(ch-'0');
	if(ch>='a' && ch<='f')return (char)(ch-'a'+10);
	if(ch>='A' && ch<='F')return (char)(ch-'A'+10);
	return -1;
}

char Str2Bin(char *str)
{
	char tempWord[2];
	char chn;

	tempWord[0] = Char2Int(str[0]);                                //make the B to 11 -- 00001011
	tempWord[1] = Char2Int(str[1]);                                //make the 0 to 0  -- 00000000

	chn = (tempWord[0] << 4) | tempWord[1];                //to change the BO to 10110000

	return chn;
}


int Transfer(const char ch)
{
	switch (ch)
	{
	case '0':
		return 0;
		break;
	case '1':
		return 1;
		break;
	case '2':
		return 2;
		break;
	case '3':
		return 3;
		break;
	case '4':
		return 4;
		break;
	case '5':
		return 5;
		break;
	case '6':
		return 6;
		break;
	case '7':
		return 7;
		break;
	case '8':
		return 8;
		break;
	case '9':
		return 9;
		break;
	case 'A':
		return 10;
		break;
	case 'B':
		return 11;
		break;
	case 'C':
		return 12;
		break;
	case 'D':
		return 13;
		break;
	case 'E':
		return 14;
		break;
	case 'F':
		return 15;
		break;
	}
	return -1;
}
BYTE StrToHex(const char *str)
{
	BYTE iResult = Transfer(str[0])*16;
	iResult += Transfer(str[1]);
	return iResult;
}
std::string URLDecode(std::string &sIn)
{

	std::string strResult = "";
	
	for (unsigned int i=0; i<sIn.length(); i++)
	{
		if (sIn.substr(i,1) == "%")
		{
			bool bMatch = false;
			std::string tag = sIn.substr(i,3);
			if (tag == "%20")
			{					
				strResult.append(" ");
				bMatch = true;
			}
			if (tag == "%21")
			{					
				strResult.append("!");
				bMatch = true;
			}
			if (tag == "%22")
			{					
				strResult.append("""");
				bMatch = true;
			}
			if (tag == "%23")
			{					
				strResult.append("#");
				bMatch = true;
			}
			if (tag == "%24")
			{					
				strResult.append("$");
				bMatch = true;
			}
			if (tag == "%25")
			{					
				strResult.append("%");
				bMatch = true;
			}
			if (tag == "%26")
			{					
				strResult.append("&");
				bMatch = true;
			}
			if (tag == "%27")
			{					
				strResult.append("'");
				bMatch = true;
			}
			if (tag == "%28")
			{					
				strResult.append("(");
				bMatch = true;
			}
			if (tag == "%29")
			{					
				strResult.append(")");
				bMatch = true;
			}
			if (tag == "%2A")
			{					
				strResult.append("*");
				bMatch = true;
			}
			if (tag == "%2B")
			{					
				strResult.append("+");
				bMatch = true;
			}
			if (tag == "%2C")
			{					
				strResult.append(",");
				bMatch = true;
			}
			if (tag == "%2D")
			{					
				strResult.append("-");
				bMatch = true;
			}
			if (tag == "%2E")
			{					
				strResult.append(".");
				bMatch = true;
			}
			if (tag == "%2F")
			{					
				strResult.append("/");
				bMatch = true;
			}

				//case "%30":
				//	strResult.append("%0");
				//	break;

				//case "%31":
				//	strResult.append("%1");
				//	break;

				//case "%32":
				//	strResult.append("%2");
				//	break;

				//case "%33":
				//	strResult.append("%3");
				//	break;

				//case "%34":
				//	strResult.append("%4");
				//	break;

				//case "%35":
				//	strResult.append("%5");
				//	break;

				//case "%36":
				//	strResult.append("%6");
				//	break;

				//case "%37":
				//	strResult.append("%7");
				//	break;

				//case "%38":
				//	strResult.append("%8");
				//	break;

				//case "%39":
				//	strResult.append("%9");
				//	break;
			if (tag == "%3A")
			{					
				strResult.append(":");
				bMatch = true;
			}
			if (tag == "%3B")
			{					
				strResult.append(";");
				bMatch = true;
			}
			if (tag == "%3C")
			{					
				strResult.append("<");
				bMatch = true;
			}
			if (tag == "%3D")
			{					
				strResult.append("=");
				bMatch = true;
			}
			if (tag == "%3E")
			{					
				strResult.append(">");
				bMatch = true;
			}
			if (tag == "%3F")
			{					
				strResult.append("?");
				bMatch = true;
			}
			if (tag == "%40")
			{					
				strResult.append("@");
				bMatch = true;
			}
			if (tag == "%5B")
			{					
				strResult.append("[");
				bMatch = true;
			}
			if (tag == "%5C")
			{					
				strResult.append("\\");
				bMatch = true;
			}
			if (tag == "%5D")
			{					
				strResult.append("]");
				bMatch = true;
			}
			if (tag == "%5E")
			{					
				strResult.append("^");
				bMatch = true;
			}
			if (tag == "%5F")
			{					
				strResult.append("_");
				bMatch = true;
			}
			if (tag == "%60")
			{					
				strResult.append("`");
				bMatch = true;
			}
			if (tag == "%7B")
			{					
				strResult.append("{");
				bMatch = true;
			}
			if (tag == "%7C")
			{					
				strResult.append("|");
				bMatch = true;
			}
			if (tag == "%7D")
			{					
				strResult.append("}");
				bMatch = true;
			}
			if (tag == "%7E")
			{					
				strResult.append("~");
				bMatch = true;
			}	
			
			if (!bMatch)
			{
					//いゅ				
					char UTF_8[3], szBig_5[3];
					ZeroMemory(UTF_8, sizeof(UTF_8));
					ZeroMemory(szBig_5, sizeof(szBig_5));
					UTF_8[0] = StrToHex(sIn.substr(i+1, 2).c_str());
					UTF_8[1] = StrToHex(sIn.substr(i+4, 2).c_str());
					UTF_8[2] = StrToHex(sIn.substr(i+7, 2).c_str());				

					WCHAR wszUni[1]; //Unicode絪絏才
					MultiByteToWideChar(CP_UTF8, 0, UTF_8, 3, wszUni, 2);//UTF-8锣Unicode
					//安砞块UTF_8常琌3byte(絋﹚琌いゅ)
					WideCharToMultiByte(950, 0, wszUni, -1, szBig_5, 2, NULL, NULL);//Unicode锣Big5				
					strResult.append(szBig_5);					
					i = i + 6;
			}			
			i = i + 2;
		}else{
			//璣ゅ&计
			strResult.append(sIn.substr(i, 1));				
		}//end if ("%")	
	}//end for		
		
	return strResult;

}

/**************************
* FUNCTION:WCharTochar
* DESCRIPTION:WCharTochar
* PARAMETERS:none
* RETURNED:none
* CREATED:
* BY:sawyer
* COMMENTS:2011.09.29
**************************/
////////////////////////////////////////////////////////////////
//	char *pAniFilename = NULL;
//	SYTool::Instance()->WCharTochar(appPath.c_str(), &pAniFilename);
//  delete pAniFilename;
void WCharTochar(const wchar_t *source, char **dest)
{
	*dest = (char *)new char[2 * wcslen(source)+1] ;
	memset(*dest , 0 , 2 * wcslen(source)+1 );

	int   nLen   =   (int)wcslen(source)+1;
	WideCharToMultiByte(CP_ACP,   0,   source,   nLen,   *dest,   2*nLen,   NULL,   NULL);
	//OutputDebugStringA((*dest));

}
/**************************
* FUNCTION:CharTowchar
* DESCRIPTION:CharTowchar
* PARAMETERS:none
* RETURNED:none
* CREATED:
* BY:sawyer
* COMMENTS:2012.10.05
**************************/
////////////////////////////////////////////////////////////////
//	wchar_t *pUniFilename = NULL;
//	SYTool::Instance()->CharTowchar("test", &pUniFilename);
//  delete pUniFilename;
void CharTowchar(const char *source, wchar_t **dest)
{
	// ANSI(char) to Unicode(WCHAR)

	int nIndex = MultiByteToWideChar(CP_ACP, 0, source, -1, NULL, 0);
	*dest = new wchar_t[nIndex + 1];
	MultiByteToWideChar(CP_ACP, 0, source, -1, *dest, nIndex);
}
//======================================================================================================================
std::string ws2s(const std::wstring& ws)
{
	/*
	std::string curLocale = setlocale(LC_ALL, NULL);        // curLocale = "C";
	setlocale(LC_ALL, "chs");
	const wchar_t* _Source = ws.c_str();
	size_t _Dsize = 2 * ws.size() + 1;
	char *_Dest = new char[_Dsize];
	memset(_Dest,0,_Dsize);
	wcstombs(_Dest,_Source,_Dsize);

	std::string result = _Dest;
	delete []_Dest;
	setlocale(LC_ALL, curLocale.c_str());
	return result;
	*/

	std::string result = "";
	char *pAniFilename = NULL;
	WCharTochar(ws.c_str(), &pAniFilename);
	result.append(pAniFilename);
	delete pAniFilename;
	return result;
}

//======================================================================================================================
std::wstring s2ws(const std::string& s)
{
	/*
	setlocale(LC_ALL, "chs"); 
	const char* _Source = s.c_str();
	size_t _Dsize = s.size() + 1;
	wchar_t *_Dest = new wchar_t[_Dsize];
	wmemset(_Dest, 0, _Dsize);
	mbstowcs(_Dest,_Source,_Dsize);
	std::wstring result = _Dest;
	delete []_Dest;
	setlocale(LC_ALL, "C");
	return result;
	*/
	std::wstring result = L"";
	wchar_t *pUniFilename = NULL;
	CharTowchar(s.c_str(), &pUniFilename);
	result.append(pUniFilename);
	delete pUniFilename;
	return result;
}
