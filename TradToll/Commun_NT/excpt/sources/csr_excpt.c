#include <windows.h>
#include <stdio.h>
#include <dbghelp.h>
#include <psapi.h>

#define CSR_EXCPT_DEF
#include <csr_excpt.h>
#undef CSR_EXCPT_DEF
#include <excpt_loc.h>

#include <memclass.h>

#define MAX_DUMP_SIZE 20000

void* gpvExcptHook = EXPT_MODE_EXITPROCESS;

PRIVATE VOID GetExceptFilePath(char* path, long size);
PRIVATE VOID GetExceptFileMaxSize(DWORD* p_size);

// ------------------------------------------------------------------------

EXPORT void WINAPI ExcptSetDefaultThreadMode(void* pvMode)
{
	gpvExcptHook = pvMode;
}

// ------------------------------------------------------------------------

EXPORT void* WINAPI ExcptGetDefaultThreadMode()
{
	return gpvExcptHook;
}

// ------------------------------------------------------------------------

EXPORT HANDLE WINAPI ExcptCreateThread(SECURITY_ATTRIBUTES* psThreadAttributes, DWORD dwStackSize, LPTHREAD_START_ROUTINE pfStartAddress, void* pvParameter, DWORD dwCreationFlags, DWORD* pdwThreadId, char* szThreadName)
{
	return ExcptCreateThreadWithHandler(psThreadAttributes, dwStackSize, pfStartAddress, pvParameter, dwCreationFlags, pdwThreadId, szThreadName, EXPT_MODE_DEFAULT);
}

// ------------------------------------------------------------------------

EXPORT HANDLE WINAPI ExcptCreateThreadWithHandler(
	SECURITY_ATTRIBUTES* psThreadAttributes,
	DWORD dwStackSize,
	LPTHREAD_START_ROUTINE pfStartAddress,
	void* pvParameter,
	DWORD dwCreationFlags,
	DWORD* pdwThreadId,
	char* szThreadName,
	void* pvHandler)
{
	DWORD dwErr = NO_ERROR;
	EXCPT_THREAD* psThread;
	HANDLE hThread = NULL;

	__try
	{
		__try
		{
			psThread = HeapAlloc(GetProcessHeap(), 0, sizeof(*psThread));
			if (psThread == NULL)
			{
				dwErr = ERROR_NOT_ENOUGH_MEMORY;
				__leave;
			}

			psThread->pfStartAddress = pfStartAddress;
			psThread->pvParameter = pvParameter;
			psThread->pvExcptHook = pvHandler;
			strncpy_s(psThread->szName, MAX_PATH, szThreadName, sizeof(psThread->szName));
			psThread->szName[sizeof(psThread->szName) - 1] = '\0';

			hThread = CreateThread(psThreadAttributes, dwStackSize, ExcptThreadWrapper, (void*)psThread, dwCreationFlags, pdwThreadId);
			if (hThread == NULL)
			{
				dwErr = GetLastError();
				__leave;
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			dwErr = ERROR_INVALID_DATA;
		}
	}
	__finally
	{
		if (dwErr != NO_ERROR)
		{
			if (hThread != NULL)
			{
				TerminateThread(hThread, 0xFFFFFFFF);
				CloseHandle(hThread);
				hThread = NULL;
			}

			if (psThread != NULL)
				HeapFree(GetProcessHeap(), 0, psThread);
		}

		SetLastError(dwErr);
	}

	return hThread;
}

// ------------------------------------------------------------------------

EXPORT void WINAPI ExcptLog(EXCEPTION_POINTERS* psExcptPrt, char* pcContext)
{
	DWORD dwSize;
	DWORD dwMaxSize = 0;
	HANDLE hFile;
	char szPathFile[MAX_PATH] = { 0 };
	char szPathFileOld[MAX_PATH + 1] = { 0 };
	char szText[MAX_DUMP_SIZE + 1] = { 0 };

	dwSize = sizeof(szText);
	ExcptDump(psExcptPrt, pcContext, szText, &dwSize);

	GetExceptFilePath(szPathFile, _countof(szPathFile));
	GetExceptFileMaxSize(&dwMaxSize);

	hFile = CreateFile(szPathFile, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return;

	if (GetFileSize(hFile, NULL) > dwMaxSize)
	{
		CloseHandle(hFile);

		sprintf_s(szPathFileOld, _countof(szPathFileOld), "%s.OLD", szPathFile);

		MoveFileEx(szPathFile, szPathFileOld, MOVEFILE_REPLACE_EXISTING);
		hFile = CreateFile(szPathFile, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
			return;
	}

	SetFilePointer(hFile, 0, 0, FILE_END);

	WriteFile(hFile, szText, (DWORD)strlen(szText), &dwSize, NULL);

	FlushFileBuffers(hFile);
	CloseHandle(hFile);
}

// ------------------------------------------------------------------------

EXPORT char* WINAPI ExcptDump(EXCEPTION_POINTERS* psExcptPrt, char* pcContext, char* pcString, DWORD* pdwStringSize)
{
	HANDLE hProcess = GetCurrentProcess();
	HANDLE hThread = GetCurrentThread();

	pcString = ExcptPrint(pcString, pdwStringSize, "EXCEPTION CONTEXT [%s]\n", pcContext == NULL ? "#ERR(CONTEXT)#" : pcContext);
	pcString = ExcptDumpTime(pcString, pdwStringSize);
	pcString = ExcptDumpException(psExcptPrt->ExceptionRecord, pcString, pdwStringSize);
	pcString = ExcptDumpStack(hProcess, hThread, psExcptPrt->ContextRecord, pcString, pdwStringSize);
	pcString = ExcptDumpRegisters(psExcptPrt->ContextRecord, pcString, pdwStringSize);
	pcString = ExcptDumpModules(hProcess, pcString, pdwStringSize);
	pcString = ExcptPrint(pcString, pdwStringSize, "---\n\n");

	return pcString;
}

// ------------------------------------------------------------------------

PROTECTED char* WINAPI ExcptDumpTime(char* pcString, DWORD* pdwStringSize)
{
	SYSTEMTIME sTime;
	ULONGLONG ullPerf;
	ULONGLONG ullFreq;
	ULONGLONG ullMS;
	DWORD dwTick;

	if (!QueryPerformanceCounter((LARGE_INTEGER*)&ullPerf))
		ullPerf = (ULONGLONG)0;
	if (!QueryPerformanceFrequency((LARGE_INTEGER*)&ullFreq))
		ullFreq = (ULONGLONG)1;
	dwTick = GetTickCount();
	GetLocalTime(&sTime);

	ullMS = (ullPerf * (ULONGLONG)1000) / ullFreq;

	pcString = ExcptPrint(pcString, pdwStringSize,
		"TIMESTAMP   :\n"
		"  Date      = %04u/%02u/%02u\n"
		"  Time      = %02u:%02u:%02u.%03u\n"
		"  Tick      = %u ms\n"
		"  HPTick    = %I64u ms ( %I64u / %I64u Hz)\n",
		sTime.wYear, sTime.wMonth, sTime.wDay,
		sTime.wHour, sTime.wMinute, sTime.wSecond, sTime.wMilliseconds,
		dwTick,
		ullMS, ullPerf, ullFreq
		);

	return pcString;
}

// ------------------------------------------------------------------------

PROTECTED char* WINAPI ExcptDumpException(EXCEPTION_RECORD* psExcpt, char* pcString, DWORD* pdwStringSize)
{
	DWORD dwSection;
	PVOID pvOffset;
	BOOL bBoost1;
	BOOL bBoost2;
	char szFaultingModule[MAX_PATH];
	char szTemp[MAX_PATH];

	if (!GetModuleFileName(GetModuleHandle(NULL), szTemp, sizeof(szTemp)))
		strcpy_s(szTemp, MAX_PATH, "#ERR(GETMODULEFILENAME)#");

	bBoost1 = GetThreadPriorityBoost(GetCurrentThread(), &bBoost2);

	ExcptGetLogicalAddress(psExcpt->ExceptionAddress, szFaultingModule, sizeof(szFaultingModule), &dwSection, &pvOffset);

	pcString = ExcptPrint(pcString, pdwStringSize,
		"EXCEPTION   :\n"
		"  Code      = 0x%08X (%s)\n"
		"  Adress    = %08p %02X:%08X %s\n"
		"  Command   = %s\n"
		"  Module    = %s\n"
		"  Process   = ID=0x%08X (%u) / %s\n"
		"  Thread    = ID=0x%08X (%u) / %s / BOOST=%s (%s)\n",
		psExcpt->ExceptionCode, ExcptGetExceptionString(psExcpt->ExceptionCode),
		psExcpt->ExceptionAddress, dwSection, pvOffset, szFaultingModule,
		GetCommandLine(),
		szTemp,
		GetCurrentProcessId(), GetCurrentProcessId(),
		ExcptGetPriorityClassString(GetPriorityClass(GetCurrentProcess())),
		GetCurrentThreadId(), GetCurrentThreadId(),
		ExcptGetPriorityString(GetThreadPriority(GetCurrentThread())),
		bBoost1 ? "TRUE" : "FALSE", bBoost2 ? "ACTIVE" : "INACTIVE");

	return pcString;
}

// ------------------------------------------------------------------------

PROTECTED char* WINAPI ExcptDumpModules(HANDLE hProcess, char* pcString, DWORD* pdwStringSize)
{
	DWORD dwCount;
	DWORD dwIndex;
	BOOL bResult;
	HMODULE thModules[256];
	char szTemp[MAX_PATH];

	pcString = ExcptPrint(pcString, pdwStringSize,
		"MODULES     :\n");
	bResult = EnumProcessModules(hProcess, thModules, sizeof(thModules), &dwCount);
	dwCount = dwCount / sizeof(thModules[0]);
	if ((!bResult) || (dwCount == 0))
	{
		pcString = ExcptPrint(pcString, pdwStringSize, "  #ERR(ENUMPROCESSMODULES)#\n");
	}
	else
	{
		for (dwIndex = 0; dwIndex < dwCount; dwIndex++)
		{
			bResult = GetModuleFileName(thModules[dwIndex], szTemp, sizeof(szTemp));
			if (!bResult)
				strcpy_s(szTemp, MAX_PATH, "#ERR(GETMODULEFILENAME)#");

			pcString = ExcptPrint(pcString, pdwStringSize, "  %s\n", szTemp);
		}
	}

	return pcString;
}

// ------------------------------------------------------------------------

PROTECTED char* WINAPI ExcptDumpRegisters(CONTEXT* psCtx, char* pcString, DWORD* pdwStringSize)
{
	pcString = ExcptPrint(pcString, pdwStringSize,
		"REGISTERS   :\n"
#ifdef _M_AMD64
		"  EAX=%08p EBX=%08p ECX=%08p EDX=%08p ESI=%08p EDI=%08p\n"
		"  EIP=%08p ESP=%08p EBP=%08p\n"
		"  CS=%04X SS=%04X DS=%04X ES=%04X FS=%04X GS=%04X\n"
		"  EFL=%08X\n",
		psCtx->Rax, psCtx->Rbx, psCtx->Rcx, psCtx->Rdx, psCtx->Rsi, psCtx->Rdi,
		psCtx->Rip, psCtx->Rsp, psCtx->Rbp,
#else
		"  EAX=%08X EBX=%08X ECX=%08X EDX=%08X ESI=%08X EDI=%08X\n"
		"  EIP=%08X ESP=%08X EBP=%08X\n"
		"  CS=%04X SS=%04X DS=%04X ES=%04X FS=%04X GS=%04X\n"
		"  EFL=%08X\n",
		psCtx->Eax, psCtx->Ebx, psCtx->Ecx, psCtx->Edx, psCtx->Esi, psCtx->Edi,
		psCtx->Eip, psCtx->Esp, psCtx->Ebp,
#endif
		psCtx->SegCs, psCtx->SegSs, psCtx->SegDs, psCtx->SegEs, psCtx->SegFs, psCtx->SegGs,
		psCtx->EFlags);

	return pcString;
}

// ------------------------------------------------------------------------

PROTECTED char* WINAPI ExcptDumpStack(HANDLE hProcess, HANDLE hThread, CONTEXT* psCtx, char* pcString, DWORD* pdwStringSize)
{
	char szSearch[MAX_DUMP_SIZE + 1];

	pcString = ExcptPrint(pcString, pdwStringSize, "STACK CALL:\n" "  Address           Frame             Logical address (module)\n");

	if (!SymInitialize(hProcess, ExcptBuildSearchPath(szSearch, sizeof(szSearch)), TRUE))
	{
		pcString = ExcptRawDumpStack(hProcess, hThread, psCtx, pcString, pdwStringSize);
	}
	else
	{
		pcString = ExcptDetailedDumpStack(hProcess, hThread, psCtx, pcString, pdwStringSize);
		SymCleanup(GetCurrentProcess());
	}

	return pcString;
}

// ------------------------------------------------------------------------

PROTECTED char* WINAPI ExcptRawDumpStack(HANDLE hProcess, HANDLE hThread, CONTEXT* psCtx, char* pcString, DWORD* pdwStringSize)
{
	DWORD64 dwEip;

	DWORD* pdwFrame;
	DWORD* pdwPrevFrame;
	char szModule[MAX_PATH];
	DWORD dwSection;
	PVOID pvOffset;
#ifdef _M_AMD64
	dwEip = psCtx->Rip;
	pdwFrame = (PDWORD)psCtx->Rbp;
#else
	dwEip = psCtx->Eip;
	pdwFrame = (PDWORD)psCtx->Ebp;
#endif

	do
	{
		ExcptGetLogicalAddress((void*)dwEip, szModule, sizeof(szModule), &dwSection, &pvOffset);

		pcString = ExcptPrint(pcString, pdwStringSize, "  %08p  %08p  %04X:%08p (%s)\n", dwEip, pdwFrame, dwSection, pvOffset, szModule);

		dwEip = pdwFrame[1];
		pdwPrevFrame = pdwFrame;
		pdwFrame = (DWORD*)pdwFrame[0];

		if ((DWORD)pdwFrame & 3)
			break;

		if (pdwFrame <= pdwPrevFrame)
			break;

		if (IsBadWritePtr(pdwFrame, sizeof(void*) * 2))
			break;
	} while (TRUE);

	return pcString;
}

// ------------------------------------------------------------------------

PROTECTED char* WINAPI ExcptDetailedDumpStack(HANDLE hProcess, HANDLE hThread, CONTEXT* psCtx, char* pcString, DWORD* pdwStringSize)
{
	STACKFRAME sStack = { 0 };
	BYTE tbSymBuf[sizeof(IMAGEHLP_SYMBOL) + 512];
	IMAGEHLP_SYMBOL* psSymbol;

#if defined(_WIN64)
	DWORD64 dwSymDisp;
#else
	DWORD dwSymDisp;
#endif

	BOOL bResult;
	char szModule[MAX_PATH];
	char szUndecor[512];
	DWORD dwSection;
	PVOID pvOffset;
#ifdef _M_AMD64    
	sStack.AddrPC.Offset = psCtx->Rip;
	sStack.AddrPC.Mode = AddrModeFlat;

	sStack.AddrStack.Offset = psCtx->Rsp;
	sStack.AddrStack.Mode = AddrModeFlat;

	sStack.AddrFrame.Offset = psCtx->Rbp;
	sStack.AddrFrame.Mode = AddrModeFlat;
#else
	sStack.AddrPC.Offset = psCtx->Eip;
	sStack.AddrPC.Mode = AddrModeFlat;

	sStack.AddrStack.Offset = psCtx->Esp;
	sStack.AddrStack.Mode = AddrModeFlat;

	sStack.AddrFrame.Offset = psCtx->Ebp;
	sStack.AddrFrame.Mode = AddrModeFlat;
#endif

	SYSTEM_INFO sysinfo = { 0 };
	GetSystemInfo(&sysinfo);

	DWORD MachineType;
	switch (sysinfo.wProcessorArchitecture)
	{
	case PROCESSOR_ARCHITECTURE_AMD64:
		MachineType = IMAGE_FILE_MACHINE_AMD64;
		break;

	case PROCESSOR_ARCHITECTURE_IA64:
		MachineType = IMAGE_FILE_MACHINE_IA64;
		break;
	case PROCESSOR_ARCHITECTURE_INTEL:
		MachineType = IMAGE_FILE_MACHINE_I386;
		break;

	case PROCESSOR_ARCHITECTURE_ARM:
	case PROCESSOR_ARCHITECTURE_UNKNOWN:
	default:
		MachineType = 0;
		break;
	}

	while (TRUE)
	{
		psSymbol = (IMAGEHLP_SYMBOL*)tbSymBuf;
		dwSymDisp = 0;

		bResult = StackWalk(MachineType, hProcess, hThread, &sStack, psCtx, 0, SymFunctionTableAccess, SymGetModuleBase, 0);
		if (!bResult)
			break;

		if (sStack.AddrFrame.Offset == 0)
			break;

		pcString = ExcptPrint(pcString, pdwStringSize, "  %08p  %08p  ", sStack.AddrPC.Offset, sStack.AddrFrame.Offset);
		ExcptGetLogicalAddress((void*)sStack.AddrPC.Offset, szModule, sizeof(szModule), &dwSection, &pvOffset);

		psSymbol->SizeOfStruct = sizeof(tbSymBuf);
		psSymbol->MaxNameLength = 512;

		bResult = SymGetSymFromAddr(GetCurrentProcess(), sStack.AddrPC.Offset, &dwSymDisp, psSymbol);
		if (bResult)
		{
			bResult = UnDecorateSymbolName(psSymbol->Name, szUndecor, sizeof(szUndecor), UNDNAME_COMPLETE);
			if (bResult)
				pcString = ExcptPrint(pcString, pdwStringSize, "%s+%08X (%s)\n", szUndecor, dwSymDisp, szModule);
			else
				pcString = ExcptPrint(pcString, pdwStringSize, "%s+%08p (%s)\n", psSymbol->Name, dwSymDisp, szModule);
		}
		else
		{
			pcString = ExcptPrint(pcString, pdwStringSize, "%04X:%08p %s\n", dwSection, pvOffset, szModule);
		}
	}

	return pcString;
}

// ------------------------------------------------------------------------

PROTECTED void WINAPI ExcptGetLogicalAddress(PVOID pvAddr, char* szModule, long sizeModule, DWORD* pdwSection, PVOID* ppvOffset)
{
	PVOID pvModule;
	PVOID pvRva;
	DWORD dwIndex;
	PVOID pvSectionStart;
	PVOID pvSectionEnd;
	IMAGE_DOS_HEADER* psDosHdr;
	IMAGE_NT_HEADERS* psNtHdr;
	IMAGE_SECTION_HEADER* psSection;
	MEMORY_BASIC_INFORMATION sMemInfo;

	(*pdwSection) = 0;
	(*ppvOffset) = (PVOID)pvAddr;

	if (szModule != NULL && sizeModule > 0)
	{
		strcpy_s(szModule, sizeModule, "#ERR(LOGICALADDRESS)#");
	}

	if (!VirtualQuery(pvAddr, &sMemInfo, sizeof(sMemInfo)))
		return;

	pvModule = sMemInfo.AllocationBase;

	if (szModule != NULL && sizeModule > 0)
	{
		if (!GetModuleFileName((HMODULE)pvModule, szModule, sizeModule))
		{
			strcpy_s(szModule, sizeModule, "#ERR(LOGICALADDRESS)#");
		}
	}

	psDosHdr = (PIMAGE_DOS_HEADER)pvModule;
	psNtHdr = (PIMAGE_NT_HEADERS)((__int64)pvModule + psDosHdr->e_lfanew);
	psSection = IMAGE_FIRST_SECTION(psNtHdr);
	pvRva = (PVOID)((__int64)pvAddr - (__int64)pvModule);

	for (dwIndex = 0; dwIndex < psNtHdr->FileHeader.NumberOfSections; dwIndex++, psSection++)
	{
		pvSectionStart = (PVOID)psSection->VirtualAddress;

		if (psSection->SizeOfRawData > psSection->Misc.VirtualSize)
			pvSectionEnd = (PVOID)((__int64)pvSectionStart + psSection->SizeOfRawData);
		else
			pvSectionEnd = (PVOID)((__int64)pvSectionStart + psSection->Misc.VirtualSize);

		if ((pvRva >= pvSectionStart) && (pvRva <= pvSectionEnd))
		{
			(*pdwSection) = dwIndex + 1;
			(*ppvOffset) = (PVOID)((__int64)pvRva - (__int64)pvSectionStart);
			return;
		}
	}
}

// ------------------------------------------------------------------------

PROTECTED char* WINAPI ExcptGetExceptionString(DWORD dwCode)
{
#define MK_STR( x ) case EXCEPTION_##x: return (#x);

	switch (dwCode)
	{
		MK_STR(ACCESS_VIOLATION)
			MK_STR(DATATYPE_MISALIGNMENT)
			MK_STR(BREAKPOINT)
			MK_STR(SINGLE_STEP)
			MK_STR(ARRAY_BOUNDS_EXCEEDED)
			MK_STR(FLT_DENORMAL_OPERAND)
			MK_STR(FLT_DIVIDE_BY_ZERO)
			MK_STR(FLT_INEXACT_RESULT)
			MK_STR(FLT_INVALID_OPERATION)
			MK_STR(FLT_OVERFLOW)
			MK_STR(FLT_STACK_CHECK)
			MK_STR(FLT_UNDERFLOW)
			MK_STR(INT_DIVIDE_BY_ZERO)
			MK_STR(INT_OVERFLOW)
			MK_STR(PRIV_INSTRUCTION)
			MK_STR(IN_PAGE_ERROR)
			MK_STR(ILLEGAL_INSTRUCTION)
			MK_STR(NONCONTINUABLE_EXCEPTION)
			MK_STR(STACK_OVERFLOW)
			MK_STR(INVALID_DISPOSITION)
			MK_STR(GUARD_PAGE)
			MK_STR(INVALID_HANDLE)
	}

	return "#ERR(EXCEPTION)#";

#undef MK_STR
}

// ------------------------------------------------------------------------

PROTECTED char* WINAPI ExcptGetPriorityString(DWORD dwCode)
{
#define MK_STR( x ) case THREAD_PRIORITY_##x: return (#x);

	switch (dwCode)
	{
		MK_STR(IDLE)
			MK_STR(LOWEST)
			MK_STR(BELOW_NORMAL)
			MK_STR(NORMAL)
			MK_STR(ABOVE_NORMAL)
			MK_STR(HIGHEST)
			MK_STR(TIME_CRITICAL)
	}

	return "#ERR(PRIORITY)#";

#undef MK_STR
}


// ------------------------------------------------------------------------


PROTECTED char* WINAPI ExcptGetPriorityClassString(DWORD dwCode)
{
#define MK_STR( x ) case x##_PRIORITY_CLASS : return (#x);

	switch (dwCode)
	{
		//        MK_STR( ABOVE_NORMAL )
		//        MK_STR( BELOW_NORMAL )
		MK_STR(HIGH)
			MK_STR(IDLE)
			MK_STR(NORMAL)
			MK_STR(REALTIME)
	}

	return "#ERR(PRIORITYCLASS)#";
#undef MK_STR
}


// ------------------------------------------------------------------------


PROTECTED char* WINAPI ExcptPrint(char* pcString, DWORD* pdwSize, char* pcFormat, ...)
{
	DWORD dwSize = (*pdwSize);
	int iCount;
	va_list sArgs;

	if ((pcString != NULL) && (dwSize > 0))
	{
		va_start(sArgs, pcFormat);
		iCount = _vsnprintf_s(pcString, dwSize, dwSize, pcFormat, sArgs);
		if ((iCount < 0) || ((DWORD)iCount == dwSize))
		{
			pcString[dwSize - 1] = '\0';
			(*pdwSize) = 0;
			return NULL;
		}

		(*pdwSize) = dwSize - (DWORD)iCount;
		pcString += iCount;
		return pcString;
	}

	return NULL;
}


// ------------------------------------------------------------------------


PROTECTED DWORD WINAPI ExcptThreadWrapper(void* pvThread)
{
	DWORD dwReturnCode = NO_ERROR;
	EXCPT_THREAD sThread;

	sThread = *(EXCPT_THREAD*)pvThread;
	HeapFree(GetProcessHeap(), 0, pvThread);

	__try
	{
		dwReturnCode = sThread.pfStartAddress(sThread.pvParameter);
	}
	__except (ExcptLog((EXCEPTION_POINTERS*)_exception_info(), sThread.szName), dwReturnCode = _exception_code(), EXCEPTION_EXECUTE_HANDLER)
	{
		if (sThread.pvExcptHook == EXPT_MODE_DEFAULT)
			sThread.pvExcptHook = gpvExcptHook;

		if (sThread.pvExcptHook == EXPT_MODE_EXITTHREAD)
		{
			ExitThread(dwReturnCode);
		}
		else if ((sThread.pvExcptHook == EXPT_MODE_EXITPROCESS) || (sThread.pvExcptHook == EXPT_MODE_DEFAULT))
		{
			ExitProcess(dwReturnCode);
		}
		else
		{
			__try
			{
				((EXCPT_CALLBACK*)sThread.pvExcptHook)(dwReturnCode, sThread.szName);
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
			}
		}
	}

	ExitThread(dwReturnCode);
}

// ------------------------------------------------------------------------


PROTECTED char* WINAPI ExcptBuildSearchPath(char* szList, DWORD dwListSize)
{
	BOOL bFirst = TRUE;
	char* pcCur = szList;
	char* pcWork;
	char szBuffer[MAX_DUMP_SIZE + 1];

	(*szList) = '\0';

	if (GetCurrentDirectory(sizeof(szBuffer), szBuffer))
	{
		pcCur = ExcptPrint(pcCur, &dwListSize, "%s%s", bFirst ? "" : ";", szBuffer);
		bFirst = FALSE;
	}

	if (GetModuleFileName(NULL, szBuffer, sizeof(szBuffer)))
	{
		if ((pcWork = strrchr(szBuffer, '\\')) == NULL)
			pcWork = strrchr(szBuffer, ':');

		if (pcWork != NULL)
			(*(pcWork + 1)) = '\0';

		pcCur = ExcptPrint(pcCur, &dwListSize, "%s%s", bFirst ? "" : ";", szBuffer);
		bFirst = FALSE;
	}

	if (GetEnvironmentVariable("_NT_SYMBOL_PATH", szBuffer, sizeof(szBuffer)))
	{
		pcCur = ExcptPrint(pcCur, &dwListSize, "%s%s", bFirst ? "" : ";", szBuffer);
		bFirst = FALSE;
	}

	if (GetEnvironmentVariable("_NT_ALTERNATE_SYMBOL_PATH", szBuffer, sizeof(szBuffer)))
	{
		pcCur = ExcptPrint(pcCur, &dwListSize, "%s%s", bFirst ? "" : ";", szBuffer);
		bFirst = FALSE;
	}

	if (GetEnvironmentVariable("SYSTEMROOT", szBuffer, sizeof(szBuffer)))
	{
		pcCur = ExcptPrint(pcCur, &dwListSize, "%s%s", bFirst ? "" : ";", szBuffer);
		bFirst = FALSE;
	}

	return szList;
}

#define EXCP_FILE_NAME "EXCPT.ERR"

PRIVATE VOID GetExceptFilePath(char* path, long size)
{
	BOOL is_configured = FALSE;
	HKEY hKey;

	char path_directory[MAX_PATH] = { 0 };
	DWORD size_path_directory = sizeof(path_directory);

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\CSRoute\\LaneController\\Config\\", 0, KEY_READ, &hKey) == ERROR_SUCCESS)
	{
		DWORD type;

		if (RegQueryValueEx(hKey, "ErrorPath", NULL, &type, (void*)path_directory, &size_path_directory) == ERROR_SUCCESS)
		{
			is_configured = TRUE;
		}

		RegCloseKey(hKey);
	}

	if (is_configured == FALSE)
	{
		GetCurrentDirectory(size_path_directory, path_directory);
	}

	_makepath_s(path, size, NULL, path_directory, EXCP_FILE_NAME, NULL);
}

PRIVATE VOID GetExceptFileMaxSize(DWORD* p_size)
{
	HKEY hKey;
	LONG result = 1400000;

	DWORD buffer = 0;
	DWORD size_buffer = sizeof(buffer);

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\CSRoute\\LaneController\\Config\\", 0, KEY_READ, &hKey) == ERROR_SUCCESS)
	{
		DWORD type;

		if (RegQueryValueEx(hKey, "FileMaxSize", NULL, &type, (void*)&buffer, &size_buffer) == ERROR_SUCCESS)
		{
			result = buffer;
		}

		RegCloseKey(hKey);
	}

	*p_size = result;
}

void __cdecl myInvalidParameterHandler(const wchar_t* expression,
   const wchar_t* function, 
   const wchar_t* file, 
   unsigned int line, 
   uintptr_t pReserved)
{
	DWORD	*pdw = NULL;

	// throw execption to get EXCEPT trace
	*pdw = 10000;
   
	abort();
}

extern int APIENTRY DllMain (HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason) 
    {  
        // The DLL is loading due to process 
        // initialization or a call to LoadLibrary.  
        case DLL_PROCESS_ATTACH:
		   _set_invalid_parameter_handler(myInvalidParameterHandler);
           break;
        
        // The attached process creates a new thread.  
        case DLL_THREAD_ATTACH:  
            break; 

         // The thread of the attached process terminates. 
        case DLL_THREAD_DETACH:  
            break;  

        // The DLL unloading due to process termination or call to FreeLibrary. 
         case DLL_PROCESS_DETACH:  
            break;          

         default:             
            break;     
    }      
    
    return TRUE; 

    UNREFERENCED_PARAMETER(hInstance);     
    UNREFERENCED_PARAMETER(lpReserved); 
} 
