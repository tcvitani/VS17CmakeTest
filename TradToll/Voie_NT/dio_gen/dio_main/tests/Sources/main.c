#include <windows.h>

#include <string.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <CSRLC32.H>

//#include <noyau.h>
//#define IO_SIMULATOR
#include <dio_interface.h>

#define KEY_MODULE	CSR_REG_KEYn_CSRBASE \
					CSR_REG_KEYn_LANE_BASE \
					CSR_REG_KEYn_CONFIG \
					MOD_REG_KEYn_MODULES


char *dio_err[DIO_OTHER_ERROR + 1] 
= 
{
	{ "DIO_SUCCESS"					},
	{ "DIO_REG_KEY_MISSING"			},
	{ "DIO_INVALID_REG_CONFIG"		},
	{ "DIO_BOARD_INIT_ERROR"		},
	{ "DIO_INDEX_OUT_OF_RANGE"		},
	{ "DIO_FAILED_TO_LOAD_LIBRARY"	},
	{ "DIO_FAILED_TO_FREE_LIBRARY"	},
	{ "DIO_FAILED_TO_LOAD_FUNCTION"	},
	{ "DIO_PLUGIN_CONNECTION_ERROR"	},
	{ "DIO_PLUGIN_API_ERROR"		},
	{ "DIO_OTHER_ERROR"				}
};

struct_dio_dev_status sDeviceStatus = {0};

static void WINAPI Test_DioStatusOccuredEvent(DIO_STATUS_EVENT eStatusEvt,
											  PVOID pData);

void ReadOutputs(DWORD dwLastOutputIndex)
{
	DWORD		i;
	DIO_ERROR	eDioErr;

	printf("\n\n==== READ OUTPUTS ====");
	printf("\n  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15\n");
	for (i=0; i<16; i++)
	{
		if (i <= dwLastOutputIndex)
			printf("  %u", MDioGetOutputState(i, &eDioErr));
		else
			printf("  -");
	}
	printf("\n 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31\n");
	for (i=16; i<32; i++)
	{
		if (i <= dwLastOutputIndex)
			printf("  %u", MDioGetOutputState(i, &eDioErr));
		else
			printf("  -");
	}
	printf("\n 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47\n");
	for (i=32; i<48; i++)
	{
		if (i <= dwLastOutputIndex)
			printf("  %u", MDioGetOutputState(i, &eDioErr));
		else
			printf("  -");
	}
	printf("\n 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63\n");
	for (i=48; i<64; i++)
	{
		if (i <= dwLastOutputIndex)
			printf("  %u", MDioGetOutputState(i, &eDioErr));
		else
			printf("  -");
	}
}

void WriteOutputs(DWORD dwLastOutputIndex, DWORD dwValue)
{
	DWORD		i;
	DIO_ERROR	eDioErr;
	
	printf("\n\n==== WRITE OUTPUTS ====");
	printf("\n  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15\n");
	for (i=0; i<16; i++)
	{
		if (i <= dwLastOutputIndex)
		{
			printf("  %d", dwValue);
			eDioErr = MDioSetOutputState(i, dwValue);
		}
		else
		{
			printf("  -");
		}
	}
	printf("\n 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31\n");
	for (i=16; i<32; i++)
	{
		if (i <= dwLastOutputIndex)
		{
			printf("  %d", dwValue);
			eDioErr = MDioSetOutputState(i, dwValue);
		}
		else
		{
			printf("  -");
		}
	}
	printf("\n 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47\n");
	for (i=32; i<48; i++)
	{
		if (i <= dwLastOutputIndex)
		{
			printf("  %d", dwValue);
			eDioErr = MDioSetOutputState(i, dwValue);
		}
		else
		{
			printf("  -");
		}
	}
	printf("\n 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63\n");
	for (i=48; i<64; i++)
	{
		if (i <= dwLastOutputIndex)
		{
			printf("  %d", dwValue);
			eDioErr = MDioSetOutputState(i, dwValue);
		}
		else
		{
			printf("  -");
		}
	}
}

void WriteInputs(DWORD dwLastInputIndex, DWORD dwValue)
{
	DWORD		i;
	DIO_ERROR	eDioErr;
	
	printf("\n\n==== WRITE INPUTS ====");
	printf("\n  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15\n");
	for (i=0; i<16; i++)
	{
		if (i <= dwLastInputIndex)
		{
			printf("  %d", dwValue);
			eDioErr = MDioSetInputState(i, dwValue);
		}
		else
		{
			printf("  -");
		}
	}
	printf("\n 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31\n");
	for (i=16; i<32; i++)
	{
		if (i <= dwLastInputIndex)
		{
			printf("  %d", dwValue);
			eDioErr = MDioSetInputState(i, dwValue);
		}
		else
		{
			printf("  -");
		}
	}
	printf("\n 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47\n");
	for (i=32; i<48; i++)
	{
		if (i <= dwLastInputIndex)
		{
			printf("  %d", dwValue);
			eDioErr = MDioSetInputState(i, dwValue);
		}
		else
		{
			printf("  -");
		}
	}
	printf("\n 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63\n");
	for (i=48; i<64; i++)
	{
		if (i <= dwLastInputIndex)
		{
			printf("  %d", dwValue);
			eDioErr = MDioSetInputState(i, dwValue);
		}
		else
		{
			printf("  -");
		}
	}
}

void ReadInputs(DWORD dwLastInputIndex)
{
	DWORD		i;
	DIO_ERROR	eDioErr;

	printf("\n==== READ INPUTS ====");
	printf("\n  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15\n");
	for (i=0; i<16; i++)
	{	
		if (i <= dwLastInputIndex)
			printf("  %u", MDioGetInputState(i, &eDioErr));
		else
			printf("  -");
	}
	printf("\n 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31\n");
	for (i=16; i<32; i++)
	{
		if (i <= dwLastInputIndex)
			printf("  %u", MDioGetInputState(i, &eDioErr));
		else
			printf("  -");
	}
	printf("\n 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47\n");
	for (i=32; i<48; i++)
	{
		if (i <= dwLastInputIndex)
			printf("  %u", MDioGetInputState(i, &eDioErr));
		else
			printf("  -");
	}
	printf("\n 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63\n");
	for (i=48; i<64; i++)
	{
		if (i <= dwLastInputIndex)
			printf("  %u", MDioGetInputState(i, &eDioErr));
		else
			printf("  -");
	}

}
__cdecl main()
{
	BOOL		bAut;
	char		ch;
	char		szPluginKey[MAX_PATH]	= {0};
	DIO_ERROR	eDioErr;
	DWORD		dwFirstInputIndex		= 0;
	DWORD		dwLastInputIndex		= 0;
	DWORD		dwFirstOutputIndex		= 0;
	DWORD		dwLastOutputIndex		= 0;
	
	sprintf_s(szPluginKey, sizeof(szPluginKey), "%s%s", KEY_MODULE, DIO_REG_KEYn_IO_ROOT);
		
  	eDioErr = MDioInit(CSR_REG_KEYi_ROOT, szPluginKey, Test_DioStatusOccuredEvent);
//  	eDioErr = MDioInit(CSR_REG_KEYi_ROOT, szPluginKey, NULL);
	if(eDioErr != DIO_SUCCESS)
			printf("MDioInit() -> RETURN ERROR CODE: %s", dio_err[eDioErr]);	

	MDioGetRange(&dwFirstInputIndex, &dwLastInputIndex, &dwFirstOutputIndex, &dwLastOutputIndex);

	printf("\nIO RANGE:");
	printf("\n          * First input index:  %d", dwFirstInputIndex);
	printf("\n          * Last input index:   %d", dwLastInputIndex);
	printf("\n          * First output index: %d", dwFirstOutputIndex);
	printf("\n          * Last output index:  %d\n", dwLastOutputIndex);
	
	while (1)
	{
		printf("\nStart automatic test? (y/n): ");
		scanf_s("%c", &ch);
		
		bAut = FALSE;
		if (ch == 'y' || ch == 'Y')
			bAut = TRUE;

		if (bAut)
		{
			while(TRUE)
			{
// 				ReadInputs(dwLastInputIndex);
// 
// 				printf("\nPress any key to continue... ");
// 				ch = getch();
				
				WriteInputs(dwLastInputIndex, 1);
				
				printf("\nPress any key to continue... ");
// 				ch = getch();

				ReadInputs(dwLastInputIndex);
				
				printf("\nPress any key to continue... ");
// 				ch = getch();
				
				Sleep(1000);

				WriteInputs(dwLastInputIndex, 0);
				
				printf("\nPress any key to continue... ");
// 				ch = getch();

				ReadInputs(dwLastInputIndex);
				
				printf("\nPress any key to continue... ");
// 				ch = getch();

				Sleep(1000);

			}

			ReadOutputs(dwLastOutputIndex);

			printf("\nPress any key to continue... ");
			ch = _getch();
			
			WriteOutputs(dwLastOutputIndex, 1);
			
			printf("\nPress any key to continue... ");
			ch = _getch();
			
			ReadOutputs(dwLastOutputIndex);
			
			printf("\nPress any key to continue... ");
			ch = _getch();
			
			WriteOutputs(dwLastOutputIndex, 0);
			
			printf("\nPress any key to continue... ");
			ch = _getch();
			
			ReadOutputs(dwLastOutputIndex);
			
			printf("\nPress any key to continue... ");
			ch = _getch();
			
			WriteOutputs(dwLastOutputIndex, 0);
			
			printf("\nPress any key to continue... ");
			ch = _getch();
			
			ReadOutputs(dwLastOutputIndex);
		}
	}

	MDioRelease();

	return 1;
}

static void WINAPI Test_DioStatusOccuredEvent(DIO_STATUS_EVENT eStatusEvt,
											  PVOID pData)
{
	P_DIO_STATUS		pDioStatus		= NULL;
	P_DIO_DEV_STATUS	pDeviceStatus	= NULL;
	DWORD				dwIdx			= 0;
	DWORD				dwIdx2			= 0;
	
	switch(eStatusEvt)
	{
	case DIO_DEVICE_STATUS_EVT:
		pDeviceStatus = (struct_dio_dev_status*)pData;
		
		for (dwIdx = 0; dwIdx < pDeviceStatus->dwNbPlugins; dwIdx++)
		{
			for (dwIdx2 = 0; dwIdx2 < pDeviceStatus->sPlugin[dwIdx].dwNbBoards; dwIdx2++)
			{
				if (sDeviceStatus.sPlugin[dwIdx].sBoard[dwIdx2].bLinkError != 
					pDeviceStatus->sPlugin[dwIdx].sBoard[dwIdx2].bLinkError)
				{
					sDeviceStatus.sPlugin[dwIdx].sBoard[dwIdx2].bLinkError = pDeviceStatus->sPlugin[dwIdx].sBoard[dwIdx2].bLinkError;
					
					if (pDeviceStatus->sPlugin[dwIdx].sBoard[dwIdx2].bLinkError)
						printf("\nDIO_DEVICE_STATUS_EVT: LINK ERROR => Plugin: %d, DIO Board: %d", dwIdx, dwIdx2);
					else
						printf("\nDIO_DEVICE_STATUS_EVT: LINK OK => Plugin: %d, DIO Board: %d", dwIdx, dwIdx2);
				}
				
				if (sDeviceStatus.sPlugin[dwIdx].sBoard[dwIdx2].bDeviceError != 
					pDeviceStatus->sPlugin[dwIdx].sBoard[dwIdx2].bDeviceError)
				{
					sDeviceStatus.sPlugin[dwIdx].sBoard[dwIdx2].bDeviceError = pDeviceStatus->sPlugin[dwIdx].sBoard[dwIdx2].bDeviceError;
					
					if (pDeviceStatus->sPlugin[dwIdx].sBoard[dwIdx2].bDeviceError)
						printf("\nDIO_DEVICE_STATUS_EVT: DEVICE ERROR => Plugin: %d, DIO Board: %d", dwIdx, dwIdx2);
					else
						printf("\nDIO_DEVICE_STATUS_EVT: DEVICE OK => Plugin: %d, DIO Board: %d", dwIdx, dwIdx2);
				}
			}
		}
		break;
		
	case DIO_DI_STATUS_EVT:
		pDioStatus = (struct_dio_status*)pData;
		
		printf("\nDIO_DI_STATUS_EVT => Input:%d, Status:%d", 
			pDioStatus->dwDioIdx, 
			pDioStatus->eState);
		break;
		
	case DIO_DO_STATUS_EVT:
		pDioStatus = (struct_dio_status*)pData;
		
		printf("\nDIO_DO_STATUS_EVT => Output:%d, Status:%d", 
			pDioStatus->dwDioIdx, 
			pDioStatus->eState);
		break;
		
	default:
		printf("\nTest_DioStatusOccuredEvent() => Unknown status event: %d", eStatusEvt);
		break;
	}
}