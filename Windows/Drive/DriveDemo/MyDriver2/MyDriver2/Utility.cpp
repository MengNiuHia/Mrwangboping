#include "Utility.h"
#include <malloc.h>
#include <memory.h>

#include <time.h>
#include <objbase.h>
CUtility::CUtility(void)
{	
}

CUtility::~CUtility(void)
{
}

#define LOG_PATH  TEXT("C:\\tmp")
std::string g_strLogFile = "C:\\tmp\\dllhook_log.txt";

static const char* to_b64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
#define CHARS_PER_LINE  64
#define B64DECODE_WHITESPACE   " \f\n\r\t\v"   
const char * b64decode_whitespace = B64DECODE_WHITESPACE;


char* CUtility::b64encode(const unsigned char * chunkdata, int chunksize)
{
	int div = chunksize / 3;
	int rem = chunksize % 3;
	int chars = div*4 + 4;
	int newlines = (chars + CHARS_PER_LINE - 1) / CHARS_PER_LINE;

	const unsigned char* data = chunkdata;
	char* string = (char*) malloc(chars + newlines + 1);

	if (string)
	{
		register char* buf = string;

		chars = 0;

		/*@+charindex@*/
		while (div > 0)
		{
			buf[0] = to_b64[ (data[0] >> 2) & 0x3f];
			buf[1] = to_b64[((data[0] << 4) & 0x30) + ((data[1] >> 4) & 0xf)];
			buf[2] = to_b64[((data[1] << 2) & 0x3c) + ((data[2] >> 6) & 0x3)];
			buf[3] = to_b64[  data[2] & 0x3f];
			data += 3;
			buf += 4;
			div--;
			chars += 4;
			if (chars == CHARS_PER_LINE)
			{
				chars = 0;
				*(buf++) = '\n';
			}
		}

		switch (rem)
		{
		case 2:
			buf[0] = to_b64[ (data[0] >> 2) & 0x3f];
			buf[1] = to_b64[((data[0] << 4) & 0x30) + ((data[1] >> 4) & 0xf)];
			buf[2] = to_b64[ (data[1] << 2) & 0x3c];
			buf[3] = '=';
			buf += 4;
			chars += 4;
			break;
		case 1:
			buf[0] = to_b64[ (data[0] >> 2) & 0x3f];
			buf[1] = to_b64[ (data[0] << 4) & 0x30];
			buf[2] = '=';
			buf[3] = '=';
			buf += 4;
			chars += 4;
			break;
		}
		/*@=charindex@*/
		/* *(buf++) = '\n'; This would result in a buffer overrun */
		*buf = '\0';
	}

	return string;
}


std::wstring CUtility::b64encode_to_wstring(const unsigned char * chunkdata, int chunksize)
{
	char* pszResult = b64encode(chunkdata,chunksize);
	return s2ws(pszResult);
}

/*@-internalglobs -modfilesys @*/
/*@-boundswrite@*/
int CUtility::b64decode (const char * s, void ** datap, int *lenp)
{
	unsigned char b64dec[256];
	const unsigned char *t;
	unsigned char *te;
	int ns, nt;
	unsigned a, b, c, d;

	if (!s)      return 1;

	/* Setup character lookup tables. */
	memset(b64dec, 0x80, sizeof(b64dec));
	for (c = 'A'; c <= 'Z'; c++)
		b64dec[ c ] = 0 + (c - 'A');
	for (c = 'a'; c <= 'z'; c++)
		b64dec[ c ] = 26 + (c - 'a');
	for (c = '0'; c <= '9'; c++)
		b64dec[ c ] = 52 + (c - '0');
	b64dec[(unsigned)'+'] = 62;
	b64dec[(unsigned)'/'] = 63;
	b64dec[(unsigned)'='] = 0;

	/* Mark whitespace characters. */
	if (b64decode_whitespace) {
		const char *e;
		for (e = b64decode_whitespace; *e != '\0'; e++) {
			if (b64dec[ (unsigned)*e ] == 0x80)
				b64dec[ (unsigned)*e ] = 0x81;
		}
	}

	/* Validate input buffer */
	ns = 0;
	for (t =(unsigned char *) s; *t != '\0'; t++) {
		switch (b64dec[ (unsigned)*t ]) {
		case 0x80:      /* invalid chararcter */
			return 3;
			/*@notreached@*/ /*@switchbreak@*/ break;
		case 0x81:      /* white space */
			/*@switchbreak@*/ break;
		default:
			ns++;
			/*@switchbreak@*/ break;
		}
	}

	if (ns & 0x3)       return 2;

	nt = (ns / 4) * 3;
	t = te = (unsigned char *)  malloc(nt + 1);

	while (ns > 0) 
	{
		/* Get next 4 characters, ignoring whitespace. */
		while ((a = b64dec[ (unsigned)*s++ ]) == 0x81)
		{};
		while ((b = b64dec[ (unsigned)*s++ ]) == 0x81)
		{};
		while ((c = b64dec[ (unsigned)*s++ ]) == 0x81)
		{};
		while ((d = b64dec[ (unsigned)*s++ ]) == 0x81)
		{};

		ns -= 4;
		*te++ = (a << 2) | (b >> 4);
		if (s[-2] == '=') break;
		*te++ = (b << 4) | (c >> 2);
		if (s[-1] == '=') break;
		*te++ = (c << 6) | d;
	}

	if (ns != 0) {              /* XXX can't happen, just in case */
		if (t) free((void *)t);
		return 1;
	}
	if (lenp)
		*lenp = (int)(te - t);

	if (datap)
		*datap = (void *)t;
	else
		if (t) free((void *)t);
		else
		{};

	return 0;
}

std::string CUtility::ws2s(const std::wstring &ws)
{
	size_t i;
	std::string curLocale = setlocale(LC_ALL, NULL);
	setlocale(LC_ALL, "chs");
	const wchar_t* _source = ws.c_str();
	size_t _dsize = 2 * ws.size() + 1;
	char* _dest = new char[_dsize];
	memset(_dest, 0x0, _dsize);
	wcstombs_s(&i, _dest, _dsize, _source, _dsize);
	std::string result = _dest;
	delete[] _dest;
	setlocale(LC_ALL, curLocale.c_str());
	return result;
}

std::wstring CUtility::s2ws(const std::string &s)
{
	size_t i;
	std::string curLocale = setlocale(LC_ALL, NULL);
	setlocale(LC_ALL, "chs");
	const char* _source = s.c_str();
	size_t _dsize = s.size() + 1;
	wchar_t* _dest = new wchar_t[_dsize];
	wmemset(_dest, 0x0, _dsize);
	mbstowcs_s(&i, _dest, _dsize, _source, _dsize);
	std::wstring result = _dest;
	delete[] _dest;
	setlocale(LC_ALL, curLocale.c_str());
	return result;
}


std::string CUtility::Int_to_String(int n)
{
	int m = n;
	char s[128];
	char ss[128];
	int i=0,j=0;
	if (n < 0)// 处理负数
	{
		m = 0 - m;
		j = 1;
		ss[0] = '-';
	}
	while (m>0)
	{
		s[i++] = m % 10 + '0';
		m /= 10;
	}
	s[i] = '\0';
	i = i - 1;
	while (i >= 0)
	{
		ss[j++] = s[i--];
	}
	ss[j] = '\0';
	return ss;
}

void CUtility::PrintLog(bool bTextLog,std::string strFunctionName,int nFileline,const TCHAR * lpszFormat,...)
{
	if (!bTextLog)
	{
		return;
	}

	int bufsize = 0;
	va_list argList;
	wchar_t* buf = NULL;

	try
	{
		va_start(argList, lpszFormat);
#pragma warning( push )
#pragma warning( disable : 4996 )
		bufsize = _vsnwprintf(NULL, 0, lpszFormat, argList) + 2;   
#pragma warning( pop )

		buf = new wchar_t[bufsize];   
		if (NULL != buf)
		{
			memset(buf, 0, bufsize*sizeof(wchar_t));
			_vsnwprintf_s(buf, bufsize, _TRUNCATE, lpszFormat, argList);
			va_end(argList);

			FILE* logfile = fopen(g_strLogFile.c_str(), "a+");
			WriteRecord(logfile,strFunctionName,nFileline,buf);
			fclose(logfile);
		}
	}
	catch(...)
	{
	}
	if (NULL != buf) delete[] buf;    


}

void CUtility::WriteRecord(FILE* pFile, std::string cTrackFunctionName, int iTrackFileLine, LPWSTR strInfo)
{
	static time_t  lClock;
	static struct tm stTm;
	std::wstring strFunName = s2ws(cTrackFunctionName);

	time(&lClock);
#if (_MSC_VER >= 1400) 
	{///VS 2005
		localtime_s(&stTm, &lClock);
		if (iTrackFileLine > 0)
		{
			fwprintf(pFile,L"%04d-%02d-%02d %02d:%02d:%02d,(%04d-%04d)[%04d %s]  %s\r\n",
				stTm.tm_year+1900, stTm.tm_mon+1, stTm.tm_mday,
				stTm.tm_hour, stTm.tm_min, stTm.tm_sec ,
				GetCurrentProcessId(),GetCurrentThreadId(),
				iTrackFileLine,
				strFunName.c_str(),
				strInfo);
		}
		else
		{
			fwprintf(pFile,L"%04d-%02d-%02d %02d:%02d:%02d,(%04d-%04d)  %s\r\n",
				stTm.tm_year+1900, stTm.tm_mon+1, stTm.tm_mday,
				stTm.tm_hour, stTm.tm_min, stTm.tm_sec ,
				GetCurrentProcessId(),GetCurrentThreadId(),
				strInfo);
		}
	}
#endif
}

