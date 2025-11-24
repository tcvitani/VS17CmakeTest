/******************* (v) 2007 CSSI - All rights reserved *********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE:                                 */
/* FILE:     .c                                                       */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             This file contains functions to:                              */
/*                           */
/*                  */
/*                                              */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

/*-------------------------------- INCLUDES:  -------------------------------*/
// VC++ interface
#include <windows.h>
#include <stdio.h>
#include "resource.h"
#include "csr_aes.h"

/*-------------------------------- RESERVED:  -------------------------------*/
#include <memclass.h>
/*-------------------------------- EXTERNALS: -------------------------------*/
/*-------------------------------- DEFINES:   -------------------------------*/
/*-------------------------------- TYPEDEFS:  -------------------------------*/
/*-------------------------------- FUNCTIONS: -------------------------------*/
PRIVATE int APIENTRY InitDlgProc(HWND hDlg,
								 WORD wMsg, 
								 LONG wParam, 
								 LONG lParam);
/*-------------------------------- VARIABLES: -------------------------------*/
/*-------------------------------- CODE:      -------------------------------*/

/**/
/*****************************************************************************/
/*SYNTAX: int APIENTRY WinMain( HINSTANCE hInstance,                         */
/*								HINSTANCE hPrevInstance,                     */
/*								LPSTR     lpCmdLine,                         */
/*								int       nCmdShow)                          */
/*===========================================================================*/
/*TYPE:   Main function.                                                     */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            Application entry point function. The function initializes the */
/*            application and creates the main dialog box.                   */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      (IN ) HINSTANCE hInstance     - Handle to the current instance of    */
/*                                      the application.                     */
/*      (IN ) HINSTANCE hPrevInstance - Handle to the previous instance of   */
/*                                      the application.                     */
/*      (IN ) LPSTR     lpCmdLine     - Pointer to a null-terminated string  */
/*                                      specifying the command line for the  */
/*                                      application, excluding the program   */
/*                                      name.                                */
/*      (IN ) int       nCmdShow      - Specifies how the window is to be    */
/*                                      shown.                               */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  0                     The function succeeds.                             */
/*  non zero              The function does not succeeds.                    */
/*****************************************************************************/
int APIENTRY WinMain(HINSTANCE hInstance,
					 HINSTANCE hPrevInstance,
					 LPSTR     lpCmdLine,
					 int       nCmdShow)
{
	DWORD     retCode;
	
	UNREFERENCED_PARAMETER( nCmdShow );
	
	retCode = DialogBox((HANDLE)hInstance, 
		MAKEINTRESOURCE(IDD_CRYPTTEST),
		NULL, 
		(DLGPROC)InitDlgProc);
	
	return (retCode);
}

/**/
/*******************************************************************/
/*SYNTAX: int APIENTRY InitDlgProc( HWND hDlg,                     */
/*									WORD wMsg,                     */
/*									LONG wParam,                   */
/*									LONG lParam)                   */
/*=================================================================*/
/*TYPE:   Local function.                                          */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*            The function processes the messages in the main      */
/*            dialog box queue.                                    */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*      (IN ) HWND hDlg    - Identifies the dialog box.            */
/*      (IN ) WORD wMsg    - Specifies the message.                */
/*      (IN ) LONG wParam  - Specifies additional message-specific */
/*                           information.                          */
/*      (IN ) LONG lParam  - Specifies additional message-specific */
/*                           information.                          */
/*=================================================================*/
/*  Return                Description                              */
/*-----------------------------------------------------------------*/
/*  0                     The function does not processes the      */
/*                        message.                                 */
/*  non zero              The function processes the message.      */
/*******************************************************************/
#define STREAM_LENGTH 1024

PRIVATE int APIENTRY InitDlgProc(HWND hDlg,
								 WORD wMsg, 
								 LONG wParam, 
								 LONG lParam)
{
	UCHAR	szInputFile[MAX_PATH];
	UCHAR	szOutputFile[MAX_PATH];
	UCHAR	szInputStream[STREAM_LENGTH];
//	UCHAR	szOutputStream[STREAM_LENGTH];
// 	LPBYTE	lpbyteInput;
// 	LPBYTE	lpbyteOutput;
	UCHAR	szKey[MAX_PATH];
	UCHAR	szErrorMsg[MAX_PATH];
	UCHAR	digest[33];
	DWORD	dwRetVal = CRYPT_OK;
	DWORD	dwFileLen = 0;
// 	FILE	*pFile;			
	int		iRetVal = CRYPT_OK;
	
	memset( szInputStream, 0, STREAM_LENGTH );
//	memset( szOutputStream, 0, STREAM_LENGTH );

	UNREFERENCED_PARAMETER( lParam );
	
	switch (wMsg)
	{
	case WM_INITDIALOG:
		SetDlgItemText(hDlg, IDC_KEY, "0123456789abcdeffedcba9876543210" );
		SetDlgItemText(hDlg, IDC_ENC_SRC_FILE, "input.jpg" );
		SetDlgItemText(hDlg, IDC_ENC_DEST_FILE, "output.jpg" );
		SetDlgItemText(hDlg, IDC_DEC_DEST_FILE, "output2.jpg" );

		break;
		
		
	case WM_COMMAND:
		switch (LOWORD(wParam)) // identify WM_COMMAND
		{
			
		case IDEXIT :
			EndDialog(hDlg, 0);
			break;
			
		case IDC_ENCRYPT_FILE:
			GetDlgItemText(hDlg, IDC_ENC_SRC_FILE, szInputFile, MAX_PATH );
			GetDlgItemText(hDlg, IDC_ENC_DEST_FILE, szOutputFile, MAX_PATH );
			GetDlgItemText(hDlg, IDC_KEY, szKey, MAX_PATH );
			
			if((strcmp(szInputFile,"")==0)||(strcmp(szOutputFile,"")==0))
			{
				sprintf( szErrorMsg, "Please set correct input and output file name!", iRetVal  );
				MessageBox( hDlg, szErrorMsg, "Error", MB_OK );
				break;
			}

			dwRetVal = AESEncryptFile( szInputFile, szOutputFile, szKey );
				if( dwRetVal != CRYPT_OK )
				{
					sprintf( szErrorMsg, "Error encoding file! Error code: %d", dwRetVal );
					MessageBox( hDlg, szErrorMsg, "Error", MB_OK );
				}
				else 
				{
					iRetVal  = MD5_DecryptedFile( szInputFile, AES_BLOCK_SIZE, digest );
					if( iRetVal  != CRYPT_OK )
					{
						sprintf( szErrorMsg, "Error encoding file! Error code: %d", iRetVal  );
						MessageBox( hDlg, szErrorMsg, "Error", MB_OK );
					}
					else
					{
						SetDlgItemText(hDlg, IDC_MD5_FILE1, digest );
					}
				}

		

			break;
			
		case IDC_DECRYPT_FILE:
			GetDlgItemText(hDlg, IDC_ENC_DEST_FILE, szInputFile, MAX_PATH );
			GetDlgItemText(hDlg, IDC_DEC_DEST_FILE, szOutputFile, MAX_PATH );
			GetDlgItemText(hDlg, IDC_KEY, szKey, MAX_PATH );
			
			if((strcmp(szInputFile,"")==0)||(strcmp(szOutputFile,"")==0))
			{
				sprintf( szErrorMsg, "Please set correct input and output file name!", iRetVal  );
				MessageBox( hDlg, szErrorMsg, "Error", MB_OK );
				break;
			}

			dwRetVal = AESDecryptFile( szInputFile, szOutputFile, szKey );
			if( dwRetVal != CRYPT_OK )
			{
				sprintf( szErrorMsg, "Error encoding file! Error code: %d", dwRetVal );
				MessageBox( hDlg, szErrorMsg, "Error", MB_OK );
			}
			else 
			{
				iRetVal  = MD5_DecryptedFile( szOutputFile, AES_BLOCK_SIZE, digest );
				if( iRetVal  != CRYPT_OK )
				{
					sprintf( szErrorMsg, "Error encoding file! Error code: %d", iRetVal  );
					MessageBox( hDlg, szErrorMsg, "Error", MB_OK );
				}
				else
				{
					SetDlgItemText(hDlg, IDC_MD5_FILE3, digest );
				}
			}
			break;
		case IDC_MD5_ENCSRC :
			GetDlgItemText(hDlg, IDC_ENC_SRC_FILE, szInputFile, MAX_PATH );
			
			iRetVal  = MD5_DecryptedFile( szInputFile, AES_BLOCK_SIZE, digest );
			if( iRetVal  != CRYPT_OK )
			{
				sprintf( szErrorMsg, "Error encoding file! Error code: %d", iRetVal  );
				MessageBox( hDlg, szErrorMsg, "Error", MB_OK );
			}
			else
			{
				SetDlgItemText(hDlg, IDC_MD5_FILE1, digest );
			}
			
			
			break;
		case IDC_MD5_DECSRC :
			GetDlgItemText(hDlg, IDC_ENC_DEST_FILE, szInputFile, MAX_PATH );
			
			iRetVal  = MD5_DecryptedFile( szInputFile, AES_BLOCK_SIZE, digest );
			if( iRetVal  != CRYPT_OK )
			{
				sprintf( szErrorMsg, "Error encoding file! Error code: %d", iRetVal  );
				MessageBox( hDlg, szErrorMsg, "Error", MB_OK );
			}
			else
			{
				SetDlgItemText(hDlg, IDC_MD5_FILE2, digest );
			}
			
			
			break;
		case IDC_MD5_DECDEST :
			GetDlgItemText(hDlg, IDC_DEC_DEST_FILE, szInputFile, MAX_PATH );
			
			iRetVal  = MD5_DecryptedFile( szInputFile, AES_BLOCK_SIZE, digest );
			if( iRetVal  != CRYPT_OK )
			{
				sprintf( szErrorMsg, "Error encoding file! Error code: %d", iRetVal  );
				MessageBox( hDlg, szErrorMsg, "Error", MB_OK );
			}
			else
			{
				SetDlgItemText(hDlg, IDC_MD5_FILE3, digest );
			}
			
			
			break;
			
 		}
 		break;
			

		case WM_CLOSE :
			EndDialog(hDlg, 0);
			break;
			
		case WM_DESTROY :
			break;
	}
	
	return (FALSE);
}