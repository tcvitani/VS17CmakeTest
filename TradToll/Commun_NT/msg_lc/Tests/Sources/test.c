/*****************(v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_LC.c                                                        */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*              	:	Creation of file for project                         */
/*****************************************************************************/


/*-------------------------------- INCLUDES:  -------------------------------*/

#include <csr_msg.h>
#include <test.h>
#include <msg_lc_header.h>
#include <stdio.h>
#include <conio.h>

/*-------------------------------- RESERVED:  -------------------------------*/


/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/


/*-------------------------------- TYPEDEFS:  -------------------------------*/


/*-------------------------------- FUNCTIONS: -------------------------------*/


/*-------------------------------- VARIABLES: -------------------------------*/


/*-------------------------------- CODE:      -------------------------------*/

void _cdecl main(void)
{
	Test_Command();
	_getch();

	Test_Payment();
	_getch();

	Test_Tab_Ack();
	_getch();

	Test_Bowl_Movement();
	_getch();

	Test_Bowl_Stat();
	_getch();

	Test_End_Shift();
	_getch();

	Test_Event();
	_getch();

	Test_Start_Shift();
	_getch();

	Test_Vault_Ins();
	_getch();

	Test_Status();
	_getch();

	Test_Transaction();
	_getch();
	
	Test_Vault_Witdraw();
	_getch();

	Test_Vault_Stat();
	_getch();

	Test_Comp_Inf_Tr();
	_getch();

	Test_Perm_Rq_Send_File();
	_getch();

	Test_Hourly();
	_getch();

	printf("\n\nMESSAGES FROM PCS TO LA\n");

	Test_Reference();
	_getch();

	Test_Perm_Send_File();
	_getch();

	Test_Backup_Date();
	_getch();

	Test_Backup_File();
	_getch();
}
    
/*-------------------------------- END OF FILE ------------------------------*/