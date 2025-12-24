/********** (v) 2007 CSSI - All rights reserved ********************/
/*                                                                 */
/* ----------------------------------------------------------------*/
/* MODULE:   .													   */
/* FILE:     language.h											   */
/* LANGUAGE: C                                                     */
/* ----------------------------------------------------------------*/
/* DESCRIPTION:                                                    */
/* ----------------------------------------------------------------*/
/* HISTORY:                                                        */
/*******************************************************************/
/*--------------------------- INCLUDES:  --------------------------*/
#include <windows.h>
/*--------------------------- RESERVED:  --------------------------*/

/*--------------------------- EXTERNALS: --------------------------*/

/*--------------------------- DEFINES:   --------------------------*/

/*--------------------------- TYPEDEFS:  --------------------------*/

/*--------------------------- FUNCTIONS: --------------------------*/
VOID SetControlText( IN HWND hDlg, IN int iControl, char* szString );
VOID StartUsingLanguages( IN LPSTR lpBaseKey );
VOID EndUsingLanguages( VOID );
VOID AddNewTranslationObject( HWND hWnd, HWND hWndParent, LPSTR lpRegValue );
VOID SwitchDefaultLanguage( VOID );
VOID SwitchNextLanguage ( VOID );
VOID UpdateTranslations( HWND hWndParent );
LPSTR Translation( LPSTR szQuery );

/*--------------------------- VARIABLES: --------------------------*/

/*--------------------------- END OF FILE -------------------------*/
