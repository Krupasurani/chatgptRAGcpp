/*
	Developed by Michael Haephrati
	https://www.securedglobe.net
	https://www.freelancer.com/u/NYCyber

	April 2023

*/
#include "stdafx.h"
#include "AutoChat.h"
#include "utils.h"
wchar_t LOGFILENAME[1024]{ L"Chevrusa.log" };

std::string omitSubstring(const std::string& input, const std::string& substring) 
{
	std::string result = input;
	size_t pos = result.find(substring);

	while (pos != std::string::npos) 
	{
		result.erase(pos, substring.length());
		pos = result.find(substring, pos);
	}

	// Remove excess spaces
	size_t start = 0;
	while (start < result.length()) 
	{
		if (result[start] == ' ' && (start + 1 < result.length() && result[start + 1] == ' ')) {
			result.erase(start, 1);
		}
		else 
		{
			start++;
		}
	}

	return result;
}

CAtlString CurrentDate()
{
	CTime currentDateTime = CTime::GetCurrentTime();
	// Do we need to convert the local time into GMT time?
	//currentDateTime.GetGmtTm(&gmt);
	return currentDateTime.FormatGmt(DATEFORMAT);
}
wstring CurrentTimeForUpdate()
{
	time_t currTime;
	time(&currTime);

	//write in required format
	wchar_t buffer[16];
	std::wcsftime(buffer, 16, L"%Y%m%d%H%M%S", std::localtime(&currTime));
	std::wstring yyyymmddhhmmss(buffer);
	return yyyymmddhhmmss;
}
void setcolor(int textcol, int backcol)
{
	if ((textcol % 16) == (backcol % 16))textcol++;
	textcol %= 16; backcol %= 16;
	unsigned short wAttributes = ((unsigned)backcol << 4) | (unsigned)textcol;
	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdOut, wAttributes);
}

string ObtainValue(string Buffer, string Value)
{
	string URL{ "" };
	if (tail(Value, 1) != "=")
	{
		Value += "=";
	}
	//get resultURL from response
	size_t tid_ind = Buffer.find(Value);
	size_t tid_ind_s = Buffer.substr(tid_ind).find("\"") + 1;
	size_t tid_len = Buffer.substr(tid_ind + tid_ind_s).find("\"");
	URL = Buffer.substr((tid_ind + tid_ind_s), (tid_len));
	return URL;
}

std::string tail(std::string const& source, size_t const length)
{
	if (length >= source.size()) { return source; }
	return source.substr(source.size() - length);
} // tail

std::string ReplaceAll(std::string str, const std::string& from, const std::string& to)
{
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos)
	{
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
	}
	return str;
}




// Function:	WriteLogFile
// Purpose:		Print a message to the log file and (in DEBUG) to the Console
int WriteLogFile(LPCWSTR lpText, ...)
{
	std::wofstream ofs;
	CTime Today = CTime::GetCurrentTime();

	CAtlStringW sMsg;
	va_list ptr;
	va_start(ptr, lpText);
	sMsg.FormatV(lpText, ptr);
	va_end(ptr);
	wprintf(L"%s\n", sMsg.GetString());
	try
	{
		ofs.open(GetExecutableDir() + L"\\"+ LOGFILENAME, std::ios_base::app);
		if (ofs.is_open())
		{
			ofs.imbue(std::locale("en_US.utf8")); // Set UTF-8 locale
			ofs << Today.FormatGmt(L"%d.%m.%Y %H:%M").GetString() << L": " << sMsg.GetString() << L"\n";
			ofs.close(); // Close the file after writing
		}
	}
	catch (const std::exception& e)
	{
		// Log the exception
		CStringA errorMsg;
		errorMsg.Format("Exception occurred while writing to log file: %s\n", e.what());
		OutputDebugStringA(errorMsg);
		return FALSE;
	}

	return TRUE;
}

string convertWstringToString(wstring wstr)
{
	int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
	string result(sizeNeeded, 0);
	WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &result[0], sizeNeeded, NULL, NULL);

	return result;
}

wstring convertStringToWstring(string str)
{
	int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
	wstring result(sizeNeeded, 0);
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &result[0], sizeNeeded);

	return result;
}