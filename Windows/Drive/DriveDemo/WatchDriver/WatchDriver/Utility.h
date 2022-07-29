#pragma once
#include "Windows.h"
#include <string>
#include <map>

#define TEXT_LOG
#ifdef TEXT_LOG
#define ON_OFF true
#else
#define ON_OFF false
#endif

#define PRINT_INFO(...) CUtility::PrintLog(ON_OFF,__FUNCTION__,__LINE__,__VA_ARGS__);

class CUtility
{
public:
	CUtility(void);
	~CUtility(void);


	CRITICAL_SECTION m_cs;
	static void InitLog();

	static char* b64encode(const unsigned char * chunkdata, int chunksize);
	static int b64decode (const char * s, void ** datap, int *lenp);
	static std::wstring b64encode_to_wstring(const unsigned char * chunkdata, int chunksize);
	static std::string Int_to_String(int n);
	static std::string ws2s(const std::wstring &ws);
	static std::wstring s2ws(const std::string &s);

	static void TextLog(std::string key,std::string logMessage);
	static void PrintLog(bool bTextLog,std::string strFunctionName,int nFileline,const TCHAR * lpszFormat,...);//wangbp


	///////////////////////////////////////////////////////////////////////////
	/// @fn static void WriteRecord(FILE* pFile, const LPWSTR strInfo, char* cTrackFileName,const char* cTrackFunctionName,int iTrackFileLine)
	/// @brief 向文件中写入记录
	/// @param FILE* pFile 指针
	/// @param const LPWSTR strInfo 需要记录的字符串信息
	/// @param const char* cTrackFunctionName 记录函数名
	/// @param int iTrackFileLine 记录文件行数
	/// @return void
	/// @retval 
	/// @note
	///////////////////////////////////////////////////////////////////////////
	static void WriteRecord(FILE* pFile, std::string cTrackFunctionName, int iTrackFileLine, LPWSTR strInfo);

};

