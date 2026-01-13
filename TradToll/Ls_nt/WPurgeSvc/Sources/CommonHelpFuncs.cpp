
#include "CommonHelpFuncs.h"


std::string CommonHelpFuncs::string_format(const std::string fmt_str, ...) {
	int final_n, n = ((int)fmt_str.size()) * 2; /* Reserve two times as much as the length of the fmt_str */
	std::unique_ptr<char[]> formatted;
	va_list ap;
	while (1) {
		formatted.reset(new char[n]); /* Wrap the plain char array into the unique_ptr */
		strcpy_s(&formatted[0], n, fmt_str.c_str());
		va_start(ap, fmt_str);
		final_n = vsnprintf(&formatted[0], n, fmt_str.c_str(), ap);
		va_end(ap);
		if (final_n < 0 || final_n >= n)
			n += abs(final_n - n + 1);
		else
			break;
	}
	return std::string(formatted.get());
}

std::string CommonHelpFuncs::removePathEndingBackslashes(std::string sPath)
{
	std::string szRetVal = CommonHelpFuncs::lTrimm(CommonHelpFuncs::rTrimm(sPath));
	std::regex regexRelativePath("([\\\\]+$)");

	std::smatch path_match;

	if (std::regex_search(sPath, path_match, regexRelativePath)) {

		szRetVal = path_match.prefix();
	}

	return szRetVal;
}

std::string CommonHelpFuncs::rTrimm(const std::string sPath)
{
	std::string szRetVal = sPath; 
	szRetVal.erase(szRetVal.find_last_not_of(" \n\r\t") + 1);
	return szRetVal;
}

std::string CommonHelpFuncs::lTrimm(const std::string sPath)
{
	std::string szRetVal = sPath;
	szRetVal.erase(0, szRetVal.find_first_not_of(" \n\r\t"));
	return szRetVal;
}

std::string CommonHelpFuncs::assurePathStartsAndEndsWithBackslashes(std::string sPath)
{
	std::string szRetVal = CommonHelpFuncs::lTrimm(CommonHelpFuncs::rTrimm(sPath));

	if (szRetVal.front() != '\\')
		szRetVal.insert(0,"\\");

	if(szRetVal.back() != '\\')
		szRetVal.push_back('\\');

	return szRetVal;
}

bool CommonHelpFuncs::ExtractImgReferencePathData(std::string szImageReferencePath, std::string &szFileName, std::string &szRelativePath, std::string &szRootPath)
{
	bool bRetVal = false;
	//std::regex regexRelativePath("(\\\\[a-zA-Z0-9 \\._]+\\\\[a-zA-Z0-9 \\._]+\\\\[a-zA-Z0-9 \\._]+\\\\)([a-zA-Z0-9 \\._]+$)");
	//([\\][^\\\/<>:\|\*\?\"]+[\\][^\\\/<>:\|\*\?\"]+[\\][^\\\/<>:\|\*\?\"]+[\\])([^\/<>:\|\*\?\"\\]+)$
	std::regex regexRelativePath("([\\\\][^\\\\\\/<>:\\|\\*\\?\\\"]+[\\\\][^\\\\\\/<>:\\|\\*\\?\\\"]+[\\\\][^\\\\\\/<>:\\|\\*\\?\\\"]+[\\\\])([^\\/<>:\\|\\*\\?\\\"\\\\]+)$");
	std::smatch path_match;

	if (std::regex_search(szImageReferencePath, path_match, regexRelativePath)) {

		szRootPath = path_match.prefix();

		if (path_match.size() > 2)
		{
			szRelativePath = path_match[1];
			szFileName = path_match[2];
			bRetVal = true;
		}
	}


	return bRetVal;

}

bool CommonHelpFuncs::TryExtractInvalidImgReferencePathData(std::string szImageReferencePath, 
	std::string &szFileName, std::string &szRelativePath, std::string &szRootPath)
{
	bool bRetVal = false;
	//std::regex regexRelativePath("([\\\\]{2}|[A-Z][:])(\\\\[a-zA-Z0-9 \\._\\\\]+\\\\)([a-zA-Z0-9 \\._]+$)");
	//^([a-zA-Z]{1}:\\|\\\\)([^\/<>:\|\*\?\"]+\\)([^\/<>:\|\*\?\"]+$)
	std::regex regexRelativePath("^([a-zA-Z]{1}:\\\\|\\\\\\\\|\\\\)([^\\/<>:\\|\\*\\?\\\"]+\\\\)([^\\/<>:\\|\\*\\?\\\"]+$)");
	std::smatch path_match;

	if (std::regex_search(szImageReferencePath, path_match, regexRelativePath)) {

		//szRootPath = path_match.prefix();

		if (path_match.size() > 3)
		{
			szRootPath = std::string(path_match[1]) ;
			szRelativePath = "\\" + std::string(path_match[2]);
			szFileName = path_match[3];
			bRetVal = true;
		}
	}


	return bRetVal;

}

std::string CommonHelpFuncs::ExtractLastFolderNameFromPath(std::string sPath)
{
	std::string strTrimmedStr = CommonHelpFuncs::lTrimm(CommonHelpFuncs::rTrimm(sPath));
	//std::regex regexRelativePath("([\\\\]{2}|[A-Z][:])([\\\\][a-zA-Z0-9 \\._\\\\]+)*[\\\\]([a-zA-Z0-9 \\._]+)[\\\\]?$");
	//^(([a-zA-Z]{1}:[^\/<>:\|\*\?\"]*)|([\\\\][^\/<>:\|\*\?\"]+))[\\]([^\/<>:\|\*\?\"\\]+)[\\]?$
	std::regex regexRelativePath("^(([a-zA-Z]{1}:[^\\/<>:\\|\\*\\?\\\"]*)|([\\\\\\\\][^\\/<>:\\|\\*\\?\\\"]+))[\\\\]([^\\/<>:\\|\\*\\?\\\"\\\\]+)[\\\\]?$");

	std::smatch path_match;
	std::string strRetVal;

	if (std::regex_search(strTrimmedStr, path_match, regexRelativePath)) {
	
		if (path_match.size() > 4)
		{
			strRetVal = path_match[4];
		}
	}

	return strRetVal;
}

bool CommonHelpFuncs::IsValidLocalFolder(std::string sPath)
{
	std::string strTrimmedStr = CommonHelpFuncs::lTrimm(CommonHelpFuncs::rTrimm(sPath));
	//std::regex regexRelativePath("([A-Z][:])([\\\\][a-zA-Z0-9 \\._\\\\]+)*([\\\\][a-zA-Z0-9 \\._]+)[\\\\]?$");
	//^([a-zA-Z]{1}:)([\\][^\/<>:\|\*\?\"]+)+[\\]?$
	std::regex regexRelativePath("^([a-zA-Z]{1}:)([\\\\][^\\/<>:\\|\\*\\?\\\"]+)+[\\\\]?$");

	std::smatch path_match;
	bool bRetVal = false;

	if (std::regex_search(strTrimmedStr, path_match, regexRelativePath)) {

		if (path_match.size() == 3)
		{
			bRetVal = true;
		}
	}

	return bRetVal;
}


char* __stdcall removePathEndingBackslashes(char* pPath, unsigned int dwSize)
{
	std::string szRetVal;
	szRetVal = CommonHelpFuncs::removePathEndingBackslashes(std::string(pPath));

	strcpy_s(pPath, dwSize, szRetVal.c_str());

	return pPath;
}

char* __stdcall Trimm(char* pPath, unsigned int dwSize)
{
	std::string szRetVal = CommonHelpFuncs::lTrimm(CommonHelpFuncs::rTrimm(std::string(pPath)));

	strcpy_s(pPath, dwSize, szRetVal.c_str());

	return pPath;
}


int	__stdcall IsValidLocalFolder(char* pPath, unsigned int dwSize)
{
	if (CommonHelpFuncs::IsValidLocalFolder(std::string(pPath)))
		return 1;
	else
		return 0;
}

