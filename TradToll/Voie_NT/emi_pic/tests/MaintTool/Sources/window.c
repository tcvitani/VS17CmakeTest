/************ (v) 2009 CSEE-Peage -   All rights reserved ********************/
/*																			 */
/* --------------------------------------------------------------------------*/
/* MODULE:																	 */
/* FILE:     window.c														 */
/* LANGUAGE: C																 */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:																	 */
/*****************************************************************************/
/*---------------------------------- INCLUDES:  -----------------------------*/
#include <simu.h>
/*---------------------------------- RESERVED:  -----------------------------*/
#include <memclass.h>
/*---------------------------------- EXTERNALS: -----------------------------*/

/*---------------------------------- DEFINES:   -----------------------------*/

/*---------------------------------- TYPEDEFS:  -----------------------------*/

/*---------------------------------- FUNCTIONS: -----------------------------*/

/*---------------------------------- VARIABLES: -----------------------------*/
int iMainWndHeight = DEFAULT_WND_SIZE;
HBRUSH hBrush[NB_ICONS] = { 0 };
/*---------------------------------- CODE:      -----------------------------*/
/*****************************************************************************/
/*SYNTAX: int APIENTRY WndProc( HWND	hwnd,								 */
/*								UINT	message,							 */
/*								WPARAM  wParam,								 */
/*								LPARAM  lParam)								 */
/*===========================================================================*/
/*TYPE:   Main function.                                                     */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            Application entry point function. The function                 */
/*            initializes the application.                                   */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      (IN ) HWND hwnd			 - Handle to the current instance of         */
/*                                      the application.                     */
/*      (IN ) UINT message		 - Handle to the previous instance of        */
/*                                      the application.                     */
/*      (IN ) WPARAM wParam      - Pointer to a null-terminated string       */
/*                                      specifying the command line for the  */
/*                                      application, excluding the program   */
/*                                      name.                                */
/*      (IN ) LPARAM lParam      - Specifies how the window is to be         */
/*                                      shown.                               */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  0                     The function succeeds.                             */
/*  non zero              The function does not succeed.                     */
/*****************************************************************************/
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{

			}
			break;	

		case WM_CLOSE :
				SendMessage( SIMU.hDlg, WM_CLOSE, wParam, lParam );
				return FALSE;
			break;			
	}
	
	return DefWindowProc (hwnd, message, wParam, lParam) ;
}
/*****************************************************************************/
/*SYNTAX: 	hwndInitWindow(HINSTANCE hInstance, int iCmdShow)	             */
/*===========================================================================*/
/*TYPE:		HWND                                                             */
/*===========================================================================*/
/*DESCRIPTION:	The function create window							         */
/*===========================================================================*/
/*PARAMETERS:(IN ) HINSTANCE hInstance  - Handle to the current instance of  */
/*                                        the application.                   */
/*			 (IN ) int    nCmdShow      - Specifies how the window is to be  */
/*										  shown.                             */    			    
/*===========================================================================*/
/*  Return			Description												 */
/*---------------------------------------------------------------------------*/
/*  void            This function return a handle to created window  	     */
/*****************************************************************************/
HWND hwndInitWindow(HINSTANCE hInstance,int iCmdShow)
{
	
	WNDCLASS     wndclass ;

	wndclass.style         = 0;
	wndclass.lpfnWndProc   = WndProc ;
	wndclass.cbClsExtra    = 0 ;
	wndclass.cbWndExtra    = 0 ;
	wndclass.hInstance     = hInstance ;
	wndclass.hIcon         = LoadIcon (hInstance, MAKEINTRESOURCE( IDI_MAIN_ICON ));
	wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW) ;
	wndclass.hbrBackground = (HBRUSH)(COLOR_APPWORKSPACE);
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = "MaintPyl" ;	
	
	if (!RegisterClass (&wndclass))
	{		
		return NULL ;
	}
	SIMU.hWnd = CreateWindow ("MaintPyl",		// window class name
		"",										// window caption
		WS_SYSMENU|WS_VISIBLE|WS_MINIMIZEBOX,	// window style
		340,									// initial x position
		220,									// initial y position
		0,										// initial x size
		0,										// initial y size
		NULL,									// parent window handle
		NULL,									// window menu handle
		hInstance,								// program instance handle
		NULL) ;									// creation parameters
	
	ShowWindow (SIMU.hWnd, iCmdShow) ;
	UpdateWindow (SIMU.hWnd) ;
	
	return SIMU.hWnd;
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
PUBLIC HBRUSH SetIcon( HWND hwnd, icon_type Icon )
{
	HDC hdc		= NULL;
	RECT rect;
	
	int iLR;
	
	if( hwnd != NULL )
	{
		if( ( hdc = GetDC( hwnd ) ) != NULL )
		{
			GetWindowRect( hwnd, &rect );			
			
			if( SelectObject( hdc, hBrush[Icon] ) == NULL )
				iLR = GetLastError();
			
			// Fill background
			Rectangle( hdc, 0, 0, rect.right-rect.left, rect.bottom-rect.top );
			ReleaseDC( hwnd, hdc );
			
			return hBrush[Icon];
		}
	}
	return NULL;
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
PUBLIC VOID InitBrushes( VOID )
{
	hBrush[ICON_ERROR]		= CreateSolidBrush( RGB( 255, 000, 000 ) ); 
	hBrush[ICON_OK]			= CreateSolidBrush( RGB( 000, 200, 000 ) );
	hBrush[ICON_UNDEFINED]	= CreateSolidBrush( RGB( 200, 200, 200 ) );
	hBrush[ICON_WAITING]	= CreateSolidBrush( RGB( 255, 255, 000 ) );
	hBrush[ICON_INACTIVE]	= CreateSolidBrush( RGB( 000, 000, 000 ) );
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
PUBLIC VOID FreeBrushes( VOID )
{
	INT
		i;
	
	for( i = 0; i < NB_ICONS; i++ )
	{
		if( hBrush[i] != NULL )
		{
			DeleteObject( hBrush[i] );
			hBrush[i] = NULL;
		}
	}	
}
/*---------------------------------- END FILE   -----------------------------*/