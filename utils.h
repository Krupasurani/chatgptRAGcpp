/*
	Developed by Michael Haephrati
	https://www.securedglobe.net
	https://www.freelancer.com/u/NYCyber

	April 2023

	April 2023

*/
#pragma once
#define LOG_COLOR_DARKBLUE 9
#define LOG_COLOR_DARKGREEN 2
#define LOG_COLOR_WHITE 7
#define LOG_COLOR_GREEN 10
#define LOG_COLOR_YELLOW 14 
#define LOG_COLOR_MClientA 13
#define LOG_COLOR_CIAN 11

#define DATEFORMAT L"%Y%m%d%H%M%S"
#define FRIEDLY_DATEFORMAT L"%d-%m-%Y, %H:%M:%S"
using namespace std;
#include <string>
struct WriteThis
{
	const char *readptr;
	size_t sizeleft;
};

extern wchar_t LOGFILENAME[1024];
extern HANDLE hConsole;

std::string omitSubstring(const std::string& input, const std::string& substring);


CAtlString CurrentDate();
std::wstring CurrentTimeForUpdate();

void setcolor(int textcol, int backcol);

string ObtainValue(string Buffer, string Value);
std::string tail(std::string const& source, size_t const length);
std::string ReplaceAll(std::string str, const std::string& from, const std::string& to);
int WriteLogFile(LPCWSTR lpText, ...);
string convertWstringToString(wstring wstr);
wstring convertStringToWstring(string str);