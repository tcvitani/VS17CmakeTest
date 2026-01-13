#ifndef COMMON_HELP_FUNCS_H
#define COMMON_HELP_FUNCS_H


#ifdef __cplusplus
extern "C" {
#endif 

#include <stdio.h>
#include <tchar.h>
#include <stdarg.h>  
	
char* __stdcall removePathEndingBackslashes(char* pPath, unsigned int dwSize);
char* __stdcall Trimm(char* pPath, unsigned int dwSize);
int	__stdcall IsValidLocalFolder(char* pPath, unsigned int dwSize);

#ifdef __cplusplus
}

#include <memory>    // For std::unique_ptr
#include <string> 
#include <regex>


class CommonHelpFuncs
{
public:
	static bool ExtractImgReferencePathData(std::string szImageReferencePath, 
		std::string &szFileName, std::string &szRelativePath, std::string &szRootPath);
	static bool TryExtractInvalidImgReferencePathData(std::string szImageReferencePath, 
		std::string &szFileName, std::string &szRelativePath, std::string &szRootPath);
	static std::string ExtractLastFolderNameFromPath(std::string sPath);
	static bool IsValidLocalFolder(std::string sPath);

	static std::string string_format(const std::string fmt_str, ...);

	static std::string removePathEndingBackslashes(std::string sPath);
	static std::string assurePathStartsAndEndsWithBackslashes(std::string sPath);
	static std::string rTrimm(const std::string sPath);
	static std::string lTrimm(const std::string sPath);
};
#endif 


#endif