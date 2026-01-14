///////////////////////////// INCLUDES /////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <resource.h>
#include <windows.h>
#include <hashcalc.h>

///////////////////////////// RESERVED /////////////////////////////
///////////////////////////// DEFINES //////////////////////////////
///////////////////////////// TYPEDEFS /////////////////////////////
///////////////////////////// FUNCTIONS ////////////////////////////
void ConvertBytesToHexStringLoc(IN BYTE *bytes, IN DWORD dwLen, OUT UCHAR *szHex);
void ConvertHexStringToBytesLoc(IN UCHAR *szHex, OUT BYTE *pBytes, OUT DWORD *dwLen);

int APIENTRY InitDlgProc (HWND hDlg, WORD wMsg, LONG wParam, LONG lParam);

///////////////////////////// VARIABLES/////////////////////////////
HANDLE hInst;
char szInputData[10000] = {0};
char szInputDataHex[20000] = {0};
char szHash[SHA256_LEN_HEX_STRING + 1] = {0};

//WCHAR buf[] = L"6F766F206A652074657374";	// string "ovo je test" in hex format
WCHAR output[65] = {0};

///////////////////////////// CODE /////////////////////////////////
int APIENTRY WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	DWORD retCode = 0;
	
	hInst   = hInstance;

	retCode = (DWORD)DialogBox ((HANDLE)hInst, (LPCSTR)IDD_MAIN, NULL, (DLGPROC)InitDlgProc);
	
	return  (retCode);
}

int APIENTRY InitDlgProc (HWND hDlg, WORD wMsg, LONG wParam, LONG lParam)
{
	UNREFERENCED_PARAMETER( lParam );
	
	switch (wMsg)
	{
		case WM_INITDIALOG:
			SetFocus(GetDlgItem(hDlg, IDC_INPUT));
			break;

			
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
			case ID_GET_SHA256:
				memset(szInputData, 0, sizeof(szInputData));

				if(GetDlgItemText(hDlg, IDC_INPUT, szInputData, sizeof(szInputData)))
				{
					memset(szHash, 0, sizeof(szHash));

					ConvertBytesToHexString(szInputData, (DWORD)strlen(szInputData), szInputDataHex);

					SetDlgItemText(hDlg, IDC_HEX, szInputDataHex);

					if(CalculateSha256(szInputDataHex, (int)strlen(szInputDataHex), sizeof(szHash), szHash) == ERR_OK)
					{

						SetDlgItemText(hDlg, IDC_HASH, szHash);
					}
					else
					{
						SetDlgItemText(hDlg, IDC_HASH, "");
					}
				}

				//CalculateSha256_w(buf, wcslen(buf), sizeof(output)/sizeof(WCHAR), output);
				break;

			case ID_EXIT:
				EndDialog (hDlg, TRUE);
				break;
			}
			break;

		case WM_CLOSE:
			EndDialog (hDlg, TRUE);
			break;
	}
	return (FALSE);
}

void ConvertBytesToHexStringLoc(IN BYTE *bytes, IN DWORD dwLen, OUT UCHAR *szHex)
{
	DWORD	dwH, dwA; 
	UCHAR	chCh;
	
	for (dwH=0, dwA=0; dwH<dwLen; dwH++, dwA+=2)
	{
		szHex[dwA] = (chCh = (bytes[dwH] & 0xF0) >> 4) < 10 ? (chCh | 0x30) : (chCh + 55);
		szHex[dwA+1] = (chCh = bytes[dwH] & 0x0F) < 10 ? (chCh | 0x30) : (chCh + 55);
	}
	szHex[dwA] = '\0';
}

void ConvertHexStringToBytesLoc(IN UCHAR *szHex, OUT BYTE *pBytes, OUT DWORD *dwLen)
{
	DWORD	dwH, dwA; 
	UCHAR	chCh;
	
	for (dwH=0, dwA=0; dwH<strlen(szHex)/2; dwH++, dwA+=2)
	{
		pBytes[dwH] = ((((chCh = szHex[dwA])  >= 'A') ? (chCh - 55) : (chCh & 0x0F)) << 4) |
			((((chCh = szHex[dwA+1])  >= 'A') ? (chCh - 55) : (chCh & 0x0F)));
	}
	*dwLen = dwH;
}