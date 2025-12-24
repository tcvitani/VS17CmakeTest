/******************* (v) 2007 CSSI - All rights reserved *********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE:   .																 */
/* FILE:     language.c														 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

/*-------------------------------- INCLUDES:  -------------------------------*/
#include <windows.h>
#include <stdio.h>
#include <language.h>
/*-------------------------------- RESERVED:  -------------------------------*/

/*-------------------------------- EXTERNALS: -------------------------------*/

/*-------------------------------- DEFINES:   -------------------------------*/
#define REG_VAL_DEFAULT_LANG "Languages"
/*-------------------------------- TYPEDEFS:  -------------------------------*/
typedef struct
{	
	char szKeyLanguage[MAX_PATH];
}language_params;

typedef struct
{
	HWND hWnd;
	HWND hWndParent;
	char szRegLabel[MAX_PATH];
	char szCurrentTrans[MAX_PATH];
}reg_wnd_pair;
/*-------------------------------- FUNCTIONS: -------------------------------*/
VOID EnumerateLanguages( VOID );
/*-------------------------------- VARIABLES: -------------------------------*/
char	szBaseKey[MAX_PATH] = { '\0' };
DWORD	dwLangKeyCount		= 0;
DWORD	dwCurrentLangIndex	= 0;
DWORD	dwDefLangIndex		= 0;
int		iTransObjectCount	= 0;
BOOL	bUpdated			= TRUE;
HKEY	hKeyBase			= NULL;
language_params*	lpLangArray			= NULL;
reg_wnd_pair*		lpTransObjectArray	= NULL;
/*-------------------------------- CODE:      -------------------------------*/
/*****************************************************************************/
/*SYNTAX:																	 */
/*===========================================================================*/
/*TYPE:   Local function.                                                    */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*																			 */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*		(IN)																 */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*                        Function does not return value					 */
/*****************************************************************************/
VOID StartUsingLanguages( IN LPSTR lpBaseKey )
{
	// Check if lpBaseKey is NULL
	if( lpBaseKey == NULL )
		return;
	// Set szBaseKey
	sprintf( szBaseKey, "%s", lpBaseKey );

	// Enumerate languages
	EnumerateLanguages();			
}
/*****************************************************************************/
/*SYNTAX:																	 */
/*===========================================================================*/
/*TYPE:   Local function.                                                    */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*																			 */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*		(IN)																 */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*                        Function does not return value					 */
/*****************************************************************************/
VOID EndUsingLanguages( VOID )
{
	dwLangKeyCount = 0;
	free( lpLangArray );
}
/*****************************************************************************/
/*SYNTAX:																	 */
/*===========================================================================*/
/*TYPE:   Local function.                                                    */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*																			 */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*		(IN)																 */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*                        Function does not return value					 */
/*****************************************************************************/
BOOL OpenBaseKey()
{
	LONG lRet;

	lRet = RegOpenKeyEx(
					HKEY_LOCAL_MACHINE,			// handle to open key
					szBaseKey,					// address of name of sub-key to open
					0,							// reserved
					KEY_ALL_ACCESS,				// security access mask
					&hKeyBase					// address of handle to open key
					) ;

	if( lRet == ERROR_SUCCESS)
	{
		return TRUE;
	}
	
	return FALSE;
}
/*****************************************************************************/
/*SYNTAX:																	 */
/*===========================================================================*/
/*TYPE:   Local function.                                                    */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*																			 */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*		(IN)																 */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*                        Function does not return value					 */
/*****************************************************************************/
BOOL CloseBaseKey()
{
	if( hKeyBase != NULL )
	{
		// Close base key
		if( RegCloseKey( hKeyBase ) == ERROR_SUCCESS )
		{
			hKeyBase = NULL;
			return TRUE;
		}
		else
			return FALSE;
	}
	else
		return TRUE;
}
/*****************************************************************************/
/*SYNTAX:																	 */
/*===========================================================================*/
/*TYPE:   Local function.                                                    */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*																			 */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*		(IN)																 */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*                        Function does not return value					 */
/*****************************************************************************/
VOID SetControlText( IN HWND hDlg, IN int iControl, char* szString )
{
	SetWindowText( GetDlgItem( hDlg, iControl ), szString ); 
}
/*****************************************************************************/
/*SYNTAX:																	 */
/*===========================================================================*/
/*TYPE:   Local function.                                                    */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*																			 */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*		(IN)																 */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*                        Function does not return value					 */
/*****************************************************************************/
VOID AddNewLanguage( language_params* lpLP )
{
	// Add graph data to graph data array
	language_params* lpNewLangArray = NULL;
	DWORD i = 0;
	
	// Create new array that is larger for one item
	lpNewLangArray = malloc( sizeof( language_params ) * ( dwLangKeyCount + 1 ) );
	
	// Copy graph data array to new array
	for( i = 0; i<dwLangKeyCount; i++ )
	{
		memcpy(	&lpNewLangArray[i],
				&lpLangArray[i],
				sizeof( language_params ) );
	}
	
	// Free old graph data array
	free( lpLangArray );
	lpLangArray = NULL;
	// Set new graph array as current graph array
	lpLangArray = lpNewLangArray;
	// Add new graph data to graph data array
	sprintf( lpLangArray[dwLangKeyCount].szKeyLanguage,
			 "%s",
			 lpLP->szKeyLanguage );
	// Increase graph data count
	dwLangKeyCount++;
}
/*****************************************************************************/
/*SYNTAX:																	 */
/*===========================================================================*/
/*TYPE:   Local function.                                                    */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*																			 */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*		(IN)																 */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*                        Function does not return value					 */
/*****************************************************************************/
VOID ReadDefaultLang( VOID )
{
	DWORD i;
	DWORD	dwItemSize = MAX_PATH * sizeof( CHAR );
	DWORD	dwType;
	LONG	lRetVal		= 0;
	char	szDefValBuffer[MAX_PATH] = { '\0' };
	// MFR start 26/9/2007
	HKEY	hConfKey	= NULL;
	CHAR	szConfKey[MAX_PATH]	 =  "SOFTWARE\\CsRoute\\Maintenance\\Config";


	RegOpenKeyEx(
		HKEY_LOCAL_MACHINE,			// handle to open key
		szConfKey,					// address of name of sub-key to open
		0,							// reserved
		KEY_ALL_ACCESS ,			// security access mask
		&hConfKey					// address of handle to open key
		);
	// MFR end   26/9/2007
	
	lRetVal = RegQueryValueEx(
		// MFR start 26/9/2007
//		hKeyBase,					// handle to key to query
		hConfKey,					// handle to key to query
		// MFR end   26/9/2007
		REG_VAL_DEFAULT_LANG,	// address of name of value to query
		NULL,						// reserved
		&dwType,					// address of buffer for value type
		szDefValBuffer,// address of data buffer
		&dwItemSize					// address of data buffer size
		);
	
	if( !( lRetVal != ERROR_SUCCESS || dwType != REG_SZ ) )		
	{
		for( i = 0; i < dwLangKeyCount; i++ )
		{
			if( strcmp( szDefValBuffer, lpLangArray[i].szKeyLanguage ) == 0 )
			{
				dwDefLangIndex = i;
				return;
			}
		}
	}
	dwDefLangIndex = 0;
}
/*****************************************************************************/
/*SYNTAX:																	 */
/*===========================================================================*/
/*TYPE:   Local function.                                                    */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*																			 */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*		(IN)																 */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*                        Function does not return value					 */
/*****************************************************************************/
VOID EnumerateLanguages( VOID )
{
	LONG	lRetVal;
	DWORD	dwCurrentKey = 0;
	DWORD	dwKeyNameSize;
	language_params sLP;
	
	// If base key is not open
	if( hKeyBase == NULL )
	{
		// Open base key
		if( !OpenBaseKey() )		
			return;
	}
	// Enumerate all keys
	while( TRUE )
	{
		dwKeyNameSize = MAX_PATH;

		lRetVal = RegEnumKeyEx(
			hKeyBase,			// handle to key to enumerate
			dwCurrentKey,		// index of sub-key to enumerate
			sLP.szKeyLanguage,   // address of buffer for sub-key name
			&dwKeyNameSize,		// address for size of sub-key buffer
			NULL,				// reserved
			NULL,				// address of buffer for class string
			NULL,				// address for size of class buffer
			NULL );				// address for time key last written to

		// If there were errors reading language key
		if( lRetVal != ERROR_SUCCESS )
		{
			// If we read last key
			if( lRetVal == ERROR_NO_MORE_ITEMS )
				break;
			else // There were something else wrong
			{
				// Close base key and abort
				CloseBaseKey();
				return;
			}
		}
		AddNewLanguage( &sLP );
		dwCurrentKey++;
	}
	// Read default lang key and set default lang index
	ReadDefaultLang();
	// Close base key
	CloseBaseKey();
}
/*****************************************************************************/
/*SYNTAX:																	 */
/*===========================================================================*/
/*TYPE:   Local function.                                                    */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*																			 */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*		(IN)																 */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*                        Function does not return value					 */
/*****************************************************************************/
VOID AddNewTranslationObject( HWND hWnd, HWND hWndParent, LPSTR lpRegValue )
{
	reg_wnd_pair* lpNewTransObject = NULL;
	int i = 0;

	// Create new array that is larger for one item
	lpNewTransObject = malloc( sizeof( reg_wnd_pair ) * ( iTransObjectCount + 1 ) );
	
	// Copy graph data array to new array
	for( i = 0; i < iTransObjectCount; i++ )
	{
		memcpy(	&lpNewTransObject[i],
				&lpTransObjectArray[i],
				sizeof( reg_wnd_pair ) );
	}
	
	// Free old graph data array
	free( lpTransObjectArray );
	lpTransObjectArray = NULL;
	// Set new graph array as current graph array
	lpTransObjectArray = lpNewTransObject;
	// Add new graph data to graph data array
	lpTransObjectArray[iTransObjectCount].hWnd = hWnd;
	lpTransObjectArray[iTransObjectCount].hWndParent = hWndParent;
	sprintf( lpTransObjectArray[iTransObjectCount].szRegLabel, "%s",lpRegValue );
	// Increase graph data count
	iTransObjectCount++;
	bUpdated = FALSE;
}
/*****************************************************************************/
/*SYNTAX:																	 */
/*===========================================================================*/
/*TYPE:   Local function.                                                    */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*																			 */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*		(IN)																 */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*                        Function does not return value					 */
/*****************************************************************************/
VOID ReadReg( VOID )
{
	language_params* lpLang = &lpLangArray[dwCurrentLangIndex];
	reg_wnd_pair*	lpTransObject = NULL;
	HKEY hKeyLang = NULL;
	int i;
	DWORD	dwItemSize = MAX_PATH * sizeof( CHAR );
	DWORD	dwType;
	LONG	lRetVal = 0;

	// Open language key
	if( RegOpenKeyEx(
		hKeyBase,				// handle to open key
		lpLang->szKeyLanguage,	// address of name of subkey to open
		0,						// reserved
		KEY_READ,				// security access mask
		&hKeyLang				// address of handle to open key
		) != ERROR_SUCCESS)
	{
		// If there were errors reading language key abort
		return;
	}
	
	
	for( i = 0; i < iTransObjectCount; i++ )
	{
		lpTransObject = &lpTransObjectArray[i];
		dwItemSize = MAX_PATH;
		
		lRetVal = RegQueryValueEx(
			hKeyLang,					// handle to key to query
			lpTransObject->szRegLabel,	// address of name of value to query
			NULL,						// reserved
			&dwType,					// address of buffer for value type
			lpTransObject->szCurrentTrans,// address of data buffer
			&dwItemSize					// address of data buffer size
			);

		if( lRetVal != ERROR_SUCCESS || dwType != REG_SZ )
			sprintf( lpTransObject->szCurrentTrans,
					 "%s",
					 lpTransObject->szRegLabel );
	}	
}
/*****************************************************************************/
/*SYNTAX:																	 */
/*===========================================================================*/
/*TYPE:   Local function.                                                    */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*																			 */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*		(IN)																 */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*                        Function does not return value					 */
/*****************************************************************************/
VOID ReadLanguage( VOID )
{
	if( OpenBaseKey() )
	{
		ReadReg();
		CloseBaseKey();
	}
}
/*****************************************************************************/
/*SYNTAX:																	 */
/*===========================================================================*/
/*TYPE:   Local function.                                                    */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*																			 */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*		(IN)																 */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*                        Function does not return value					 */
/*****************************************************************************/
VOID SwitchNextLanguage ( VOID )
{
	dwCurrentLangIndex++;
	
	if( dwCurrentLangIndex == dwLangKeyCount )
		dwCurrentLangIndex = 0;

	ReadLanguage();

}
/*****************************************************************************/
/*SYNTAX:																	 */
/*===========================================================================*/
/*TYPE:   Local function.                                                    */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*																			 */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*		(IN)																 */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*                        Function does not return value					 */
/*****************************************************************************/
VOID UpdateTranslations( HWND hWndParent )
{
	int i = 0;

	if( hWndParent == NULL )
		return;

	if( !bUpdated )
		ReadLanguage();

	for( i = 0; i < iTransObjectCount; i++ )
	{
		if( lpTransObjectArray[i].hWndParent == hWndParent )
		{
			SetWindowText( lpTransObjectArray[i].hWnd,
				lpTransObjectArray[i].szCurrentTrans );
		}
	}	
}
/*****************************************************************************/
/*SYNTAX:																	 */
/*===========================================================================*/
/*TYPE:   Local function.                                                    */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*																			 */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*		(IN)																 */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*                        Function does not return value					 */
/*****************************************************************************/
VOID SwitchDefaultLanguage( VOID )
{
	dwCurrentLangIndex = dwDefLangIndex;
	
	ReadLanguage();
}
/*****************************************************************************/
/*SYNTAX:																	 */
/*===========================================================================*/
/*TYPE:   Local function.                                                    */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*																			 */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*		(IN)																 */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*                        Function does not return value					 */
/*****************************************************************************/
LPSTR Translation( LPSTR szQuery )
{
	int i = 0;

	if( !bUpdated )
		ReadLanguage();
	
	for( i = 0; i < iTransObjectCount; i++ )
	{
		if( strcmp( lpTransObjectArray[i].szRegLabel, szQuery ) == 0 )		
			return lpTransObjectArray[i].szCurrentTrans;
	}
	{
		static char szReturn[MAX_PATH] = { '\0' };
		sprintf( szReturn, "Unknown registry label :%s", szQuery );
		return szReturn;
	}
}
/*-------------------------------- END OF FILE ------------------------------*/